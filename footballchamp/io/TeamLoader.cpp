// TeamLoader.cpp
#include "TeamLoader.h"
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <map>
#include <sstream>
#include "../core/Config.h"
#include "Paths.h"

namespace {

const char* const kTeamsFileName = "teams.txt";
const double kMaxRating = 1000.0;

std::string trim(const std::string& s) {
    auto isSpace = [](char c) { return c == ' ' || c == '\t' || c == '\r'; };
    size_t a = 0, b = s.size();
    while (a < b && isSpace(s[a])) ++a;
    while (b > a && isSpace(s[b - 1])) --b;
    return s.substr(a, b - a);
}

// Читає одну кодову точку UTF-8 з позиції i. false - послідовність некоректна.
bool nextCodePoint(const std::string& s, size_t& i, unsigned& cp) {
    const unsigned char c = static_cast<unsigned char>(s[i]);
    int extra = 0;
    if (c < 0x80)                { cp = c; }
    else if ((c & 0xE0) == 0xC0) { cp = c & 0x1F; extra = 1; }
    else if ((c & 0xF0) == 0xE0) { cp = c & 0x0F; extra = 2; }
    else if ((c & 0xF8) == 0xF0) { cp = c & 0x07; extra = 3; }
    else return false;

    if (i + extra >= s.size() + (extra == 0 ? 1 : 0)) return false;
    for (int k = 1; k <= extra; ++k) {
        const unsigned char cc = static_cast<unsigned char>(s[i + k]);
        if ((cc & 0xC0) != 0x80) return false;
        cp = (cp << 6) | (cc & 0x3F);
    }
    if ((extra == 1 && cp < 0x80) || (extra == 2 && cp < 0x800) || (extra == 3 && cp < 0x10000)) return false;
    if ((cp >= 0xD800 && cp <= 0xDFFF) || cp > 0x10FFFF) return false;

    i += static_cast<size_t>(extra) + 1;
    return true;
}

bool isValidUtf8(const std::string& s) {
    size_t i = 0;
    unsigned cp = 0;
    while (i < s.size()) {
        if (!nextCodePoint(s, i, cp)) return false;
    }
    return true;
}

bool hasNonAscii(const std::string& s) {
    for (char c : s) {
        if (static_cast<unsigned char>(c) >= 0x80) return true;
    }
    return false;
}

// Символ Unicode -> байт Windows-1251 (кодування консолі гри). -1, якщо в 1251 такого символу немає.
int toCp1251(unsigned cp) {
    if (cp < 0x80) return static_cast<int>(cp);
    if (cp >= 0x0410 && cp <= 0x044F) return 0xC0 + static_cast<int>(cp - 0x0410);  // А-Я, а-я
    if (cp >= 0x2010 && cp <= 0x2015) return '-';                                    // різні тире
    switch (cp) {
        case 0x0401: return 0xA8;  // Ё
        case 0x0451: return 0xB8;  // ё
        case 0x0404: return 0xAA;  // Є
        case 0x0454: return 0xBA;  // є
        case 0x0406: return 0xB2;  // І
        case 0x0456: return 0xB3;  // і
        case 0x0407: return 0xAF;  // Ї
        case 0x0457: return 0xBF;  // ї
        case 0x0490: return 0xA5;  // Ґ
        case 0x0491: return 0xB4;  // ґ
        case 0x00A0: return ' ';   // нерозривний пробіл
        case 0x2018: case 0x2019: case 0x02BC: return '\'';  // типографські апострофи
        default: return -1;
    }
}

// Рядок UTF-8 -> Windows-1251. Символи, яких немає в 1251, дають помилку з номером рядка.
std::string utf8LineToCp1251(const std::string& line, size_t lineNo, std::vector<std::string>& errors) {
    std::string out;
    size_t i = 0;
    while (i < line.size()) {
        unsigned cp = 0;
        nextCodePoint(line, i, cp);  // файл уже перевірено як коректний UTF-8
        const int byte = toCp1251(cp);
        if (byte < 0) {
            std::ostringstream message;
            message << "Рядок " << lineNo << ": символ U+" << std::hex << std::uppercase << cp
                    << " не підтримується консоллю (кодування 1251). Замініть його.";
            errors.push_back(message.str());
            out += '?';
        } else {
            out += static_cast<char>(byte);
        }
    }
    return out;
}

// Додатне число: цифри з однією десятковою крапкою або комою
bool parseRating(const std::string& text, double& value) {
    std::string s = trim(text);
    if (s.empty()) return false;
    int separators = 0;
    for (char& c : s) {
        if (c == ',') c = '.';
        if (c == '.') {
            ++separators;
        } else if (c < '0' || c > '9') {
            return false;
        }
    }
    if (separators > 1 || s == ".") return false;
    value = std::strtod(s.c_str(), nullptr);
    return value > 0.0 && value <= kMaxRating;
}

std::vector<std::string> splitFields(const std::string& line) {
    std::vector<std::string> fields;
    size_t start = 0;
    while (true) {
        const size_t pos = line.find(';', start);
        if (pos == std::string::npos) {
            fields.push_back(trim(line.substr(start)));
            return fields;
        }
        fields.push_back(trim(line.substr(start, pos - start)));
        start = pos + 1;
    }
}

// Нижній регістр для кодування 1251 (латиниця й українські літери)
std::string lowerCp1251(std::string s) {
    for (char& ch : s) {
        const unsigned char c = static_cast<unsigned char>(ch);
        if (c >= 'A' && c <= 'Z') ch = static_cast<char>(c + 32);
        else if (c >= 0xC0 && c <= 0xDF) ch = static_cast<char>(c + 32);
        else if (c == 0xA8) ch = static_cast<char>(0xB8);  // Ё -> ё
        else if (c == 0xAA) ch = static_cast<char>(0xBA);  // Є -> є
        else if (c == 0xAF) ch = static_cast<char>(0xBF);  // Ї -> ї
        else if (c == 0xB2) ch = static_cast<char>(0xB3);  // І -> і
        else if (c == 0xA5) ch = static_cast<char>(0xB4);  // Ґ -> ґ
    }
    return s;
}

// Позиція: GK / DF / MF / FW (будь-який регістр) або українське слово
bool parsePosition(const std::string& token, Position& position) {
    const std::string word = lowerCp1251(token);
    if (word == "gk" || word == "воротар")                         { position = Position::Goalkeeper; return true; }
    if (word == "df" || word == "захисник")                        { position = Position::Defender;   return true; }
    if (word == "mf" || word == "півзахисник")                     { position = Position::Midfielder; return true; }
    if (word == "fw" || word == "нападник" || word == "нападаючий") { position = Position::Forward;    return true; }
    return false;
}

// Команда, що читається зараз: рядок команди та гравці під ним
struct PendingTeam {
    Team team{};
    size_t lineNo = 0;
    bool open = false;            // рядок команди вже зустрівся
    bool valid = false;           // рядок команди розібрано без помилок
    bool squadHasErrors = false;  // у рядках гравців були помилки (кількість тоді не перевіряємо)
    std::map<std::string, size_t> lineOfPlayer;
};

// Перевіряє склад команди. Повідомлення додає в errors. true, якщо склад коректний.
// Команди без жодного гравця описуються окремо (emptySquadErrors): якщо їх більшість, це ознака
// старого формату файлу, і показувати 36 однакових помилок немає сенсу.
bool validateSquad(const PendingTeam& pending, std::vector<std::string>& errors,
                   std::vector<std::string>& emptySquadErrors) {
    if (pending.squadHasErrors) return false;  // помилки вже показано

    const Team& team = pending.team;
    const std::string prefix = "Команда «" + team.name + "» (рядок " + std::to_string(pending.lineNo) + "): ";
    bool ok = true;

    if (team.squad.empty()) {
        emptySquadErrors.push_back(prefix + "не вказано жодного гравця (потрібно " + std::to_string(Config::SQUAD_SIZE) + ").");
        return false;
    }
    if (team.squad.size() != static_cast<size_t>(Config::SQUAD_SIZE)) {
        errors.push_back(prefix + "потрібно рівно " + std::to_string(Config::SQUAD_SIZE) +
                         " гравців, знайдено " + std::to_string(team.squad.size()) + ".");
        ok = false;
    }
    int keepers = 0;
    for (const auto& player : team.squad) {
        if (player.position == Position::Goalkeeper) ++keepers;
    }
    if (keepers != 1) {
        errors.push_back(prefix + "потрібен рівно один воротар (GK), знайдено " + std::to_string(keepers) + ".");
        ok = false;
    }
    return ok;
}

}  // namespace

TeamLoadResult loadTeamsFromFile(const std::string& path) {
    TeamLoadResult result;
    result.path = path;

    std::ifstream in(path, std::ios::binary);
    if (!in) {
        result.errors.push_back("Не вдалося відкрити файл.");
        return result;
    }
    std::string raw((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (raw.compare(0, 3, "\xEF\xBB\xBF") == 0) raw.erase(0, 3);  // BOM
    const bool isUtf8 = hasNonAscii(raw) && isValidUtf8(raw);

    std::vector<std::string>& errors = result.errors;
    std::map<std::string, size_t> lineOfName;
    std::vector<std::string> emptySquadErrors;
    size_t validTeamLines = 0;
    PendingTeam current;

    // Завершує поточну команду: перевіряє склад і, якщо все гаразд, додає її в список
    auto finishTeam = [&]() {
        if (current.open && current.valid && validateSquad(current, errors, emptySquadErrors)) {
            current.team.initialRating = static_cast<int>(result.teams.size()) + 1;  // порядок у файлі = рейтинг
            result.teams.push_back(current.team);
        }
        current = PendingTeam();
    };

    std::istringstream lines(raw);
    std::string line;
    size_t lineNo = 0;
    while (std::getline(lines, line)) {
        ++lineNo;

        const std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed[0] == '#') continue;  // порожні рядки й коментарі

        if (isUtf8) line = utf8LineToCp1251(line, lineNo, errors);
        line = trim(line);
        const std::string prefix = "Рядок " + std::to_string(lineNo) + ": ";

        // ----- Рядок команди: «Назва; атака; захист» -----
        if (line.find(';') != std::string::npos) {
            finishTeam();
            current.open = true;
            current.lineNo = lineNo;

            const std::vector<std::string> fields = splitFields(line);
            if (fields.size() != 3) {
                errors.push_back(prefix + "очікується формат «Назва; атака; захист».");
                continue;
            }

            const std::string& name = fields[0];
            double attack = 0.0, defense = 0.0;
            bool lineOk = true;

            if (name.empty()) {
                errors.push_back(prefix + "порожня назва команди.");
                lineOk = false;
            }
            if (!parseRating(fields[1], attack)) {
                errors.push_back(prefix + "атака «" + fields[1] + "» - потрібне число більше 0.");
                lineOk = false;
            }
            if (!parseRating(fields[2], defense)) {
                errors.push_back(prefix + "захист «" + fields[2] + "» - потрібне число більше 0.");
                lineOk = false;
            }
            if (lineOk) {
                auto existing = lineOfName.find(name);
                if (existing != lineOfName.end()) {
                    errors.push_back(prefix + "команда «" + name + "» уже є в рядку " + std::to_string(existing->second) + ".");
                    lineOk = false;
                }
            }
            if (!lineOk) continue;

            lineOfName[name] = lineNo;
            ++validTeamLines;
            current.valid = true;
            current.team.name = name;
            current.team.attack = attack;
            current.team.defense = defense;
            continue;
        }

        // ----- Рядок гравця: «ПОЗИЦІЯ Ім'я» -----
        if (!current.open) {
            errors.push_back(prefix + "гравця вказано до першої команди.");
            continue;
        }
        if (!current.valid) continue;  // помилку в рядку команди вже показано

        const size_t split = line.find_first_of(" \t");
        const std::string positionWord = (split == std::string::npos) ? line : line.substr(0, split);
        const std::string playerName = (split == std::string::npos) ? std::string() : trim(line.substr(split));

        Position position = Position::Midfielder;
        if (!parsePosition(positionWord, position)) {
            errors.push_back(prefix + "невідома позиція «" + positionWord +
                             "» (потрібно GK, DF, MF, FW або воротар, захисник, півзахисник, нападник). "
                             "Якщо це рядок команди, використайте формат «Назва; атака; захист».");
            current.squadHasErrors = true;
            continue;
        }
        if (playerName.empty()) {
            errors.push_back(prefix + "немає імені гравця.");
            current.squadHasErrors = true;
            continue;
        }
        auto existing = current.lineOfPlayer.find(playerName);
        if (existing != current.lineOfPlayer.end()) {
            errors.push_back(prefix + "гравець «" + playerName + "» уже є в складі цієї команди (рядок " +
                             std::to_string(existing->second) + ").");
            current.squadHasErrors = true;
            continue;
        }
        current.lineOfPlayer[playerName] = lineNo;
        current.team.squad.push_back(Player{playerName, position});
    }
    finishTeam();

    if (!emptySquadErrors.empty()) {
        if (emptySquadErrors.size() == validTeamLines) {  // складів немає взагалі: старий формат файлу
            errors.push_back("У файлі немає складів. Після рядка кожної команди додайте " + std::to_string(Config::SQUAD_SIZE) +
                             " гравців у форматі «ПОЗИЦІЯ Ім'я» (приклад на початку data/teams.txt).");
        } else {
            errors.insert(errors.end(), emptySquadErrors.begin(), emptySquadErrors.end());
        }
    }

    if (errors.empty() && result.teams.size() != static_cast<size_t>(Config::TEAMS_COUNT)) {
        errors.push_back("Знайдено команд: " + std::to_string(result.teams.size()) +
                         ", а потрібно рівно " + std::to_string(Config::TEAMS_COUNT) + ".");
    }

    result.ok = errors.empty();
    if (!result.ok) result.teams.clear();
    return result;
}

TeamLoadResult loadTeams() {
    const std::string path = findDataFile(kTeamsFileName);
    if (path.empty()) {
        TeamLoadResult result;
        result.errors.push_back(std::string("Файл ") + kTeamsFileName +
                                " не знайдено. Покладіть його в теку data поруч із проєктом або програмою.");
        result.errors.push_back("Шукали тут:");
        const auto candidates = dataFileCandidates(kTeamsFileName);
        for (size_t i = 0; i < candidates.size() && i < 6; ++i) {
            result.errors.push_back("    " + candidates[i]);
        }
        return result;
    }
    return loadTeamsFromFile(path);
}

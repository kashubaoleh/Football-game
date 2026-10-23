// Tournament.h - стан і проходження одного турніру. Нічого не виводить у консоль.
#pragma once
#include <map>
#include <optional>
#include <string>
#include <vector>
#include "../core/Config.h"
#include "../core/Match.h"
#include "../core/Team.h"
#include "../sim/MatchSimulator.h"
#include "../sim/Random.h"
#include "LeagueDraw.h"
#include "LeagueTable.h"
#include "PlayoffManager.h"

// Результат одного зіграного матчу плей-оф (усе, що потрібно для відображення)
struct PlayoffStepResult {
    enum class Kind { FirstLeg, SecondLeg, Final };

    Kind kind = Kind::FirstLeg;
    const Match* match = nullptr;    // зіграний матч (вказує всередину PlayoffManager)
    int roundIndex = 0;
    std::string roundName;

    // Заповнюється для SecondLeg і Final
    Team winner;                     // переможець дуелі / фіналу
    Team loser;                      // той, хто програв (потрібно для фіналіста)
    bool decidedByPenalties = false;

    // Тільки для SecondLeg: сума голів за обидва матчі дуелі
    int homeAggregate = 0;           // господарі матчу-відповіді
    int awayAggregate = 0;           // гості матчу-відповіді

    // Індекс раунду, який щойно сформовано автоматично (2, 3 або 4), інакше -1
    int generatedRoundIndex = -1;
};

// Віхи турніру для швидкої промотки з календаря
enum class Milestone {
    LeagueEnd,           // етап ліги завершено (жеребкування 1/16 ще не проведене)
    RoundOf8Start,       // 1/16 завершено (жеребкування 1/8 ще не проведене)
    QuarterFinalsStart,  // сформовано пари 1/4
    SemiFinalsStart,     // сформовано пари 1/2
    FinalStart,          // сформовано пару фіналу
    TournamentEnd        // фінал зіграно
};

// Що саме є наступною дією в турнірі
enum class NextAction {
    LeagueDraw,     // жеребкування етапу ліги
    LeagueMatch,    // наступний матч етапу ліги
    RoundOf16Draw,  // жеребкування 1/16
    RoundOf8Draw,   // жеребкування 1/8
    PlayoffMatch,   // наступний матч плей-оф
    NewTournament   // турнір завершено, можна почати новий
};

struct TournamentResult {
    std::string champion;
    std::string finalist;
    std::map<std::string, int> leagueRank; // назва команди -> місце в таблиці ліги (1-36)
};

class Tournament {
public:
    // seed повністю визначає турнір: з тим самим seed вийдуть ті самі жеребкування й результати
    // матчів незалежно від того, у якому порядку їх проходити
    Tournament(const std::vector<Team>& teams, std::uint32_t seed);

    // ----- Стан (лише читання) -----
    const std::vector<Team>& getTeams() const { return allTeams; }
    const std::vector<Round>& getSchedule() const { return schedule; }
    const LeagueTable& getLeagueTable() const { return leagueTable; }
    const PlayoffManager& getPlayoff() const { return playoffManager; }
    std::uint32_t getSeed() const { return seed; }

    bool isDrawCompleted() const { return drawCompleted; }
    bool isPlayoffStarted() const { return playoffStarted; }
    int getLeagueRoundIndex() const { return currentRoundIndex; }  // поточний тур ліги
    int getLeagueMatchIndex() const { return currentMatchIndex; }  // поточний матч у турі

    NextAction nextAction() const;
    bool isTournamentFinished() const;
    bool isMilestoneReached(Milestone milestone) const;

    // Топ-N команд за поточною таблицею ліги (1-й елемент = 1 місце)
    std::vector<Team> getLeagueTopTeams(int count) const;

    // ----- Дії -----
    void performLeagueDraw();       // жеребкування етапу ліги
    void performRoundOf16Draw();    // жеребкування 1/16
    void performRoundOf8Draw();     // жеребкування 1/8

    // Грає наступний матч ліги (симуляція, таблиця, перехід далі) і повертає його
    const Match& playNextLeagueMatch();

    // Грає поточний матч плей-оф, визначає переможця дуелі й формує наступний раунд,
    // якщо він формується автоматично
    PlayoffStepResult playCurrentPlayoffMatch();

    // Тиха промотка: жеребкування за потреби проводяться самі
    void simulateMatches(int count);          // наступні count матчів
    void simulateUntil(Milestone milestone);  // до віхи

    void reset();                             // стерти дані турніру й почати новий (seed береться наступний)
    TournamentResult runSilentTournament();   // весь турнір без показу (для статистики)

private:
    std::uint32_t seed;
    std::vector<Team> allTeams;
    std::vector<Round> schedule;      // розклад етапу ліги
    MatchSimulator simulator;
    LeagueTable leagueTable;
    PlayoffManager playoffManager;

    bool drawCompleted = false;
    bool playoffStarted = false;
    int currentRoundIndex = 0;
    int currentMatchIndex = 0;

    Random makeRandom(std::uint64_t tag) const;  // власний генератор для жеребкування чи матчу
    void playLeagueMatch(Match& match);          // симулює матч ліги й оновлює таблицю
    void advanceLeaguePosition();                // перейти до наступного матчу ліги
    bool performPendingDraw();                   // проводить жеребкування, якщо воно потрібне
    std::optional<PlayoffStepResult> playNextMatch(); // результат є лише для матчів плей-оф
};

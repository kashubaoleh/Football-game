// Tournament.cpp - проходження турніру: жеребкування, матчі, промотка, тихий турнір
#include "Tournament.h"

namespace {

// Кожне жеребкування й кожен матч отримує власний генератор, похідний від seed турніру.
// Тому результат матчу не залежить від того, чи його промотали, чи показали, і в якому порядку.
constexpr std::uint64_t kLeagueDrawTag = 1;
constexpr std::uint64_t kRoundOf16DrawTag = 2;
constexpr std::uint64_t kRoundOf8DrawTag = 3;

std::uint64_t leagueMatchTag(int round, int match) {
    return 0x10000u + static_cast<std::uint64_t>(round) * 1000u + static_cast<std::uint64_t>(match);
}

std::uint64_t playoffMatchTag(int round, int match) {
    return 0x20000u + static_cast<std::uint64_t>(round) * 1000u + static_cast<std::uint64_t>(match);
}

}  // namespace

// ===================== Стан =====================

Tournament::Tournament(const std::vector<Team>& teams, std::uint32_t seedValue)
    : seed(seedValue), allTeams(teams) {}

Random Tournament::makeRandom(std::uint64_t tag) const {
    return Random(seed).derive(tag);
}

NextAction Tournament::nextAction() const {
    if (!drawCompleted) return NextAction::LeagueDraw;
    if (currentRoundIndex < Config::LEAGUE_ROUNDS) return NextAction::LeagueMatch;
    if (!playoffStarted) return NextAction::RoundOf16Draw;
    if (playoffManager.isPlayoffFinished()) return NextAction::NewTournament;
    if (playoffManager.getCurrentRound().roundIndex == 1 && !playoffManager.isDraw18Completed()) {
        return NextAction::RoundOf8Draw;
    }
    return NextAction::PlayoffMatch;
}

const Match& Tournament::playNextLeagueMatch() {
    Match& match = schedule[currentRoundIndex].matches[currentMatchIndex];
    playLeagueMatch(match);
    advanceLeaguePosition();
    return match;
}

// ===================== Жеребкування =====================

void Tournament::performLeagueDraw() {
    LeagueDraw drawer(allTeams);
    Random rng = makeRandom(kLeagueDrawTag);
    schedule = drawer.generateSchedule(rng);
    drawCompleted = true;
    leagueTable.initialize(allTeams);
}

void Tournament::performRoundOf16Draw() {
    Random rng = makeRandom(kRoundOf16DrawTag);
    playoffManager.generateRoundOf16(leagueTable.getTable(), allTeams, rng);
    playoffStarted = true;
}

void Tournament::performRoundOf8Draw() {
    Random rng = makeRandom(kRoundOf8DrawTag);
    playoffManager.generateRoundOf8(getLeagueTopTeams(8), rng);
}

// Проводить жеребкування, якщо воно потрібне перед наступним матчем.
// Повертає true, якщо жеребкування було проведено.
bool Tournament::performPendingDraw() {
    switch (nextAction()) {
        case NextAction::LeagueDraw:    performLeagueDraw();    return true;
        case NextAction::RoundOf16Draw: performRoundOf16Draw(); return true;
        case NextAction::RoundOf8Draw:  performRoundOf8Draw();  return true;
        default:                        return false;
    }
}

// ===================== Матчі =====================

void Tournament::playLeagueMatch(Match& match) {
    Random rng = makeRandom(leagueMatchTag(currentRoundIndex, currentMatchIndex));
    match.resultLog = simulator.simulateMatch(match.homeTeam, match.awayTeam, MatchRules::GroupStage, rng);
    match.isPlayed = true;
    leagueTable.updateMatch(match.homeTeam.name, match.resultLog.homeGoals,
                            match.awayTeam.name, match.resultLog.awayGoals);
}

std::vector<Team> Tournament::getLeagueTopTeams(int count) const {
    std::vector<Team> top;
    top.reserve(count);
    for (int rank = 1; rank <= count; ++rank) {
        top.push_back(playoffManager.findTeamByRank(leagueTable.getTable(), allTeams, rank));
    }
    return top;
}

PlayoffStepResult Tournament::playCurrentPlayoffMatch() {
    PlayoffStepResult res;

    auto& round = playoffManager.getCurrentRound();
    const int matchIdx = playoffManager.getCurrentMatchIndex();
    const int roundIdx = round.roundIndex;
    Match& match = round.matches[matchIdx];
    Random rng = makeRandom(playoffMatchTag(roundIdx, matchIdx));

    res.match = &match;
    res.roundIndex = roundIdx;
    res.roundName = round.roundName;

    // Фінал — завжди одноматчевий, на нейтральному полі
    if (roundIdx == 4) {
        match.resultLog = simulator.simulateMatch(match.homeTeam, match.awayTeam, MatchRules::Knockout, rng, false, 0, 0, true);
        match.isPlayed = true;

        res.kind = PlayoffStepResult::Kind::Final;
        res.decidedByPenalties = (match.resultLog.homeGoals == match.resultLog.awayGoals);

        bool homeWins;
        if (res.decidedByPenalties) {
            homeWins = match.resultLog.homePenaltyGoals > match.resultLog.awayPenaltyGoals;
        } else {
            homeWins = match.resultLog.homeGoals > match.resultLog.awayGoals;
        }
        res.winner = homeWins ? match.homeTeam : match.awayTeam;
        res.loser  = homeWins ? match.awayTeam : match.homeTeam;

        playoffManager.advanceMatch();
        return res;
    }

    const int halfMatches = static_cast<int>(round.matches.size()) / 2;

    // --- Перший матч дуелі ---
    if (matchIdx < halfMatches) {
        match.resultLog = simulator.simulateMatch(match.homeTeam, match.awayTeam, MatchRules::GroupStage, rng);
        match.isPlayed = true;

        res.kind = PlayoffStepResult::Kind::FirstLeg;
        playoffManager.advanceMatch();
        return res;
    }

    // --- Матч-відповідь ---
    Match& firstLeg = round.matches[matchIdx - halfMatches];

    // Господарі матчу-відповіді були гостями першого матчу (і навпаки)
    const int homeFirstLegGoals = firstLeg.resultLog.awayGoals; // голи господарів матчу-відповіді у першому матчі
    const int awayFirstLegGoals = firstLeg.resultLog.homeGoals; // голи гостей матчу-відповіді у першому матчі

    match.resultLog = simulator.simulateMatch(match.homeTeam, match.awayTeam, MatchRules::Knockout, rng,
                                              true, homeFirstLegGoals, awayFirstLegGoals);
    match.isPlayed = true;

    res.kind = PlayoffStepResult::Kind::SecondLeg;
    res.homeAggregate = homeFirstLegGoals + match.resultLog.homeGoals;
    res.awayAggregate = awayFirstLegGoals + match.resultLog.awayGoals;

    bool homeAdvances;
    if (res.homeAggregate != res.awayAggregate) {
        homeAdvances = res.homeAggregate > res.awayAggregate;
    } else {
        res.decidedByPenalties = true;
        homeAdvances = match.resultLog.homePenaltyGoals > match.resultLog.awayPenaltyGoals;
    }
    res.winner = homeAdvances ? match.homeTeam : match.awayTeam;
    res.loser  = homeAdvances ? match.awayTeam : match.homeTeam;

    playoffManager.addWinner(res.winner);
    playoffManager.advanceMatch();

    // Коли завершено всі дуелі раунду — формуємо наступний
    const size_t winnersCount = playoffManager.getWinners().size();
    if (roundIdx == 1 && winnersCount == 8) {
        playoffManager.generateQuarterFinals();
        playoffManager.clearWinners();
        res.generatedRoundIndex = 2;
    } else if (roundIdx == 2 && winnersCount == 4) {
        playoffManager.generateSemiFinals();
        playoffManager.clearWinners();
        res.generatedRoundIndex = 3;
    } else if (roundIdx == 3 && winnersCount == 2) {
        playoffManager.generateFinal();
        playoffManager.clearWinners();
        res.generatedRoundIndex = 4;
    }

    return res;
}

void Tournament::advanceLeaguePosition() {
    currentMatchIndex++;
    if (currentMatchIndex >= Config::MATCHES_PER_ROUND) {
        currentMatchIndex = 0;
        currentRoundIndex++;
    }
}

// Грає наступний матч турніру. Жеребкування для нього має бути вже проведене
// (див. performPendingDraw). Результат повертається лише для матчів плей-оф.
std::optional<PlayoffStepResult> Tournament::playNextMatch() {
    if (currentRoundIndex < Config::LEAGUE_ROUNDS) {
        playNextLeagueMatch();
        return std::nullopt;
    }
    return playCurrentPlayoffMatch();
}

// ===================== Стан і промотка =====================

// Стирає дані поточного турніру й готує новий: жеребкування, таблиця, плей-оф, позиція.
// Команди лишаються, seed переходить до наступного в ланцюжку.
void Tournament::reset() {
    seed = Random::nextSeed(seed);
    schedule.clear();
    leagueTable = LeagueTable();
    playoffManager = PlayoffManager();
    drawCompleted = false;
    playoffStarted = false;
    currentRoundIndex = 0;
    currentMatchIndex = 0;
}

bool Tournament::isTournamentFinished() const {
    return nextAction() == NextAction::NewTournament;
}

bool Tournament::isMilestoneReached(Milestone milestone) const {
    switch (milestone) {
        case Milestone::LeagueEnd:
            return drawCompleted && currentRoundIndex >= Config::LEAGUE_ROUNDS;
        case Milestone::RoundOf8Start:
            return playoffStarted && playoffManager.getCurrentRoundIndex() >= 1;
        case Milestone::QuarterFinalsStart:
            return playoffStarted && playoffManager.getCurrentRoundIndex() >= 2;
        case Milestone::SemiFinalsStart:
            return playoffStarted && playoffManager.getCurrentRoundIndex() >= 3;
        case Milestone::FinalStart:
            return playoffStarted && playoffManager.getCurrentRoundIndex() >= 4;
        case Milestone::TournamentEnd:
            return isTournamentFinished();
    }
    return false;
}

// Симулює наступні count матчів. Потрібні жеребкування проводяться тихо, але лише тоді,
// коли після них ще є матч для симуляції.
void Tournament::simulateMatches(int count) {
    for (int played = 0; played < count && !isTournamentFinished(); ++played) {
        performPendingDraw();
        playNextMatch();
    }
}

// Симулює турнір до віхи (жеребкування, після якого вона настає, не проводиться).
void Tournament::simulateUntil(Milestone milestone) {
    int guard = 0; // захист від нескінченного циклу
    while (!isMilestoneReached(milestone) && !isTournamentFinished() && guard++ < 1000) {
        if (!performPendingDraw()) {
            playNextMatch();
        }
    }
}

// ===================== Тихий турнір (для batch-статистики) =====================

TournamentResult Tournament::runSilentTournament() {
    TournamentResult res;

    simulateUntil(Milestone::LeagueEnd);

    const std::vector<Team> ranked = getLeagueTopTeams(Config::TEAMS_COUNT);
    for (size_t i = 0; i < ranked.size(); ++i) {
        res.leagueRank[ranked[i].name] = static_cast<int>(i) + 1;
    }

    int guard = 0; // захист від нескінченного циклу
    while (!isTournamentFinished() && guard++ < 1000) {
        performPendingDraw();
        auto step = playNextMatch();
        if (step && step->kind == PlayoffStepResult::Kind::Final) {
            res.champion = step->winner.name;
            res.finalist = step->loser.name;
        }
    }

    return res;
}

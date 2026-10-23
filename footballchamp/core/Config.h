// Config.h
#pragma once

namespace Config {

    // ===== Структура формату (не змінювати без переписування LeagueDraw) =====
    constexpr int LEAGUE_ROUNDS     = 8;
    constexpr int MATCHES_PER_ROUND = 18;
    constexpr int TEAMS_COUNT       = 36;
    constexpr int TEAMS_PER_POT = 9;
    constexpr int SQUAD_SIZE        = 11;  // стартовий склад: гравців у кожної команди

    // ===== Баланс симуляції (можна вільно крутити) =====
    constexpr double HOME_BONUS        = 1.08;
    constexpr double XG_POWER          = 1.6;
    constexpr double DAY_FORM_MIN      = 0.90;
    constexpr double DAY_FORM_MAX      = 1.10;

    // ===== Хто забиває: вага гравця залежно від позиції (чим більша, тим частіше забиває) =====
    // Вага рахується на кожного гравця, тож частка гола залежить і від схеми команди.
    // Для схеми 4-3-3 це приблизно 59% голів у нападників, 29% у півзахисників, 12% у захисників.
    constexpr double SCORER_WEIGHT_GOALKEEPER = 0.0;
    constexpr double SCORER_WEIGHT_DEFENDER   = 1.5;
    constexpr double SCORER_WEIGHT_MIDFIELDER = 5.0;
    constexpr double SCORER_WEIGHT_FORWARD    = 10.0;

    // ===== Швидкість трансляції (мс на хвилину) =====
    constexpr int DELAY_FAST   = 30;
    constexpr int DELAY_MEDIUM = 100;
    constexpr int DELAY_SLOW   = 300;

};

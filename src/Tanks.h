#pragma once

#include <array>

#include "EnemyTank.h"
#include "PlayerTank.h"
#include "Tank.h"

// The two player tanks and three enemy tanks for a single match.
struct TankRoster
{
    PlayerTank player1{1};
    PlayerTank player2{2};
    EnemyTank  enemy1{1};
    EnemyTank  enemy2{2};
    EnemyTank  enemy3{3};
};

// All tanks as Tank* so they can be updated/drawn polymorphically
// (non-owning views into the roster).
inline std::array<Tank*, 5> tanksOf(TankRoster& roster)
{
    return { &roster.player1, &roster.player2,
             &roster.enemy1,  &roster.enemy2, &roster.enemy3 };
}

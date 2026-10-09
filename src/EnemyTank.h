#pragma once

#include "Tank.h"

// A computer-controlled tank. In this sample the movement/shoot methods are
// driven by simple AI logic that steers toward a target position.
class EnemyTank : public Tank
{
public:
    explicit EnemyTank(int id) : m_id(id) {}

    void up() override    { m_dir = Direction::Up;    m_y += 1; }
    void down() override  { m_dir = Direction::Down;  m_y -= 1; }
    void left() override  { m_dir = Direction::Left;  m_x -= 1; }
    void right() override { m_dir = Direction::Right; m_x += 1; }

    void shoot() override
    {
        // Spawn a projectile from (m_x, m_y) travelling in m_dir.
        m_ammo -= 1;
    }

    // Simple AI: move one step toward the assigned target.
    void step()
    {
        if (m_tx > m_x)      right();
        else if (m_tx < m_x) left();
        else if (m_ty > m_y) up();
        else if (m_ty < m_y) down();
    }

    void setTarget(int x, int y) { m_tx = x; m_ty = y; }

    int id() const { return m_id; }
    int x() const override { return m_x; }
    int y() const override { return m_y; }
    Direction direction() const override { return m_dir; }
    int ammo() const { return m_ammo; }

    // --- sprites -----------------------------------------------------------
    SpritePoint spriteOrigin() const { return enemySpriteOrigin(m_id); }
    SpriteRect sprite() const override { return spriteRect(spriteOrigin(), m_dir, m_frame); }
    int frame() const { return m_frame; }
    void setFrame(int f) { m_frame = f % kFramesPerDir; }

    // Scene helpers used by the demo to lay tanks out.
    void place(int x, int y) { m_x = x; m_y = y; }
    void face(Direction dir) { m_dir = dir; }

private:
    int m_id;
    int m_x = 0;
    int m_y = 0;
    int m_tx = 0;
    int m_ty = 0;
    int m_ammo = 3;
    int m_frame = 0;
    Direction m_dir = Direction::Down;
};

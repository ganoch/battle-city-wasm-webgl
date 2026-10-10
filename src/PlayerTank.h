#pragma once

#include "Tank.h"

// A human-controlled tank. Movement is one grid step per call; shoot()
// fires a projectile in the direction the tank is currently facing.
class PlayerTank : public Tank
{
public:
    explicit PlayerTank(int id) : m_id(id) {}

    void up() override    { m_dir = Direction::Up;    m_y += 1; }
    void down() override  { m_dir = Direction::Down;  m_y -= 1; }
    void left() override  { m_dir = Direction::Left;  m_x -= 1; }
    void right() override { m_dir = Direction::Right; m_x += 1; }

    void shoot() override
    {
        // Spawn a projectile from (m_x, m_y) travelling in m_dir.
        m_ammo -= 1;
    }

    int id() const { return m_id; }
    int x() const override { return m_x; }
    int y() const override { return m_y; }
    Direction direction() const override { return m_dir; }
    int ammo() const { return m_ammo; }

    // --- sprites -----------------------------------------------------------
    SpritePoint spriteOrigin() const { return playerSpriteOrigin(m_id, m_level); }
    SpriteRect sprite() const override { return spriteRect(spriteOrigin(), m_dir, m_frame); }
    int frame() const { return m_frame; }
    void setFrame(int f) { m_frame = f % kFramesPerDir; }

    // Scene helpers used by the demo to lay tanks out.
    void place(int x, int y) { m_x = x; m_y = y; }
    void face(Direction dir) { m_dir = dir; }
    void levelUp() { if(m_level < 3) { m_level += 1; } }
    void levelDown() { if(m_level > 0) { m_level -= 1; } }

private:
    int m_id;
    int m_x = 0;
    int m_y = 0;
    int m_ammo = 5;
    int m_frame = 0;
    int m_level = 0;
    Direction m_dir = Direction::Up;
};

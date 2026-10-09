#pragma once

#include "Sprites.h"

// Abstract base class for all tank entities.
//
// The pure virtual methods below define the minimum interface every concrete
// tank must implement. Movement is expressed in grid/board units (one step per
// call) and shooting fires a single projectile.
//
// Rendering is shared: render() (implemented in tank.cpp) draws the sprite
// returned by sprite() at the tank's grid position, so a tank's appearance
// automatically follows its current direction.
class Tank
{
public:
    virtual ~Tank() = default;

    // Movement actions (one step in the given direction).
    virtual void up() = 0;
    virtual void down() = 0;
    virtual void left() = 0;
    virtual void right() = 0;

    // Fire a single projectile in the direction the tank is currently facing.
    virtual void shoot() = 0;

    // --- state used for rendering -----------------------------------------
    virtual int x() const = 0;                // grid column
    virtual int y() const = 0;                // grid row (grows upward)
    virtual Direction direction() const = 0;  // current facing
    // Source rectangle in sprites.png for the current direction/frame.
    virtual SpriteRect sprite() const = 0;

    // --- rendering ---------------------------------------------------------
    // One-time setup: builds the sprite shader/quad and starts loading the
    // sprite sheet from `url` (resolved relative to index.html, e.g.
    // "sprites.png"). Requires a current WebGL2 context.
    static void initRenderer(const char* url = "sprites.png");

    // True once the sprite sheet image has finished loading.
    static bool spritesReady();

    // Draw this tank's sprite at its grid position, facing its current
    // direction.
    void render() const;
};

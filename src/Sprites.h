#pragma once

// Direction a tank is facing / travelling in.
enum class Direction
{
    Up,
    Down,
    Left,
    Right
};

// ---------------------------------------------------------------------------
// sprites.png (400 x 256) - tank strips
// ---------------------------------------------------------------------------
// Each tank occupies one horizontal strip of 8 sprites, 16x16 each
// (128 x 16 px total). Frames are grouped per direction, two frames each,
// in the order Up, Left, Down, Right:
//
//   index :  0     1    |  2      3    |  4      5    |  6       7
//   dir   :  Up    Up   |  Left   Left |  Down   Down |  Right   Right
//   frame :  0     1    |  0      1    |  0      1    |  0       1
//
// Top-left pixel of each tank's strip in sprites.png:
//   player1 : (0,   0)    0,0     -> 127,15
//   player2 : (0,   128)  0,128   -> 127,143
//   enemy1  : (128, 0)    128,0   -> 255,15
//   enemy2  : (128, 16)   128,16  -> 255,31   (next row down)
//   enemy3  : (128, 32)   128,32  -> 255,47   (row down again)
// ---------------------------------------------------------------------------

constexpr int kSpriteSize   = 16;                              // 16x16 sprites
constexpr int kSpriteCount  = 8;                               // frames per tank
constexpr int kStripWidth   = kSpriteSize * kSpriteCount;      // 128
constexpr int kDirections   = 4;                               // Up/Left/Down/Right
constexpr int kFramesPerDir = kSpriteCount / kDirections;      // 2

struct SpritePoint
{
    int x;
    int y;
};

struct SpriteRect
{
    int x;
    int y;
    int w;
    int h;
};

constexpr SpritePoint kPlayer1Origin{   0,   0 };
constexpr SpritePoint kPlayer2Origin{   0, 128 };
constexpr SpritePoint kEnemy1Origin { 128,  64 };
constexpr SpritePoint kEnemy2Origin { 128,  80 };
constexpr SpritePoint kEnemy3Origin { 128,  96 };
constexpr SpritePoint kEnemy4Origin { 128, 112 };

// Slot of a direction inside the strip's Up/Left/Down/Right cycle.
constexpr int dirCycleIndex(Direction dir)
{
    switch (dir)
    {
        case Direction::Up:    return 0;
        case Direction::Left:  return 1;
        case Direction::Down:  return 2;
        case Direction::Right: return 3;
    }
    return 0;
}

// Source rectangle in sprites.png for a strip origin, facing and frame.
constexpr SpriteRect spriteRect(SpritePoint origin, Direction dir, int frame = 0)
{
    const int index = dirCycleIndex(dir) * kFramesPerDir + (frame % kFramesPerDir);
    return { origin.x + index * kSpriteSize, origin.y, kSpriteSize, kSpriteSize };
}

// Strip origin for each tank, keyed by the id used in the roster.
constexpr SpritePoint playerSpriteOrigin(int id,int level = 0)
{
    int x = id == 2 ? kPlayer2Origin.x : kPlayer1Origin.x;
    int y = id == 2 ? kPlayer2Origin.y : kPlayer1Origin.y ;
    return { x, y + (level * 16) };
}

constexpr SpritePoint enemySpriteOrigin(int id)
{
    switch (id)
    {
        case 2:  return kEnemy2Origin;
        case 3:  return kEnemy3Origin;
        case 4:  return kEnemy4Origin;
        default: return kEnemy1Origin;
    }
}

#include <GLES3/gl3.h>

#include <cstring>

#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/html5_webgl.h>

#include "../Tanks.h"
#include "TankRenderer.h"

static TankRoster g_roster;
static int g_tick = 0;

// ---------------------------------------------------------------------------
// Keyboard movement
// ---------------------------------------------------------------------------
// Arrow keys are hold-to-move controls: while a key is down the player tank
// keeps taking one grid step in that direction, and releasing every direction
// key stops it. The newest key that is still held wins, so tapping a second
// arrow changes direction without letting go of the first.
static Direction g_heldDirs[kDirections];  // held directions, newest last
static int g_heldCount = 0;

// Frames between grid steps while a direction key is held (~3 steps/second
// at the default 60 fps main loop). Lower this to move faster.
static constexpr int kStepIntervalFrames = 20;
static int g_stepCooldown = 0;

// Maps an arrow key name to a direction; returns false for any other key.
static bool directionForKey(const char* key, Direction& out)
{
    if (strcmp(key, "ArrowUp") == 0)    { out = Direction::Up;    return true; }
    if (strcmp(key, "ArrowDown") == 0)  { out = Direction::Down;  return true; }
    if (strcmp(key, "ArrowLeft") == 0)  { out = Direction::Left;  return true; }
    if (strcmp(key, "ArrowRight") == 0) { out = Direction::Right; return true; }
    return false;
}

static void holdDirection(Direction dir)
{
    for (int i = 0; i < g_heldCount; ++i)
    {
        if (g_heldDirs[i] != dir)
            continue;

        // Pressed again: make it the newest direction once more.
        for (int j = i; j + 1 < g_heldCount; ++j)
            g_heldDirs[j] = g_heldDirs[j + 1];
        g_heldDirs[g_heldCount - 1] = dir;
        g_stepCooldown = 0;
        return;
    }

    if (g_heldCount < kDirections)
        g_heldDirs[g_heldCount++] = dir;

    g_stepCooldown = 0;  // react to the key press right away
}

static void releaseDirection(Direction dir)
{
    for (int i = 0; i < g_heldCount; ++i)
    {
        if (g_heldDirs[i] != dir)
            continue;

        for (int j = i; j + 1 < g_heldCount; ++j)
            g_heldDirs[j] = g_heldDirs[j + 1];
        --g_heldCount;
        return;
    }
}

// One grid step for the player tank in the given direction.
static void stepPlayer(Direction dir)
{
    switch (dir)
    {
        case Direction::Up:    g_roster.player1.up();    break;
        case Direction::Down:  g_roster.player1.down();  break;
        case Direction::Left:  g_roster.player1.left();  break;
        case Direction::Right: g_roster.player1.right(); break;
    }
}

static EM_BOOL onKeyDown(int eventType, const EmscriptenKeyboardEvent* event, void* userData)
{
    if (!event || !event->key)
        return EM_FALSE;

    const char* key = event->key;

    if (strcmp(key, "p") == 0 || strcmp(key, "P") == 0)
    {
        g_roster.player2.levelUp();
        return EM_TRUE;
    }

    if (strcmp(key, "o") == 0 || strcmp(key, "O") == 0)
    {
        g_roster.player2.levelDown();
        return EM_TRUE;
    }

    Direction dir;
    if (!directionForKey(key, dir))
        return EM_FALSE;

    if (!event->repeat)
        holdDirection(dir);

    return EM_TRUE;  // consume arrow keys so the page does not scroll
}

static EM_BOOL onKeyUp(int eventType, const EmscriptenKeyboardEvent* event, void* userData)
{
    if (!event || !event->key)
        return EM_FALSE;

    Direction dir;
    if (!directionForKey(event->key, dir))
        return EM_FALSE;

    releaseDirection(dir);
    return EM_TRUE;
}

static void frame()
{
    glClearColor(0.07f, 0.08f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Drive the player tank from whichever direction key is currently held.
    if (g_heldCount > 0)
    {
        const Direction dir = g_heldDirs[g_heldCount - 1];
        g_roster.player1.face(dir);

        if (g_stepCooldown > 0)
            --g_stepCooldown;
        else
        {
            stepPlayer(dir);
            g_stepCooldown = kStepIntervalFrames;
        }
    }

    if (EmscriptenTankRenderer::spritesReady())
    {
        const int frame = (g_tick / 15) % kFramesPerDir;
        g_roster.player1.setFrame(frame);
        g_roster.player2.setFrame(frame);
        g_roster.enemy1.setFrame(frame);
        g_roster.enemy2.setFrame(frame);
        g_roster.enemy3.setFrame(frame);
        g_roster.enemy4.setFrame(frame);

        static const Direction kSpin[4] = {
            Direction::Up, Direction::Right, Direction::Down, Direction::Left
        };
        g_roster.enemy3.face(kSpin[(g_tick / 60) % 4]);

        for (Tank* tank : tanksOf(g_roster))
            EmscriptenTankRenderer::render(*tank);
    }

    ++g_tick;
}

int main()
{
    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx =
        emscripten_webgl_create_context("#canvas", &attr);

    emscripten_webgl_make_context_current(ctx);
    EmscriptenTankRenderer::init("sprites.png");

    g_roster.player1.place(1 *4 , 1*4);  g_roster.player1.face(Direction::Up);
    g_roster.player2.place(3*4, 1*4);  g_roster.player2.face(Direction::Right);
    g_roster.enemy1.place(6*4, 4*4);   g_roster.enemy1.face(Direction::Down);
    g_roster.enemy2.place(8*4, 2*4);   g_roster.enemy2.face(Direction::Left);
    g_roster.enemy3.place(11*4, 3*4);  g_roster.enemy3.face(Direction::Up);
    g_roster.enemy4.place(4*4, 2*4);  g_roster.enemy4.face(Direction::Down);

    g_roster.player2.levelUp();

    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, 1, onKeyDown);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, 1, onKeyUp);
    emscripten_set_main_loop(frame, 0, 0);
    return 0;
}

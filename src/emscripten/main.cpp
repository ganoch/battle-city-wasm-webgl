#include <GLES3/gl3.h>

#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/html5_webgl.h>

#include "../Tanks.h"
#include "TankRenderer.h"

static TankRoster g_roster;
static int g_tick = 0;

static EM_BOOL onKeyDown(int eventType, const EmscriptenKeyboardEvent* event, void* userData)
{
    if (!event || !event->key)
        return EM_FALSE;

    const char* key = event->key;
    
    if (strcmp(key, "p") == 0 || strcmp(key, "P") == 0)
        g_roster.player2.levelUp();
    else if (strcmp(key, "o") == 0 || strcmp(key, "O") == 0)
        g_roster.player2.levelDown();
    else if (strcmp(key, "ArrowUp") == 0)
        g_roster.player1.face(Direction::Up);
    else if (strcmp(key, "ArrowDown") == 0)
        g_roster.player1.face(Direction::Down);
    else if (strcmp(key, "ArrowLeft") == 0)
        g_roster.player1.face(Direction::Left);
    else if (strcmp(key, "ArrowRight") == 0)
        g_roster.player1.face(Direction::Right);


    return EM_FALSE;
}

static void frame()
{
    glClearColor(0.07f, 0.08f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

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

    g_roster.player1.place(1, 1);  g_roster.player1.face(Direction::Up);
    g_roster.player2.place(3, 1);  g_roster.player2.face(Direction::Right);
    g_roster.enemy1.place(6, 4);   g_roster.enemy1.face(Direction::Down);
    g_roster.enemy2.place(8, 2);   g_roster.enemy2.face(Direction::Left);
    g_roster.enemy3.place(11, 3);  g_roster.enemy3.face(Direction::Up);
    g_roster.enemy4.place(4, 2);  g_roster.enemy4.face(Direction::Down);

    g_roster.player2.levelUp();

    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, 1, onKeyDown);
    emscripten_set_main_loop(frame, 0, 0);
    return 0;
}

#include <emscripten.h>
#include <GLES3/gl3.h>
#include <emscripten/html5_webgl.h>

EMSCRIPTEN_KEEPALIVE
void render()
{
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

int main()
{
    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx =
        emscripten_webgl_create_context("#canvas", &attr);

    emscripten_webgl_make_context_current(ctx);

    render();
}
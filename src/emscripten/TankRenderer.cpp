#include "TankRenderer.h"

#include <GLES3/gl3.h>

#include <cstdlib>

#include <emscripten.h>
#include <emscripten/html5.h>

namespace
{
constexpr float kTilePx = 32.0f;

GLuint g_program = 0;
GLuint g_vao = 0;
GLuint g_vbo = 0;
GLuint g_texture = 0;

GLint g_uDestRect = -1;
GLint g_uViewport = -1;
GLint g_uSrcRect = -1;
GLint g_uTexSize = -1;
GLint g_uTexture = -1;

int g_viewW = 800;
int g_viewH = 600;
float g_sheetW = 1.0f;
float g_sheetH = 1.0f;
bool g_ready = false;

const char* kVertexSrc = R"GLSL(#version 300 es
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
uniform vec4 uDestRect;
uniform vec2 uViewport;
uniform vec4 uSrcRect;
uniform vec2 uTexSize;
out vec2 vUV;
void main()
{
    vec2 pixel = uDestRect.xy + aPos * uDestRect.zw;
    vec2 ndc = vec2((pixel.x / uViewport.x) * 2.0 - 1.0,
                    1.0 - (pixel.y / uViewport.y) * 2.0);
    gl_Position = vec4(ndc, 0.0, 1.0);
    vUV = (uSrcRect.xy + aUV * uSrcRect.zw) / uTexSize;
}
)GLSL";

const char* kFragmentSrc = R"GLSL(#version 300 es
precision mediump float;
uniform sampler2D uTexture;
in vec2 vUV;
out vec4 fragColor;
void main()
{
    fragColor = texture(uTexture, vUV);
}
)GLSL";

GLuint compileShader(GLenum type, const char* src)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        emscripten_log(EM_LOG_ERROR, "tank sprite shader failed to compile: %s", log);
    }
    return shader;
}
} // namespace

EM_JS(void, jsLoadSpriteSheet, (const char* url), {
    var src = UTF8ToString(url);
    var img = new Image();
    img.onload = function () {
        var canvas = document.createElement('canvas');
        canvas.width = img.naturalWidth;
        canvas.height = img.naturalHeight;
        var ctx = canvas.getContext('2d');
        ctx.drawImage(img, 0, 0);
        var data = ctx.getImageData(0, 0, canvas.width, canvas.height).data;

        // Remove the near-black background used by the sprite artwork.
        for (var i = 0; i < data.length; i += 4) {
            if (data[i] === 0 && data[i + 1] === 0 &&
                (data[i + 2] === 0 || data[i + 2] === 1)) {
                data[i + 3] = 0;
            }
        }

        var ptr = _malloc(data.length);
        HEAPU8.set(data, ptr);
        Module['__spriteSheetPtr'] = ptr;
        Module['__spriteSheetW'] = canvas.width;
        Module['__spriteSheetH'] = canvas.height;
        Module['__spriteSheetReady'] = true;
    };
    img.onerror = function () {
        console.error('Failed to load sprite sheet: ' + src);
    };
    img.src = src;
});

EM_JS(int, jsSpriteSheetReady, (), {
    return (typeof Module !== 'undefined' && Module['__spriteSheetReady']) ? 1 : 0;
});

EM_JS(int, jsSpriteSheetPtr, (), {
    return (typeof Module !== 'undefined' && Module['__spriteSheetPtr']) | 0;
});

EM_JS(int, jsSpriteSheetWidth, (), {
    return (typeof Module !== 'undefined' && Module['__spriteSheetW']) | 0;
});

EM_JS(int, jsSpriteSheetHeight, (), {
    return (typeof Module !== 'undefined' && Module['__spriteSheetH']) | 0;
});

void EmscriptenTankRenderer::init(const char* url)
{
    emscripten_get_canvas_element_size("#canvas", &g_viewW, &g_viewH);

    const GLuint vs = compileShader(GL_VERTEX_SHADER, kVertexSrc);
    const GLuint fs = compileShader(GL_FRAGMENT_SHADER, kFragmentSrc);

    g_program = glCreateProgram();
    glAttachShader(g_program, vs);
    glAttachShader(g_program, fs);
    glLinkProgram(g_program);
    glDeleteShader(vs);
    glDeleteShader(fs);

    g_uDestRect = glGetUniformLocation(g_program, "uDestRect");
    g_uViewport = glGetUniformLocation(g_program, "uViewport");
    g_uSrcRect = glGetUniformLocation(g_program, "uSrcRect");
    g_uTexSize = glGetUniformLocation(g_program, "uTexSize");
    g_uTexture = glGetUniformLocation(g_program, "uTexture");

    static const float kQuad[] = {
        0.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
    };
    glGenVertexArrays(1, &g_vao);
    glBindVertexArray(g_vao);
    glGenBuffers(1, &g_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kQuad), kQuad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          reinterpret_cast<const void*>(2 * sizeof(float)));
    glBindVertexArray(0);

    glGenTextures(1, &g_texture);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    jsLoadSpriteSheet(url);
}

bool EmscriptenTankRenderer::spritesReady()
{
    if (!g_ready && jsSpriteSheetReady())
    {
        const int w = jsSpriteSheetWidth();
        const int h = jsSpriteSheetHeight();
        void* pixels = reinterpret_cast<void*>(jsSpriteSheetPtr());

        glBindTexture(GL_TEXTURE_2D, g_texture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        std::free(pixels);

        g_sheetW = static_cast<float>(w);
        g_sheetH = static_cast<float>(h);
        g_ready = true;
    }
    return g_ready;
}

void EmscriptenTankRenderer::render(const Tank& tank)
{
    if (!g_ready)
        return;

    const SpriteRect src = tank.sprite();
    const float left = static_cast<float>(tank.x()) * kTilePx;
    const float top = static_cast<float>(g_viewH) -
                      (static_cast<float>(tank.y()) + 1.0f) * kTilePx;

    glUseProgram(g_program);
    glUniform4f(g_uDestRect, left, top, kTilePx, kTilePx);
    glUniform2f(g_uViewport, static_cast<float>(g_viewW), static_cast<float>(g_viewH));
    glUniform4f(g_uSrcRect, static_cast<float>(src.x), static_cast<float>(src.y),
                static_cast<float>(src.w), static_cast<float>(src.h));
    glUniform2f(g_uTexSize, g_sheetW, g_sheetH);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_texture);
    glUniform1i(g_uTexture, 0);

    glBindVertexArray(g_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

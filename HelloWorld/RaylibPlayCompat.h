#pragma once

#include "raylib.h"
#include <cmath>
#include <vector>
#include <cstdlib>

// =========================
// Play Namespace
// =========================
namespace Play {

// -------------------------
// Types
// -------------------------
struct Point2f {
    float x, y;
    Point2f(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}
    Point2f(int x, int y) : x(static_cast<float>(x)), y(static_cast<float>(y)) {}
    explicit operator Vector2() const { return { x, y }; }
};

typedef Color Colour;

// -------------------------
// Colour Constants
// -------------------------
constexpr auto cBlack = BLACK;
constexpr auto cWhite = WHITE;
constexpr auto cRed = RED;
constexpr auto cGreen = GREEN;
constexpr auto cBlue = BLUE;
constexpr auto cYellow = YELLOW;
constexpr auto cMagenta = MAGENTA;
constexpr auto cOrange = ORANGE;
constexpr Colour cCyan = { 0, 255, 255, 255 };

// -------------------------
// Key Constants
// -------------------------
constexpr int KEY_UP = ::KEY_UP;
constexpr int KEY_DOWN = ::KEY_DOWN;
constexpr int KEY_LEFT = ::KEY_LEFT;
constexpr int KEY_RIGHT = ::KEY_RIGHT;
constexpr int KEY_ESCAPE = ::KEY_ESCAPE;

// -------------------------
// Public API
// -------------------------
inline void CreateManager(const int displayWidth, const int displayHeight, const int displayScale);
inline void DestroyManager();
inline bool KeyDown(const int key);
inline void ClearDrawingBuffer(const Colour& colour);
inline void PresentDrawingBuffer();
inline void DrawRect(const Point2f& topLeft, const Point2f& bottomRight, const Colour& colour, bool filled = true);
inline void DrawCircle(const Point2f& center, int radius, const Colour& colour);
inline void DrawDebugText(const Point2f& pos, const char* text);

// -------------------------
// Internal Details
// -------------------------
namespace Internal {
    static int g_displayWidth = 0;
    static int g_displayHeight = 0;
    static int g_displayScale = 1;
    static RenderTexture2D g_renderTexture;
    static bool g_textureInitialized = false;
}

// -------------------------
// Implementation
// -------------------------

inline void CreateManager(const int displayWidth, const int displayHeight, const int displayScale) {
    Internal::g_displayWidth = displayWidth;
    Internal::g_displayHeight = displayHeight;
    Internal::g_displayScale = displayScale;
    InitWindow(displayWidth * displayScale, displayHeight * displayScale, "Pacman Game");
    SetTargetFPS(60);
    Internal::g_renderTexture = LoadRenderTexture(displayWidth, displayHeight);
    Internal::g_textureInitialized = true;
}

inline void DestroyManager() {
    if (Internal::g_textureInitialized) {
        UnloadRenderTexture(Internal::g_renderTexture);
        Internal::g_textureInitialized = false;
    }
    CloseWindow();
}

inline bool KeyDown(const int key) {
    return IsKeyDown(key);
}

inline void ClearDrawingBuffer(const Colour& colour) {
    BeginTextureMode(Internal::g_renderTexture);
    ClearBackground(colour);
}

inline void PresentDrawingBuffer() {
    EndTextureMode();
    BeginDrawing();
    ClearBackground(BLACK);
    const Rectangle source{ 0, 0, static_cast<float>(Internal::g_displayWidth), -static_cast<float>(Internal::g_displayHeight) };
    const Rectangle dest{ 0, 0, static_cast<float>(Internal::g_displayWidth * Internal::g_displayScale), static_cast<float>(Internal::g_displayHeight * Internal::g_displayScale) };
    DrawTexturePro(Internal::g_renderTexture.texture, source, dest, { 0, 0 }, 0.0f, WHITE);
    EndDrawing();
}

inline void DrawRect(const Point2f& topLeft, const Point2f& bottomRight, const Colour& colour, bool filled) {
    const int x = static_cast<int>(topLeft.x);
    const int y = static_cast<int>(topLeft.y);
    const int w = static_cast<int>(bottomRight.x - topLeft.x + 1);
    const int h = static_cast<int>(bottomRight.y - topLeft.y + 1);
    if (filled) DrawRectangle(x, y, w, h, colour);
    else        DrawRectangleLines(x, y, w, h, colour);
}

inline void DrawCircle(const Point2f& center, int radius, const Colour& colour) {
    DrawCircleV(static_cast<Vector2>(center), static_cast<float>(radius), colour);
}

inline void DrawDebugText(const Point2f& pos, const char* text, int fontSize = 20, Colour col = cWhite)
{
    const int textWidth = MeasureText(text, fontSize);
    const int x = static_cast<int>(pos.x - static_cast<float>(textWidth) / 2);
    const int y = static_cast<int>(pos.y - static_cast<float>(fontSize) / 2);
    DrawText(text, x, y, fontSize, col);
}

} // namespace Play
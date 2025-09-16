#include "RaylibPlayCompat.h"

// Forward declarations for PlayBuffer-style callbacks
void MainGameEntry();
bool MainGameUpdate(float elapsed);
int MainGameExit();

// Main function that PlayBuffer would normally provide
int main() {
    MainGameEntry();

    while (!WindowShouldClose() && !MainGameUpdate(GetFrameTime())) {
        // Game loop handled by callbacks
    }

    return MainGameExit();
}

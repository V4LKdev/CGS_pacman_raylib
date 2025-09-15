#include "RaylibPlayCompat.h"

// Forward declarations for PlayBuffer-style callbacks
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE);
bool MainGameUpdate(float elapsed);
int MainGameExit();

// Main function that PlayBuffer would normally provide
int main() {
    MainGameEntry(PLAY_IGNORE_COMMAND_LINE);

    while (!WindowShouldClose() && !MainGameUpdate(GetFrameTime())) {
        // Game loop handled by callbacks
    }

    return MainGameExit();
}

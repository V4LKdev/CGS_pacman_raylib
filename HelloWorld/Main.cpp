// Defines for Playbuffer
#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER

// Includes
#include "Play.h"
#include "Game.h"

// Our game instance
static Game GameInstance;

void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(Cfg::DISPLAY_W, Cfg::DISPLAY_H, Cfg::DISPLAY_SCALE);
	GameInstance.Init();
}

bool MainGameUpdate(float elapsed)
{
	Play::ClearDrawingBuffer(Play::cBlack);

	GameInstance.Update(elapsed);
	GameInstance.Draw();

	Play::PresentDrawingBuffer();

	return Play::KeyDown(KEY_ESCAPE);
}

int MainGameExit()
{
	Play::DestroyManager();
	return PLAY_OK;
}

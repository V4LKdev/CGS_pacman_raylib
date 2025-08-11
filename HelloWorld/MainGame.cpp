#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

const int DISPLAY_WIDTH = 640;
const int DISPLAY_HEIGHT = 360;
const int DISPLAY_SCALE = 2;

const int TILE_SIZE = 16;
const int GRID_WIDTH = DISPLAY_WIDTH / TILE_SIZE;
const int GRID_HEIGHT = DISPLAY_HEIGHT / TILE_SIZE;

enum TileType { TILE_EMPTY, TILE_WALL, TILE_PELLET };
TileType maze[GRID_HEIGHT][GRID_WIDTH];

Play::Point2f playerPos;
Play::Point2f playerDir;
Play::Point2f queuedDir;

void InitMaze()
{
	for (int y = 0; y < GRID_HEIGHT; y++)
	{
		for (int x = 0; x < GRID_WIDTH; x++)
		{
			if (x == 0 || y == 0 || x == GRID_WIDTH - 1 || y == GRID_HEIGHT - 1)
				maze[y][x] = TILE_WALL;
			else
				maze[y][x] = TILE_PELLET;
		}
	}
}

void DrawMaze()
{
	for (int y = 0; y < GRID_HEIGHT; y++)
	{
		for (int x = 0; x < GRID_WIDTH; x++)
		{
			int px = x * TILE_SIZE;
			int py = y * TILE_SIZE;

			if (maze[y][x] == TILE_WALL)
				Play::DrawRect({ px, py }, { px + TILE_SIZE - 1, py + TILE_SIZE - 1 }, Play::cBlue, true);
			else if (maze[y][x] == TILE_PELLET)
				Play::DrawCircle({ px + TILE_SIZE / 2, py + TILE_SIZE / 2 }, 2, Play::cWhite);
		}
	}
}

bool IsWall(int gx, int gy)
{
	if (gx < 0 || gy < 0 || gx >= GRID_WIDTH || gy >= GRID_HEIGHT)
		return true; // Treat out-of-bounds as wall
	return maze[gy][gx] == TILE_WALL;
}

Play::Point2f PixelToGrid(const Play::Point2f& pos)
{
	return { int(pos.x / TILE_SIZE), int(pos.y / TILE_SIZE) };
}

bool IsAlignedToGrid(const Play::Point2f& pos)
{
	return (int(pos.x) % TILE_SIZE == TILE_SIZE / 2 && int(pos.y) % TILE_SIZE == TILE_SIZE / 2);
}

void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	InitMaze();

	playerPos = { TILE_SIZE * 5 + TILE_SIZE / 2, TILE_SIZE * 5 + TILE_SIZE / 2 };
	playerDir = { 0, 0 };
	queuedDir = { 0, 0 };
}

bool MainGameUpdate(float elapsedTime)
{
	Play::ClearDrawingBuffer(Play::cBlack);

	// Input (y+ is up)
	if (Play::KeyDown(KEY_UP))    queuedDir = { 0,  1 };
	if (Play::KeyDown(KEY_DOWN))  queuedDir = { 0, -1 };
	if (Play::KeyDown(KEY_LEFT))  queuedDir = { -1, 0 };
	if (Play::KeyDown(KEY_RIGHT)) queuedDir = { 1,  0 };

	Play::Point2f gridPos = PixelToGrid(playerPos);

	// Try to change direction at center
	if (IsAlignedToGrid(playerPos))
	{
		int nx = int(gridPos.x) + int(queuedDir.x);
		int ny = int(gridPos.y) + int(queuedDir.y);
		if (!IsWall(nx, ny))
			playerDir = queuedDir;
	}

	// Collision check for current direction before moving
	int nextX = int(gridPos.x) + int(playerDir.x);
	int nextY = int(gridPos.y) + int(playerDir.y);

	bool canMove = !IsWall(nextX, nextY) || !IsAlignedToGrid(playerPos);

	if (canMove)
	{
		playerPos += playerDir * (TILE_SIZE * elapsedTime * 6);
	}
	else
	{
		// Snap to center of current tile if blocked
		playerPos = { gridPos.x * TILE_SIZE + TILE_SIZE / 2, gridPos.y * TILE_SIZE + TILE_SIZE / 2 };
	}

	// Pellet collection
	gridPos = PixelToGrid(playerPos);
	if (maze[int(gridPos.y)][int(gridPos.x)] == TILE_PELLET)
		maze[int(gridPos.y)][int(gridPos.x)] = TILE_EMPTY;

	DrawMaze();
	Play::DrawCircle(playerPos, TILE_SIZE / 2 - 2, Play::cYellow);

	// Win check
	bool pelletsLeft = false;
	for (int y = 0; y < GRID_HEIGHT && !pelletsLeft; y++)
		for (int x = 0; x < GRID_WIDTH; x++)
			if (maze[y][x] == TILE_PELLET)
				pelletsLeft = true;

	if (!pelletsLeft)
		Play::DrawDebugText({ DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 }, "YOU WIN!");

	Play::PresentDrawingBuffer();
	return Play::KeyDown(KEY_ESCAPE);
}

int MainGameExit(void)
{
	Play::DestroyManager();
	return PLAY_OK;
}

#include "SafetyMap.h"
#include "Room.h"
#include "Grenade.h"
#include <cstring>
#include <cstdlib>

SafetyMap::SafetyMap()
{
	reset();
}

void SafetyMap::reset()
{
	memset(cellRisk, 0, sizeof(cellRisk));
	memset(roomRisk, 0, sizeof(roomRisk));
}

void SafetyMap::updateRoomRisk(int roomIndex, double risk)
{
	if (roomIndex >= 0 && roomIndex < NUM_ROOMS)
		roomRisk[roomIndex] = risk;
}

// fake grenade blasts at random spots to build up cell-level risk
void SafetyMap::addCombatRisk(int roomIndex, Room* room, int maze[MSZ][MSZ])
{
	int samples = 5;
	int halfH = room->getHeight() / 4;
	int halfW = room->getWidth() / 4;
	if (halfH < 1) halfH = 1;
	if (halfW < 1) halfW = 1;

	for (int i = 0; i < samples; i++)
	{
		int r = room->getCenterRow() - halfH + rand() % (halfH * 2 + 1);
		int c = room->getCenterCol() - halfW + rand() % (halfW * 2 + 1);
		Grenade g((double)c, (double)r);
		g.CreateSecurityMap(cellRisk, maze);
	}

	roomRisk[roomIndex] += 0.3;
	if (roomRisk[roomIndex] > 1.0)
		roomRisk[roomIndex] = 1.0;
}

void SafetyMap::decayRisk()
{
	for (int i = 0; i < MSZ; i++)
		for (int j = 0; j < MSZ; j++)
			cellRisk[i][j] *= 0.995;

	for (int i = 0; i < NUM_ROOMS; i++)
		roomRisk[i] *= 0.998;
}

double SafetyMap::getCellRisk(int r, int c)
{
	if (r >= 0 && r < MSZ && c >= 0 && c < MSZ)
		return cellRisk[r][c];
	return 1.0;
}

double SafetyMap::getRoomRisk(int roomIndex)
{
	if (roomIndex >= 0 && roomIndex < NUM_ROOMS)
		return roomRisk[roomIndex];
	return 1.0;
}

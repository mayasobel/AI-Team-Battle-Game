#pragma once
#include "Globals.h"

class Room;

class SafetyMap
{
public:
	double cellRisk[MSZ][MSZ];
	double roomRisk[NUM_ROOMS];

	SafetyMap();
	void reset();
	void updateRoomRisk(int roomIndex, double risk);
	void addCombatRisk(int roomIndex, Room* room, int maze[MSZ][MSZ]);
	void decayRisk();
	double getCellRisk(int r, int c);
	double getRoomRisk(int roomIndex);
};

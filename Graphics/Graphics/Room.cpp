#include "Room.h"
#include <cstdlib>

Room::Room(int cr, int cc, int w, int h)
{
	center_row = cr;
	center_col = cc;
	width = w;
	height = h;
	depot = DEPOT_NONE;
	riskScore = 0;
}

void Room::addConnection(int roomIndex)
{
	for (size_t i = 0; i < connectedRooms.size(); i++)
		if (connectedRooms[i] == roomIndex) return;
	connectedRooms.push_back(roomIndex);
}

bool Room::isInside(double px, double py)
{
	int startCol = center_col - width / 2;
	int endCol = center_col + width / 2;
	int startRow = center_row - height / 2;
	int endRow = center_row + height / 2;
	return px >= startCol && px <= endCol && py >= startRow && py <= endRow;
}

void Room::addObstacles(int maze[MSZ][MSZ])
{
	int numObs = 1 + rand() % 3;
	int sr = center_row - height / 2 + 2;
	int er = center_row + height / 2 - 2;
	int sc = center_col - width / 2 + 2;
	int ec = center_col + width / 2 - 2;

	if (er <= sr || ec <= sc) return;

	for (int n = 0; n < numObs; n++)
	{
		int r = sr + rand() % (er - sr);
		int c = sc + rand() % (ec - sc);
		int ow = 1 + rand() % 2;
		int oh = 1 + rand() % 2;

		for (int i = r; i < r + oh && i < er; i++)
			for (int j = c; j < c + ow && j < ec; j++)
				if (i != center_row || j != center_col)
					maze[i][j] = WALL;
	}
}

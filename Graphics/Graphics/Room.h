#pragma once
#include "Globals.h"
#include <vector>

class Room
{
private:
	int center_row, center_col;
	int width, height;
	DepotType depot;
	double riskScore;
	std::vector<int> connectedRooms;

public:
	Room(int cr, int cc, int w, int h);
	int getCenterRow() { return center_row; }
	int getCenterCol() { return center_col; }
	int getWidth() { return width; }
	int getHeight() { return height; }

	void setDepot(DepotType d) { depot = d; }
	DepotType getDepot() { return depot; }
	void setRiskScore(double r) { riskScore = r; }
	double getRiskScore() { return riskScore; }

	void addConnection(int roomIndex);
	std::vector<int>& getConnections() { return connectedRooms; }
	bool isInside(double px, double py);
	void addObstacles(int maze[MSZ][MSZ]);
};

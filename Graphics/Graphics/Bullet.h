#pragma once
#include "Globals.h"

class Bullet
{
private:
	double x, y;
	double dirx, diry;
	bool isMoving;
public:
	Bullet(double xx, double yy, double angle);
	void show();
	void move(int maze[MSZ][MSZ]);
	void setIsMoving(bool value) { isMoving = value; }
	bool getIsMoving() { return isMoving; }
	double getX() { return x; }
	double getY() { return y; }
	void CreateSecurityMap(double smap[MSZ][MSZ], int maze[MSZ][MSZ]);
};

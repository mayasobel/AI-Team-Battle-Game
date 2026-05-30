#pragma once
#include "Bullet.h"

class Grenade
{
private:
	Bullet* bullets[NUM_GRENADE_BULLETS];
	double x, y;
	bool isExploding;
	int team;
public:
	Grenade(double xx, double yy, int teamId = 0);
	~Grenade();
	void show();
	void setIsExploding(bool value) { isExploding = value; }
	void explode(int maze[MSZ][MSZ]);
	void startExploding();
	bool getIsExploding() { return isExploding; }
	void CreateSecurityMap(double smap[MSZ][MSZ], int maze[MSZ][MSZ]);
};

#include "Grenade.h"
#include "glut.h"
#include <math.h>

const double PI = 3.14159265;

Grenade::Grenade(double xx, double yy, int teamId)
{
	int i;
	double alpha;
	double delta = 2 * PI / NUM_GRENADE_BULLETS;
	x = xx;
	y = yy;
	isExploding = false;
	team = teamId;

	for (i = 0, alpha = 0; i < NUM_GRENADE_BULLETS; i++, alpha += delta)
		bullets[i] = new Bullet(x, y, alpha);
}

Grenade::~Grenade()
{
	for (int i = 0; i < NUM_GRENADE_BULLETS; i++)
		delete bullets[i];
}

// color matches the team that threw it
void Grenade::show()
{
	for (int i = 0; i < NUM_GRENADE_BULLETS; i++)
	{
		if (!bullets[i]->getIsMoving()) continue;
		double bx = bullets[i]->getX();
		double by = bullets[i]->getY();

		if (team == 0)
			glColor3d(0.2, 0.5, 1.0);
		else
			glColor3d(1.0, 0.4, 0.2);

		glBegin(GL_POLYGON);
		glVertex2d(bx - 0.25, by - 0.25);
		glVertex2d(bx - 0.25, by + 0.25);
		glVertex2d(bx + 0.25, by + 0.25);
		glVertex2d(bx + 0.25, by - 0.25);
		glEnd();
	}
}

void Grenade::explode(int maze[MSZ][MSZ])
{
	for (int i = 0; i < NUM_GRENADE_BULLETS; i++)
		bullets[i]->move(maze);
}

void Grenade::startExploding()
{
	for (int i = 0; i < NUM_GRENADE_BULLETS; i++)
		bullets[i]->setIsMoving(isExploding);
}

void Grenade::CreateSecurityMap(double smap[MSZ][MSZ], int maze[MSZ][MSZ])
{
	for (int i = 0; i < NUM_GRENADE_BULLETS; i++)
		bullets[i]->CreateSecurityMap(smap, maze);
}

#include <math.h>
#include "Bullet.h"
#include "glut.h"

Bullet::Bullet(double xx, double yy, double angle)
{
	x = xx;
	y = yy;
	dirx = cos(angle);
	diry = sin(angle);
	isMoving = false;
}

void Bullet::show()
{
	glColor3d(1, 0, 0);
	glBegin(GL_POLYGON);
	glVertex2d(x - 0.5, y);
	glVertex2d(x, y + 0.5);
	glVertex2d(x + 0.5, y);
	glVertex2d(x, y - 0.5);
	glEnd();

	glColor3d(0, 0, 0);
	glBegin(GL_LINE_LOOP);
	glVertex2d(x - 0.5, y);
	glVertex2d(x, y + 0.5);
	glVertex2d(x + 0.5, y);
	glVertex2d(x, y - 0.5);
	glEnd();
}

void Bullet::move(int maze[MSZ][MSZ])
{
	if (isMoving)
	{
		x += dirx * BULLET_SPEED;
		y += diry * BULLET_SPEED;
		int ix = (int)x;
		int iy = (int)y;
		if (ix < 0 || ix >= MSZ || iy < 0 || iy >= MSZ || maze[iy][ix] == WALL)
			isMoving = false;
	}
}

// walks the bullet forward and adds risk to every cell it passes through
void Bullet::CreateSecurityMap(double smap[MSZ][MSZ], int maze[MSZ][MSZ])
{
	isMoving = true;
	while (isMoving)
	{
		int ix = (int)x;
		int iy = (int)y;
		if (ix >= 0 && ix < MSZ && iy >= 0 && iy < MSZ)
			smap[iy][ix] += SECURITY_SCORE;

		x += dirx * BULLET_SPEED;
		y += diry * BULLET_SPEED;
		ix = (int)x;
		iy = (int)y;
		if (ix < 0 || ix >= MSZ || iy < 0 || iy >= MSZ || maze[iy][ix] == WALL)
			isMoving = false;
	}
}

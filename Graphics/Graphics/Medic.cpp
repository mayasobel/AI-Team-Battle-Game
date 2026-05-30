#include "Medic.h"
#include "SupportStates.h"
#include "glut.h"
#include <math.h>
#include <cstdlib>

Medic::Medic(int team, int id, int startRoom, double startX, double startY)
	: BaseAgent(team, id, startRoom, startX, startY)
{
	healCooldown = 0;
	healSupply = MEDIC_MAX_SUPPLY;
	currentClientId = -1;
	personality.riskAversion = 0.7 + (rand() % 20) / 100.0;
	setCurrentState(new IdleSupport());
}

void Medic::show()
{
	if (!alive) return;

	if (teamId == 0)
		glColor3d(0.2, 0.5, 1.0);
	else
		glColor3d(1.0, 0.5, 0.2);

	glBegin(GL_POLYGON);
	glVertex2d(x - 0.4, y - 1.2);
	glVertex2d(x - 0.4, y + 1.2);
	glVertex2d(x + 0.4, y + 1.2);
	glVertex2d(x + 0.4, y - 1.2);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex2d(x - 1.2, y - 0.4);
	glVertex2d(x - 1.2, y + 0.4);
	glVertex2d(x + 1.2, y + 0.4);
	glVertex2d(x + 1.2, y - 0.4);
	glEnd();

	glColor3d(1, 1, 1);
	glBegin(GL_POLYGON);
	glVertex2d(x - 0.2, y - 0.8);
	glVertex2d(x - 0.2, y + 0.8);
	glVertex2d(x + 0.2, y + 0.8);
	glVertex2d(x + 0.2, y - 0.8);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex2d(x - 0.8, y - 0.2);
	glVertex2d(x - 0.8, y + 0.2);
	glVertex2d(x + 0.8, y + 0.2);
	glVertex2d(x + 0.8, y - 0.2);
	glEnd();

	// health icon
	glColor3d(0.9, 0.1, 0.1);
	glLineWidth(3.0f);
	glBegin(GL_LINES);
	glVertex2d(x - 2.8, y + 2.25);
	glVertex2d(x - 1.4, y + 2.25);
	glVertex2d(x - 2.1, y + 1.85);
	glVertex2d(x - 2.1, y + 2.65);
	glEnd();
	glLineWidth(1.0f);

	// health bar
	glColor3d(0.3, 0.0, 0.0);
	glBegin(GL_POLYGON);
	glVertex2d(x - 1, y + 2);
	glVertex2d(x - 1, y + 2.5);
	glVertex2d(x + 1, y + 2.5);
	glVertex2d(x + 1, y + 2);
	glEnd();

	double hpRatio = health / MAX_HEALTH;
	glColor3d(1 - hpRatio, hpRatio, 0);
	glBegin(GL_POLYGON);
	glVertex2d(x - 1, y + 2);
	glVertex2d(x - 1, y + 2.5);
	glVertex2d(x - 1 + 2 * hpRatio, y + 2.5);
	glVertex2d(x - 1 + 2 * hpRatio, y + 2);
	glEnd();

	glColor3d(0, 0, 0);
	glBegin(GL_LINE_LOOP);
	glVertex2d(x - 1, y + 2);
	glVertex2d(x - 1, y + 2.5);
	glVertex2d(x + 1, y + 2.5);
	glVertex2d(x + 1, y + 2);
	glEnd();

	// supply icon
	glColor3d(0.1, 0.8, 0.2);
	glLineWidth(3.0f);
	glBegin(GL_LINES);
	glVertex2d(x - 2.8, y + 3.05);
	glVertex2d(x - 1.4, y + 3.05);
	glVertex2d(x - 2.1, y + 2.65);
	glVertex2d(x - 2.1, y + 3.45);
	glEnd();
	glLineWidth(1.0f);

	// supply bar
	glColor3d(0.15, 0.15, 0.15);
	glBegin(GL_POLYGON);
	glVertex2d(x - 1, y + 2.8);
	glVertex2d(x - 1, y + 3.3);
	glVertex2d(x + 1, y + 3.3);
	glVertex2d(x + 1, y + 2.8);
	glEnd();

	double supplyRatio = (double)healSupply / MEDIC_MAX_SUPPLY;
	glColor3d(0.2, 0.9, 0.3);
	glBegin(GL_POLYGON);
	glVertex2d(x - 1, y + 2.8);
	glVertex2d(x - 1, y + 3.3);
	glVertex2d(x - 1 + 2 * supplyRatio, y + 3.3);
	glVertex2d(x - 1 + 2 * supplyRatio, y + 2.8);
	glEnd();

	glColor3d(0, 0, 0);
	glBegin(GL_LINE_LOOP);
	glVertex2d(x - 1, y + 2.8);
	glVertex2d(x - 1, y + 3.3);
	glVertex2d(x + 1, y + 3.3);
	glVertex2d(x + 1, y + 2.8);
	glEnd();
}

void Medic::update()
{
	if (!alive) return;
	if (healCooldown > 0) healCooldown--;
	doMovement();
	if (pCurrentState)
		pCurrentState->Execute(this);
}

// can't heal self (target == this check)
void Medic::healTarget(BaseAgent* target)
{
	if (!alive) return;
	if (target == nullptr || !target->isAlive()) return;
	if (target == this) return;
	if (target->getHealth() >= MAX_HEALTH) return;
	if (healSupply <= 0) return;

	target->setHealth(MAX_HEALTH);
	healSupply--;
}

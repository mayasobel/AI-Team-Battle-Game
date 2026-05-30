#include "Supplier.h"
#include "Fighter.h"
#include "SupportStates.h"
#include "glut.h"
#include <cstdlib>

Supplier::Supplier(int team, int id, int startRoom, double startX, double startY)
	: BaseAgent(team, id, startRoom, startX, startY)
{
	supplyCooldown = 0;
	ammoInventory = SUPPLIER_MAX_SUPPLY;
	currentClientId = -1;
	personality.riskAversion = 0.7 + (rand() % 20) / 100.0;
	setCurrentState(new IdleSupport());
}

void Supplier::show()
{
	if (!alive) return;

	if (teamId == 0)
		glColor3d(0.1, 0.4, 0.8);
	else
		glColor3d(0.8, 0.4, 0.1);

	glBegin(GL_POLYGON);
	glVertex2d(x, y + 1.5);
	glVertex2d(x - 1.2, y - 1);
	glVertex2d(x + 1.2, y - 1);
	glEnd();

	if (teamId == 0)
		glColor3d(0.3, 0.6, 1.0);
	else
		glColor3d(1.0, 0.6, 0.3);
	glBegin(GL_POLYGON);
	glVertex2d(x, y + 0.8);
	glVertex2d(x - 0.6, y - 0.5);
	glVertex2d(x + 0.6, y - 0.5);
	glEnd();

	glColor3d(0, 0, 0);
	glBegin(GL_LINE_LOOP);
	glVertex2d(x, y + 1.5);
	glVertex2d(x - 1.2, y - 1);
	glVertex2d(x + 1.2, y - 1);
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

	// ammo icon
	glColor3d(0.9, 0.75, 0.1);
	glBegin(GL_POLYGON);
	glVertex2d(x - 2.7, y + 2.85);
	glVertex2d(x - 2.7, y + 3.25);
	glVertex2d(x - 2.0, y + 3.25);
	glVertex2d(x - 2.0, y + 2.85);
	glEnd();
	glColor3d(0.7, 0.4, 0.0);
	glBegin(GL_POLYGON);
	glVertex2d(x - 2.0, y + 2.88);
	glVertex2d(x - 2.0, y + 3.22);
	glVertex2d(x - 1.4, y + 3.05);
	glEnd();

	// ammo bar
	glColor3d(0.15, 0.15, 0.15);
	glBegin(GL_POLYGON);
	glVertex2d(x - 1, y + 2.8);
	glVertex2d(x - 1, y + 3.3);
	glVertex2d(x + 1, y + 3.3);
	glVertex2d(x + 1, y + 2.8);
	glEnd();

	double ammoRatio = (double)ammoInventory / SUPPLIER_MAX_SUPPLY;
	glColor3d(0.0, 0.8, 0.9);
	glBegin(GL_POLYGON);
	glVertex2d(x - 1, y + 2.8);
	glVertex2d(x - 1, y + 3.3);
	glVertex2d(x - 1 + 2 * ammoRatio, y + 3.3);
	glVertex2d(x - 1 + 2 * ammoRatio, y + 2.8);
	glEnd();

	glColor3d(0, 0, 0);
	glBegin(GL_LINE_LOOP);
	glVertex2d(x - 1, y + 2.8);
	glVertex2d(x - 1, y + 3.3);
	glVertex2d(x + 1, y + 3.3);
	glVertex2d(x + 1, y + 2.8);
	glEnd();
}

void Supplier::update()
{
	if (!alive) return;
	if (supplyCooldown > 0) supplyCooldown--;
	doMovement();
	if (pCurrentState)
		pCurrentState->Execute(this);
}

void Supplier::supplyTarget(Fighter* target)
{
	if (!alive) return;
	if (target == nullptr || !target->isAlive()) return;
	if (ammoInventory <= 0) return;

	int needed = FIGHTER_MAX_AMMO - target->getAmmo();
	if (needed <= 0) return;

	int given = (needed > ammoInventory) ? ammoInventory : needed;
	target->addAmmo(given);
	ammoInventory -= given;
}

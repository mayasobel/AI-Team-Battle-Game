#include "Fighter.h"
#include "FighterStates.h"
#include "Team.h"
#include "Room.h"
#include "SafetyMap.h"
#include "glut.h"
#include <cstdlib>

extern Team* teams[2];
extern SafetyMap safetyMap;
extern int maze[MSZ][MSZ];
extern Room* rooms[NUM_ROOMS];

Fighter::Fighter(int team, int id, int startRoom, double startX, double startY)
	: BaseAgent(team, id, startRoom, startX, startY)
{
	ammo = FIGHTER_START_AMMO;
	attackCooldown = 0;
	justFired = false;
	lastWasGrenade = false;
	lastTargetX = lastTargetY = 0;
	requestingHeal = false;
	requestingAmmo = false;
	assignedMedicId = -1;
	assignedSupplierId = -1;
	setCurrentState(new SeekEnemy());
}

void Fighter::show()
{
	if (!alive) return;

	// body
	if (teamId == 0)
		glColor3d(0.1, 0.1, 0.9);
	else
		glColor3d(0.9, 0.1, 0.1);

	glBegin(GL_POLYGON);
	glVertex2d(x - 1, y - 1);
	glVertex2d(x - 1, y + 1);
	glVertex2d(x + 1, y + 1);
	glVertex2d(x + 1, y - 1);
	glEnd();

	// head
	double hr = 0.6;
	if (teamId == 0)
		glColor3d(0.3, 0.3, 1.0);
	else
		glColor3d(1.0, 0.3, 0.3);
	glBegin(GL_POLYGON);
	glVertex2d(x - hr, y + 1);
	glVertex2d(x - hr, y + 1 + hr * 2);
	glVertex2d(x + hr, y + 1 + hr * 2);
	glVertex2d(x + hr, y + 1);
	glEnd();

	glColor3d(0, 0, 0);
	glBegin(GL_LINE_LOOP);
	glVertex2d(x - 1, y - 1);
	glVertex2d(x - 1, y + 1);
	glVertex2d(x + 1, y + 1);
	glVertex2d(x + 1, y - 1);
	glEnd();

	// health icon
	glColor3d(0.9, 0.1, 0.1);
	glLineWidth(3.0f);
	glBegin(GL_LINES);
	glVertex2d(x - 2.8, y + 3.75);
	glVertex2d(x - 1.4, y + 3.75);
	glVertex2d(x - 2.1, y + 3.35);
	glVertex2d(x - 2.1, y + 4.15);
	glEnd();
	glLineWidth(1.0f);

	// health bar
	glColor3d(0.3, 0.0, 0.0);
	glBegin(GL_POLYGON);
	glVertex2d(x - 1, y + 3.5);
	glVertex2d(x - 1, y + 4);
	glVertex2d(x + 1, y + 4);
	glVertex2d(x + 1, y + 3.5);
	glEnd();

	double hpRatio = health / MAX_HEALTH;
	glColor3d(1 - hpRatio, hpRatio, 0);
	glBegin(GL_POLYGON);
	glVertex2d(x - 1, y + 3.5);
	glVertex2d(x - 1, y + 4);
	glVertex2d(x - 1 + 2 * hpRatio, y + 4);
	glVertex2d(x - 1 + 2 * hpRatio, y + 3.5);
	glEnd();

	glColor3d(0, 0, 0);
	glBegin(GL_LINE_LOOP);
	glVertex2d(x - 1, y + 3.5);
	glVertex2d(x - 1, y + 4);
	glVertex2d(x + 1, y + 4);
	glVertex2d(x + 1, y + 3.5);
	glEnd();

	// ammo icon
	glColor3d(0.9, 0.75, 0.1);
	glBegin(GL_POLYGON);
	glVertex2d(x - 2.7, y + 4.35);
	glVertex2d(x - 2.7, y + 4.75);
	glVertex2d(x - 2.0, y + 4.75);
	glVertex2d(x - 2.0, y + 4.35);
	glEnd();
	glColor3d(0.7, 0.4, 0.0);
	glBegin(GL_POLYGON);
	glVertex2d(x - 2.0, y + 4.38);
	glVertex2d(x - 2.0, y + 4.72);
	glVertex2d(x - 1.4, y + 4.55);
	glEnd();

	// ammo bar
	glColor3d(0.15, 0.15, 0.15);
	glBegin(GL_POLYGON);
	glVertex2d(x - 1, y + 4.3);
	glVertex2d(x - 1, y + 4.8);
	glVertex2d(x + 1, y + 4.8);
	glVertex2d(x + 1, y + 4.3);
	glEnd();

	double ammoRatio = (double)ammo / FIGHTER_MAX_AMMO;
	glColor3d(0.0, 0.8, 0.9);
	glBegin(GL_POLYGON);
	glVertex2d(x - 1, y + 4.3);
	glVertex2d(x - 1, y + 4.8);
	glVertex2d(x - 1 + 2 * ammoRatio, y + 4.8);
	glVertex2d(x - 1 + 2 * ammoRatio, y + 4.3);
	glEnd();

	glColor3d(0, 0, 0);
	glBegin(GL_LINE_LOOP);
	glVertex2d(x - 1, y + 4.3);
	glVertex2d(x - 1, y + 4.8);
	glVertex2d(x + 1, y + 4.8);
	glVertex2d(x + 1, y + 4.3);
	glEnd();

}

void Fighter::update()
{
	if (!alive) return;
	if (attackCooldown > 0) attackCooldown--;
	doMovement();
	if (pCurrentState)
		pCurrentState->Execute(this);
}

void Fighter::fireAtTarget(BaseAgent* target)
{
	if (!alive || ammo <= 0 || attackCooldown > 0) return;
	if (target == nullptr || !target->isAlive()) return;
	if (target == this) return;
	if (target->getTeamId() == teamId) return;
	if (!hasLineOfSight(x, y, target->getX(), target->getY())) return;

	double damage = BULLET_DAMAGE + (rand() % 5 - 2);
	target->takeDamage(damage, teamId);
	ammo--;
	attackCooldown = ATTACK_COOLDOWN;

	justFired = true;
	lastWasGrenade = false;
	lastTargetX = target->getX();
	lastTargetY = target->getY();
}

void Fighter::throwGrenadeInRoom()
{
	if (!alive || ammo < GRENADE_AMMO_COST || attackCooldown > 0) return;
	if (!teams[1 - teamId]) return;

	int enemyTeam = 1 - teamId;
	double gx = x, gy = y;

	const double GRENADE_RANGE = 10.0;
	std::vector<BaseAgent*>& enemies = teams[enemyTeam]->getAllAgents();
	bool hitAny = false;
	for (size_t i = 0; i < enemies.size(); i++)
	{
		if (enemies[i] == this) continue;
		if (!enemies[i]->isAlive()) continue;
		if (enemies[i]->getTeamId() == teamId) continue;
		if (!hasLineOfSight(x, y, enemies[i]->getX(), enemies[i]->getY()))
			continue;

		double dist = sqrt(pow(x - enemies[i]->getX(), 2) +
		                   pow(y - enemies[i]->getY(), 2));
		if (enemies[i]->getCurrentRoom() != currentRoom && dist > GRENADE_RANGE)
			continue;

		double dmg = GRENADE_DAMAGE + (rand() % 5 - 2);
		enemies[i]->takeDamage(dmg, teamId);
		gx = enemies[i]->getX();
		gy = enemies[i]->getY();
		hitAny = true;
	}
	if (!hitAny) return;

	ammo -= GRENADE_AMMO_COST;
	attackCooldown = ATTACK_COOLDOWN * 2;
	justFired = true;
	lastWasGrenade = true;
	lastTargetX = gx;
	lastTargetY = gy;

	if (currentRoom >= 0 && currentRoom < NUM_ROOMS)
		safetyMap.addCombatRisk(currentRoom, rooms[currentRoom], maze);
}

void Fighter::addAmmo(int amount)
{
	ammo += amount;
	if (ammo > FIGHTER_MAX_AMMO)
		ammo = FIGHTER_MAX_AMMO;
}

#include "FighterStates.h"
#include "Fighter.h"
#include "Medic.h"
#include "Supplier.h"
#include "Team.h"
#include "Room.h"
#include "SafetyMap.h"
#include <math.h>
#include <algorithm>


extern Room* rooms[NUM_ROOMS];
extern Team* teams[2];
extern SafetyMap safetyMap;
extern int maze[MSZ][MSZ];

// Bresenham line check for walls/obstacles between two points
bool hasLineOfSight(double x1, double y1, double x2, double y2)
{
	int ix1 = (int)x1, iy1 = (int)y1;
	int ix2 = (int)x2, iy2 = (int)y2;
	int dx = abs(ix2 - ix1), dy = abs(iy2 - iy1);
	int sx = (ix1 < ix2) ? 1 : -1;
	int sy = (iy1 < iy2) ? 1 : -1;
	int err = dx - dy;

	while (true)
	{
		if (ix1 < 0 || ix1 >= MSZ || iy1 < 0 || iy1 >= MSZ)
			return false;
		if (maze[iy1][ix1] == WALL || maze[iy1][ix1] == OBSTACLE)
			return false;
		if (ix1 == ix2 && iy1 == iy2)
			break;
		int e2 = 2 * err;
		if (e2 > -dy) { err -= dy; ix1 += sx; }
		if (e2 < dx)  { err += dx; iy1 += sy; }
	}
	return true;
}

BaseAgent* findNearestEnemy(BaseAgent* agent)
{
	int enemyTeam = 1 - agent->getTeamId();
	if (!teams[enemyTeam]) return nullptr;

	BaseAgent* nearest = nullptr;
	double minDist = 999999;

	std::vector<BaseAgent*>& enemies = teams[enemyTeam]->getAllAgents();
	for (size_t i = 0; i < enemies.size(); i++)
	{
		if (enemies[i] == agent) continue;
		if (!enemies[i]->isAlive()) continue;
		if (enemies[i]->getTeamId() == agent->getTeamId()) continue;
		double dist = sqrt(pow(agent->getX() - enemies[i]->getX(), 2) +
		                   pow(agent->getY() - enemies[i]->getY(), 2));
		if (dist < minDist)
		{
			minDist = dist;
			nearest = enemies[i];
		}
	}
	return nearest;
}

// also checks nearby enemies in corridors (proximity fallback)
BaseAgent* findEnemyInRoom(BaseAgent* agent)
{
	int enemyTeam = 1 - agent->getTeamId();
	if (!teams[enemyTeam]) return nullptr;

	int room = agent->getCurrentRoom();
	BaseAgent* nearest = nullptr;
	double minDist = 999999;

	std::vector<BaseAgent*>& enemies = teams[enemyTeam]->getAllAgents();
	for (size_t i = 0; i < enemies.size(); i++)
	{
		if (enemies[i] == agent) continue;
		if (!enemies[i]->isAlive()) continue;
		if (enemies[i]->getTeamId() == agent->getTeamId()) continue;
		if (enemies[i]->getCurrentRoom() != room) continue;
		if (!hasLineOfSight(agent->getX(), agent->getY(),
			enemies[i]->getX(), enemies[i]->getY()))
			continue;

		double dist = sqrt(pow(agent->getX() - enemies[i]->getX(), 2) +
		                   pow(agent->getY() - enemies[i]->getY(), 2));
		if (dist < minDist)
		{
			minDist = dist;
			nearest = enemies[i];
		}
	}

	// close-range fallback for corridor encounters
	if (!nearest)
	{
		const double CLOSE_RANGE = 10.0;
		for (size_t i = 0; i < enemies.size(); i++)
		{
			if (enemies[i] == agent) continue;
			if (!enemies[i]->isAlive()) continue;
			if (enemies[i]->getTeamId() == agent->getTeamId()) continue;

			double dist = sqrt(pow(agent->getX() - enemies[i]->getX(), 2) +
			                   pow(agent->getY() - enemies[i]->getY(), 2));
			if (dist > CLOSE_RANGE) continue;
			if (!hasLineOfSight(agent->getX(), agent->getY(),
				enemies[i]->getX(), enemies[i]->getY()))
				continue;

			if (dist < minDist)
			{
				minDist = dist;
				nearest = enemies[i];
			}
		}
	}

	return nearest;
}

int findSafestRoom(BaseAgent* agent)
{
	int bestRoom = agent->getCurrentRoom();
	double bestScore = 999999;

	int enemyTeam = 1 - agent->getTeamId();

	for (int i = 0; i < NUM_ROOMS; i++)
	{
		double score = safetyMap.getRoomRisk(i);

		// farther from enemies = lower score = safer
		if (teams[enemyTeam])
		{
			std::vector<BaseAgent*>& enemies = teams[enemyTeam]->getAllAgents();
			for (size_t e = 0; e < enemies.size(); e++)
			{
				if (!enemies[e]->isAlive()) continue;
				double dist = sqrt(
					pow(rooms[i]->getCenterCol() - enemies[e]->getX(), 2) +
					pow(rooms[i]->getCenterRow() - enemies[e]->getY(), 2));
				score -= dist * 0.01;
			}
		}

		if (score < bestScore)
		{
			bestScore = score;
			bestRoom = i;
		}
	}
	return bestRoom;
}

int findNearestDepot(BaseAgent* agent, DepotType type)
{
	int bestRoom = -1;
	double minDist = 999999;

	for (int i = 0; i < NUM_ROOMS; i++)
	{
		if (rooms[i]->getDepot() != type) continue;
		double dist = sqrt(
			pow(rooms[i]->getCenterCol() - agent->getX(), 2) +
			pow(rooms[i]->getCenterRow() - agent->getY(), 2));
		if (dist < minDist)
		{
			minDist = dist;
			bestRoom = i;
		}
	}
	return bestRoom;
}

BaseAgent* findNearestTeammate(BaseAgent* agent, AgentType type)
{
	if (!teams[agent->getTeamId()]) return nullptr;

	BaseAgent* nearest = nullptr;
	double minDist = 999999;

	std::vector<BaseAgent*>& allies = teams[agent->getTeamId()]->getAllAgents();
	for (size_t i = 0; i < allies.size(); i++)
	{
		if (allies[i] == agent || !allies[i]->isAlive()) continue;
		if (allies[i]->getType() != type) continue;
		double dist = sqrt(pow(agent->getX() - allies[i]->getX(), 2) +
		                   pow(agent->getY() - allies[i]->getY(), 2));
		if (dist < minDist)
		{
			minDist = dist;
			nearest = allies[i];
		}
	}
	return nearest;
}

// A* between rooms, risky rooms cost more
std::vector<int> findRoomPath(int startRoom, int endRoom, double riskAversion)
{
	if (startRoom == endRoom)
		return std::vector<int>(1, startRoom);

	struct RNode {
		double f, g, h;
		int parent;
		bool closed;
	};

	RNode nodes[NUM_ROOMS];
	for (int i = 0; i < NUM_ROOMS; i++)
	{
		nodes[i].f = nodes[i].g = nodes[i].h = 999999;
		nodes[i].parent = -1;
		nodes[i].closed = false;
	}

	double hVal = sqrt(
		pow(rooms[startRoom]->getCenterCol() - rooms[endRoom]->getCenterCol(), 2.0) +
		pow(rooms[startRoom]->getCenterRow() - rooms[endRoom]->getCenterRow(), 2.0));
	nodes[startRoom].g = 0;
	nodes[startRoom].h = hVal;
	nodes[startRoom].f = hVal;

	while (true)
	{
		int current = -1;
		double bestF = 999999;
		for (int i = 0; i < NUM_ROOMS; i++)
		{
			if (!nodes[i].closed && nodes[i].f < bestF)
			{
				bestF = nodes[i].f;
				current = i;
			}
		}

		if (current == -1 || current == endRoom) break;
		nodes[current].closed = true;

		for (int i = 0; i < NUM_ROOMS; i++)
		{
			if (nodes[i].closed || i == current) continue;

			double dist = sqrt(
				pow(rooms[current]->getCenterCol() - rooms[i]->getCenterCol(), 2.0) +
				pow(rooms[current]->getCenterRow() - rooms[i]->getCenterRow(), 2.0));
			double risk = safetyMap.getRoomRisk(i) * riskAversion * 50.0;
			double newG = nodes[current].g + dist + risk;

			if (newG < nodes[i].g)
			{
				nodes[i].g = newG;
				nodes[i].h = sqrt(
					pow(rooms[i]->getCenterCol() - rooms[endRoom]->getCenterCol(), 2.0) +
					pow(rooms[i]->getCenterRow() - rooms[endRoom]->getCenterRow(), 2.0));
				nodes[i].f = nodes[i].g + nodes[i].h;
				nodes[i].parent = current;
			}
		}
	}

	std::vector<int> path;
	int cur = endRoom;
	while (cur != -1)
	{
		path.push_back(cur);
		cur = nodes[cur].parent;
	}
	std::reverse(path.begin(), path.end());
	return path;
}

void navigateToRoom(BaseAgent* agent, int targetRoom)
{
	if (targetRoom < 0 || targetRoom >= NUM_ROOMS) return;
	if (agent->getCurrentRoom() == targetRoom && !agent->getIsMoving())
		return;

	agent->setTargetRoom(targetRoom);
	std::vector<int> path = findRoomPath(
		agent->getCurrentRoom(), targetRoom,
		agent->getPersonality().riskAversion);
	agent->setRoomPath(path);

	if (path.size() > 1)
	{
		int nextRoom = path[1];
		agent->moveToward(rooms[nextRoom]->getCenterCol(),
		                  rooms[nextRoom]->getCenterRow());
	}
	else if (path.size() == 1)
	{
		agent->moveToward(rooms[path[0]]->getCenterCol(),
		                  rooms[path[0]]->getCenterRow());
	}
}

void SeekEnemy::OnEnter(BaseAgent* agent)
{
	BaseAgent* enemy = findNearestEnemy(agent);
	if (enemy)
		navigateToRoom(agent, enemy->getCurrentRoom());
}

// FSM priority: critical HP > engage > low HP > low ammo > seek
void SeekEnemy::Execute(BaseAgent* agent)
{
	Fighter* fighter = dynamic_cast<Fighter*>(agent);
	if (!fighter) return;

	if (fighter->getHealth() <= CRITICAL_HP_THRESHOLD)
	{
		fighter->clearAmmoRequest();
		fighter->setCurrentState(new MoveToMedic());
		return;
	}

	BaseAgent* enemyInRoom = findEnemyInRoom(agent);
	if (enemyInRoom && fighter->getAmmo() > 0)
	{
		fighter->setCurrentState(new EngageCombat());
		return;
	}

	if (fighter->getHealth() < fighter->getPersonality().healthThreshold)
	{
		fighter->clearAmmoRequest();
		fighter->setCurrentState(new MoveToMedic());
		return;
	}

	if (fighter->getAmmo() < fighter->getPersonality().ammoThreshold)
	{
		fighter->clearHealRequest();
		fighter->setCurrentState(new MoveToSupplier());
		return;
	}

	if (!agent->getIsMoving())
	{
		BaseAgent* enemy = findNearestEnemy(agent);
		if (enemy)
			navigateToRoom(agent, enemy->getCurrentRoom());
	}
}

void SeekEnemy::OnExit(BaseAgent* agent)
{
	agent->setIsMoving(false);
}

void EngageCombat::OnEnter(BaseAgent* agent)
{
	agent->setIsMoving(false);
}

void EngageCombat::Execute(BaseAgent* agent)
{
	Fighter* fighter = dynamic_cast<Fighter*>(agent);
	if (!fighter) return;

	if (fighter->getHealth() <= CRITICAL_HP_THRESHOLD)
	{
		fighter->clearAmmoRequest();
		fighter->setCurrentState(new MoveToMedic());
		return;
	}

	if (fighter->getAmmo() <= 0)
	{
		fighter->clearHealRequest();
		fighter->setCurrentState(new MoveToSupplier());
		return;
	}

	BaseAgent* enemy = findEnemyInRoom(agent);
	if (!enemy)
	{
		fighter->setCurrentState(new SeekEnemy());
		return;
	}

	// higher aggression = more likely to throw grenade
	bool useGrenade = false;
	if (fighter->getAmmo() >= GRENADE_AMMO_COST)
	{
		int roll = rand() % 100;
		if (roll < (int)(fighter->getPersonality().aggression * 40))
			useGrenade = true;
	}

	if (useGrenade)
		fighter->throwGrenadeInRoom();
	else
		fighter->fireAtTarget(enemy);

	safetyMap.updateRoomRisk(agent->getCurrentRoom(),
		safetyMap.getRoomRisk(agent->getCurrentRoom()) + 0.1);
}

void EngageCombat::OnExit(BaseAgent* agent)
{
}

void MoveToMedic::OnEnter(BaseAgent* agent)
{
	Fighter* fighter = dynamic_cast<Fighter*>(agent);
	if (!fighter) return;

	fighter->setRequestingHeal(true);
	fighter->clearAmmoRequest();
	int myTeam = agent->getTeamId();
	Medic* medic = teams[myTeam] ? teams[myTeam]->getMedic() : nullptr;

	if (medic && medic->isAlive())
		navigateToRoom(agent, medic->getCurrentRoom());
	else
	{
		int safeRoom = findSafestRoom(agent);
		navigateToRoom(agent, safeRoom);
	}
}

void MoveToMedic::Execute(BaseAgent* agent)
{
	Fighter* fighter = dynamic_cast<Fighter*>(agent);
	if (!fighter) return;

	bool criticalHP = fighter->getHealth() <= CRITICAL_HP_THRESHOLD;

	if (!criticalHP)
	{
		BaseAgent* enemyInRoom = findEnemyInRoom(agent);
		if (enemyInRoom && fighter->getAmmo() > 0)
		{
			fighter->setCurrentState(new EngageCombat());
			return;
		}
	}

	if (fighter->getHealth() >= MAX_HEALTH)
	{
		fighter->clearHealRequest();
		fighter->setCurrentState(new SeekEnemy());
		return;
	}

	int myTeam = agent->getTeamId();
	Medic* medic = teams[myTeam] ? teams[myTeam]->getMedic() : nullptr;

	if (!medic || !medic->isAlive())
	{
		if (!agent->getIsMoving())
		{
			int safeRoom = findSafestRoom(agent);
			navigateToRoom(agent, safeRoom);
		}
		return;
	}

	if (medic->getCurrentRoom() == agent->getCurrentRoom())
	{
		double dist = sqrt(pow(agent->getX() - medic->getX(), 2) +
		                   pow(agent->getY() - medic->getY(), 2));
		if (dist < SUPPORT_INTERACT_RANGE)
		{
			fighter->setAssignedMedicId(medic->getAgentId());
			fighter->setCurrentState(new ReceiveHeal());
			return;
		}
		else
		{
			if (!agent->getIsMoving())
				agent->moveToward(medic->getX(), medic->getY());
		}
	}
	else
	{
		if (!agent->getIsMoving())
			navigateToRoom(agent, medic->getCurrentRoom());
	}
}

void MoveToMedic::OnExit(BaseAgent* agent)
{
	agent->setIsMoving(false);
}

void MoveToSupplier::OnEnter(BaseAgent* agent)
{
	Fighter* fighter = dynamic_cast<Fighter*>(agent);
	if (!fighter) return;

	if (fighter->getHealth() <= CRITICAL_HP_THRESHOLD)
	{
		fighter->setCurrentState(new MoveToMedic());
		return;
	}

	fighter->setRequestingAmmo(true);
	fighter->clearHealRequest();
	int myTeam = agent->getTeamId();
	Supplier* supplier = teams[myTeam] ? teams[myTeam]->getSupplier() : nullptr;

	if (supplier && supplier->isAlive())
		navigateToRoom(agent, supplier->getCurrentRoom());
	else
	{
		int safeRoom = findSafestRoom(agent);
		navigateToRoom(agent, safeRoom);
	}
}

void MoveToSupplier::Execute(BaseAgent* agent)
{
	Fighter* fighter = dynamic_cast<Fighter*>(agent);
	if (!fighter) return;

	if (fighter->getHealth() <= CRITICAL_HP_THRESHOLD)
	{
		fighter->clearAmmoRequest();
		fighter->setCurrentState(new MoveToMedic());
		return;
	}

	BaseAgent* enemyInRoom = findEnemyInRoom(agent);
	if (enemyInRoom && fighter->getAmmo() > 0)
	{
		fighter->setCurrentState(new EngageCombat());
		return;
	}

	if (fighter->getAmmo() >= FIGHTER_MAX_AMMO)
	{
		fighter->clearAmmoRequest();
		fighter->setCurrentState(new SeekEnemy());
		return;
	}

	int myTeam = agent->getTeamId();
	Supplier* supplier = teams[myTeam] ? teams[myTeam]->getSupplier() : nullptr;

	if (!supplier || !supplier->isAlive())
	{
		if (!agent->getIsMoving())
		{
			int safeRoom = findSafestRoom(agent);
			navigateToRoom(agent, safeRoom);
		}
		return;
	}

	if (supplier->getCurrentRoom() == agent->getCurrentRoom())
	{
		double dist = sqrt(pow(agent->getX() - supplier->getX(), 2) +
		                   pow(agent->getY() - supplier->getY(), 2));
		if (dist < SUPPORT_INTERACT_RANGE)
		{
			fighter->setAssignedSupplierId(supplier->getAgentId());
			fighter->setCurrentState(new ReceiveAmmo());
			return;
		}
		else
		{
			if (!agent->getIsMoving())
				agent->moveToward(supplier->getX(), supplier->getY());
		}
	}
	else
	{
		if (!agent->getIsMoving())
			navigateToRoom(agent, supplier->getCurrentRoom());
	}
}

void MoveToSupplier::OnExit(BaseAgent* agent)
{
	agent->setIsMoving(false);
}

void ReceiveHeal::OnEnter(BaseAgent* agent)
{
	agent->setIsMoving(false);
}

void ReceiveHeal::Execute(BaseAgent* agent)
{
	Fighter* fighter = dynamic_cast<Fighter*>(agent);
	if (!fighter) return;

	agent->setIsMoving(false);

	if (fighter->getHealth() >= MAX_HEALTH)
	{
		fighter->clearHealRequest();
		fighter->setCurrentState(new SeekEnemy());
		return;
	}

	bool criticalHP = fighter->getHealth() <= CRITICAL_HP_THRESHOLD;
	if (!criticalHP)
	{
		BaseAgent* enemyInRoom = findEnemyInRoom(agent);
		if (enemyInRoom && fighter->getAmmo() > 0)
		{
			fighter->clearHealRequest();
			fighter->setCurrentState(new EngageCombat());
			return;
		}
	}

	int myTeam = agent->getTeamId();
	Medic* medic = teams[myTeam] ? teams[myTeam]->getMedic() : nullptr;

	if (!medic || !medic->isAlive())
	{
		fighter->clearHealRequest();
		fighter->setCurrentState(new SeekEnemy());
		return;
	}

	double dist = sqrt(pow(agent->getX() - medic->getX(), 2) +
	                   pow(agent->getY() - medic->getY(), 2));
	if (dist >= SUPPORT_INTERACT_RANGE * 2 ||
		medic->getCurrentRoom() != agent->getCurrentRoom())
	{
		fighter->setCurrentState(new MoveToMedic());
		return;
	}
}

void ReceiveHeal::OnExit(BaseAgent* agent)
{
}

void ReceiveAmmo::OnEnter(BaseAgent* agent)
{
	agent->setIsMoving(false);
}

void ReceiveAmmo::Execute(BaseAgent* agent)
{
	Fighter* fighter = dynamic_cast<Fighter*>(agent);
	if (!fighter) return;

	agent->setIsMoving(false);

	if (fighter->getHealth() <= CRITICAL_HP_THRESHOLD)
	{
		fighter->clearAmmoRequest();
		fighter->setCurrentState(new MoveToMedic());
		return;
	}

	if (fighter->getAmmo() >= FIGHTER_MAX_AMMO)
	{
		fighter->clearAmmoRequest();
		fighter->setCurrentState(new SeekEnemy());
		return;
	}

	BaseAgent* enemyInRoom = findEnemyInRoom(agent);
	if (enemyInRoom && fighter->getAmmo() > 0)
	{
		fighter->clearAmmoRequest();
		fighter->setCurrentState(new EngageCombat());
		return;
	}

	int myTeam = agent->getTeamId();
	Supplier* supplier = teams[myTeam] ? teams[myTeam]->getSupplier() : nullptr;

	if (!supplier || !supplier->isAlive())
	{
		fighter->clearAmmoRequest();
		fighter->setCurrentState(new SeekEnemy());
		return;
	}

	double dist = sqrt(pow(agent->getX() - supplier->getX(), 2) +
	                   pow(agent->getY() - supplier->getY(), 2));
	if (dist >= SUPPORT_INTERACT_RANGE * 2 ||
		supplier->getCurrentRoom() != agent->getCurrentRoom())
	{
		fighter->setCurrentState(new MoveToSupplier());
		return;
	}
}

void ReceiveAmmo::OnExit(BaseAgent* agent)
{
}

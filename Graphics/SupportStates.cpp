#include "SupportStates.h"
#include "BaseAgent.h"
#include "Fighter.h"
#include "Medic.h"
#include "Supplier.h"
#include "Team.h"
#include "Room.h"
#include "SafetyMap.h"
#include "FighterStates.h"
#include <math.h>

extern Room* rooms[NUM_ROOMS];
extern Team* teams[2];
extern SafetyMap safetyMap;

static bool needsSelfMaintain(BaseAgent* agent)
{
	Medic* medic = dynamic_cast<Medic*>(agent);
	if (medic)
		return !medic->hasSupply();

	Supplier* supplier = dynamic_cast<Supplier*>(agent);
	if (supplier)
		return supplier->getAmmoInventory() < SUPPLY_RESTOCK_THRESHOLD;

	return false;
}

static bool enemyNearWithoutProtection(BaseAgent* agent)
{
	BaseAgent* enemy = findEnemyInRoom(agent);
	if (!enemy) return false;

	int myTeam = agent->getTeamId();
	if (!teams[myTeam]) return true;

	std::vector<Fighter*>& fighters = teams[myTeam]->getFighters();
	for (size_t i = 0; i < fighters.size(); i++)
	{
		if (fighters[i]->isAlive() &&
			fighters[i]->getCurrentRoom() == agent->getCurrentRoom())
			return false;
	}
	return true;
}

// fighters who sent an explicit request get +100 score
static Fighter* findMostUrgentClient(BaseAgent* agent)
{
	int myTeam = agent->getTeamId();
	if (!teams[myTeam]) return nullptr;

	Fighter* best = nullptr;
	double bestScore = -1;

	std::vector<Fighter*>& fighters = teams[myTeam]->getFighters();
	for (size_t i = 0; i < fighters.size(); i++)
	{
		if (!fighters[i]->isAlive()) continue;

		if (agent->getType() == AGENT_MEDIC)
		{
			if (fighters[i]->getAssignedMedicId() != -1 &&
				fighters[i]->getAssignedMedicId() != agent->getAgentId())
				continue;

			if (fighters[i]->getHealth() >= MAX_HEALTH) continue;

			double score = MAX_HEALTH - fighters[i]->getHealth();
			if (fighters[i]->isRequestingHeal()) score += 100;
			if (score > bestScore)
			{
				bestScore = score;
				best = fighters[i];
			}
		}
		else if (agent->getType() == AGENT_SUPPLIER)
		{
			if (fighters[i]->getAssignedSupplierId() != -1 &&
				fighters[i]->getAssignedSupplierId() != agent->getAgentId())
				continue;

			if (fighters[i]->getAmmo() >= FIGHTER_MAX_AMMO) continue;

			double score = FIGHTER_MAX_AMMO - fighters[i]->getAmmo();
			if (fighters[i]->isRequestingAmmo()) score += 100;
			if (score > bestScore)
			{
				bestScore = score;
				best = fighters[i];
			}
		}
	}
	return best;
}

static Fighter* findClientById(int teamId, int clientId)
{
	if (clientId < 0 || !teams[teamId]) return nullptr;

	std::vector<Fighter*>& fighters = teams[teamId]->getFighters();
	for (size_t i = 0; i < fighters.size(); i++)
	{
		if (fighters[i]->getAgentId() == clientId)
			return fighters[i];
	}
	return nullptr;
}

static void acceptClient(BaseAgent* agent, Fighter* client)
{
	Medic* medic = dynamic_cast<Medic*>(agent);
	if (medic)
	{
		medic->setCurrentClientId(client->getAgentId());
		client->setAssignedMedicId(agent->getAgentId());
	}

	Supplier* supplier = dynamic_cast<Supplier*>(agent);
	if (supplier)
	{
		supplier->setCurrentClientId(client->getAgentId());
		client->setAssignedSupplierId(agent->getAgentId());
	}
}

static void clearClientAssignment(BaseAgent* agent)
{
	int teamId = agent->getTeamId();
	Medic* medic = dynamic_cast<Medic*>(agent);
	if (medic)
	{
		int cid = medic->getCurrentClientId();
		Fighter* client = findClientById(teamId, cid);
		if (client) client->setAssignedMedicId(-1);
		medic->clearClient();
	}
	Supplier* supplier = dynamic_cast<Supplier*>(agent);
	if (supplier)
	{
		int cid = supplier->getCurrentClientId();
		Fighter* client = findClientById(teamId, cid);
		if (client) client->setAssignedSupplierId(-1);
		supplier->clearClient();
	}
}

static int getCurrentClientIdFromAgent(BaseAgent* agent)
{
	Medic* medic = dynamic_cast<Medic*>(agent);
	if (medic) return medic->getCurrentClientId();
	Supplier* supplier = dynamic_cast<Supplier*>(agent);
	if (supplier) return supplier->getCurrentClientId();
	return -1;
}

void SelfMaintain::OnEnter(BaseAgent* agent)
{
	clearClientAssignment(agent);

	DepotType needed = (agent->getType() == AGENT_MEDIC) ? DEPOT_MEDICAL : DEPOT_AMMO;
	int depot = findNearestDepot(agent, needed);
	if (depot >= 0)
		navigateToRoom(agent, depot);
}

void SelfMaintain::Execute(BaseAgent* agent)
{
	if (!agent->isAlive()) return;

	if (enemyNearWithoutProtection(agent))
	{
		agent->setCurrentState(new FleeState());
		return;
	}

	if (!agent->getIsMoving())
	{
		DepotType needed = (agent->getType() == AGENT_MEDIC) ? DEPOT_MEDICAL : DEPOT_AMMO;
		int rm = agent->getCurrentRoom();

		if (rm >= 0 && rm < NUM_ROOMS && rooms[rm]->getDepot() == needed)
		{
			Medic* medic = dynamic_cast<Medic*>(agent);
			if (medic)
				medic->refillSupply();

			Supplier* supplier = dynamic_cast<Supplier*>(agent);
			if (supplier)
				supplier->refillSupply();

			agent->setCurrentState(new IdleSupport());
			return;
		}

		int depot = findNearestDepot(agent, needed);
		if (depot >= 0)
			navigateToRoom(agent, depot);
	}
}

void SelfMaintain::OnExit(BaseAgent* agent)
{
	agent->setIsMoving(false);
}

void MoveToClient::OnEnter(BaseAgent* agent)
{
	if (needsSelfMaintain(agent))
	{
		agent->setCurrentState(new SelfMaintain());
		return;
	}

	int cid = getCurrentClientIdFromAgent(agent);
	Fighter* client = findClientById(agent->getTeamId(), cid);

	if (!client || !client->isAlive())
	{
		clearClientAssignment(agent);
		Fighter* newClient = findMostUrgentClient(agent);
		if (newClient)
		{
			acceptClient(agent, newClient);
			navigateToRoom(agent, newClient->getCurrentRoom());
		}
		else
		{
			agent->setCurrentState(new IdleSupport());
		}
		return;
	}

	navigateToRoom(agent, client->getCurrentRoom());
}

void MoveToClient::Execute(BaseAgent* agent)
{
	if (!agent->isAlive()) return;

	int cid = getCurrentClientIdFromAgent(agent);
	Fighter* client = findClientById(agent->getTeamId(), cid);

	if (!client || !client->isAlive())
	{
		clearClientAssignment(agent);
		agent->setCurrentState(new IdleSupport());
		return;
	}

	Medic* medic = dynamic_cast<Medic*>(agent);
	if (medic && client->getHealth() >= MAX_HEALTH)
	{
		clearClientAssignment(agent);
		agent->setCurrentState(new IdleSupport());
		return;
	}
	Supplier* supplier = dynamic_cast<Supplier*>(agent);
	if (supplier && client->getAmmo() >= FIGHTER_MAX_AMMO)
	{
		clearClientAssignment(agent);
		agent->setCurrentState(new IdleSupport());
		return;
	}

	if (client->getCurrentRoom() == agent->getCurrentRoom())
	{
		double dist = sqrt(pow(agent->getX() - client->getX(), 2) +
		                   pow(agent->getY() - client->getY(), 2));
		if (dist < SUPPORT_INTERACT_RANGE)
		{
			agent->setCurrentState(new ServeClient());
			return;
		}
		else if (!agent->getIsMoving())
		{
			agent->moveToward(client->getX(), client->getY());
		}
	}
	else
	{
		if (agent->getTargetRoom() != client->getCurrentRoom())
		{
			agent->setIsMoving(false);
			navigateToRoom(agent, client->getCurrentRoom());
		}
		else if (!agent->getIsMoving())
		{
			navigateToRoom(agent, client->getCurrentRoom());
		}
	}
}

void MoveToClient::OnExit(BaseAgent* agent)
{
	agent->setIsMoving(false);
}

void ServeClient::OnEnter(BaseAgent* agent)
{
	agent->setIsMoving(false);
}

void ServeClient::Execute(BaseAgent* agent)
{
	if (!agent->isAlive()) return;

	int cid = getCurrentClientIdFromAgent(agent);
	Fighter* client = findClientById(agent->getTeamId(), cid);

	if (!client || !client->isAlive())
	{
		clearClientAssignment(agent);
		agent->setCurrentState(new IdleSupport());
		return;
	}

	if (client->getCurrentRoom() != agent->getCurrentRoom())
	{
		agent->setCurrentState(new MoveToClient());
		return;
	}

	double dist = sqrt(pow(agent->getX() - client->getX(), 2) +
	                   pow(agent->getY() - client->getY(), 2));

	if (dist > SUPPORT_INTERACT_RANGE)
	{
		if (!agent->getIsMoving())
			agent->moveToward(client->getX(), client->getY());
		return;
	}

	Medic* medic = dynamic_cast<Medic*>(agent);
	if (medic)
	{
		if (client->getHealth() >= MAX_HEALTH)
		{
			client->clearHealRequest();
			clearClientAssignment(agent);
			agent->setCurrentState(new IdleSupport());
			return;
		}

		if (!medic->hasSupply())
		{
			clearClientAssignment(agent);
			agent->setCurrentState(new SelfMaintain());
			return;
		}

		if (dist <= SUPPORT_INTERACT_RANGE)
			medic->healTarget(client);
	}

	Supplier* supplier = dynamic_cast<Supplier*>(agent);
	if (supplier)
	{
		if (client->getAmmo() >= FIGHTER_MAX_AMMO)
		{
			client->clearAmmoRequest();
			clearClientAssignment(agent);
			agent->setCurrentState(new IdleSupport());
			return;
		}

		if (!supplier->hasSupply())
		{
			clearClientAssignment(agent);
			agent->setCurrentState(new SelfMaintain());
			return;
		}

		if (dist <= SUPPORT_INTERACT_RANGE)
			supplier->supplyTarget(client);
	}
}

void ServeClient::OnExit(BaseAgent* agent)
{
	agent->setIsMoving(false);
}

// only checks non-fighters (medic can heal supplier and vice versa)
static BaseAgent* findInjuredTeammate(BaseAgent* agent)
{
	int myTeam = agent->getTeamId();
	if (!teams[myTeam]) return nullptr;

	BaseAgent* best = nullptr;
	double worstHealth = MAX_HEALTH;

	std::vector<BaseAgent*>& all = teams[myTeam]->getAllAgents();
	for (size_t i = 0; i < all.size(); i++)
	{
		if (all[i] == agent || !all[i]->isAlive()) continue;
		if (all[i]->getType() == AGENT_FIGHTER) continue;
		if (all[i]->getHealth() >= MAX_HEALTH) continue;
		if (all[i]->getHealth() < worstHealth)
		{
			worstHealth = all[i]->getHealth();
			best = all[i];
		}
	}
	return best;
}

void IdleSupport::OnEnter(BaseAgent* agent)
{
	clearClientAssignment(agent);

	Fighter* client = findMostUrgentClient(agent);
	if (client)
	{
		acceptClient(agent, client);
		agent->setCurrentState(new MoveToClient());
		return;
	}

	Medic* medic = dynamic_cast<Medic*>(agent);
	if (medic && medic->hasSupply())
	{
		BaseAgent* injured = findInjuredTeammate(agent);
		if (injured)
		{
			navigateToRoom(agent, injured->getCurrentRoom());
			return;
		}
	}

	int myTeam = agent->getTeamId();
	if (teams[myTeam])
	{
		std::vector<Fighter*>& fighters = teams[myTeam]->getFighters();
		Fighter* nearest = nullptr;
		double minDist = 999999;
		for (size_t i = 0; i < fighters.size(); i++)
		{
			if (!fighters[i]->isAlive()) continue;
			double dist = sqrt(pow(agent->getX() - fighters[i]->getX(), 2) +
			                   pow(agent->getY() - fighters[i]->getY(), 2));
			if (dist < minDist)
			{
				minDist = dist;
				nearest = fighters[i];
			}
		}
		if (nearest)
		{
			navigateToRoom(agent, nearest->getCurrentRoom());
			return;
		}
	}

	int safeRoom = findSafestRoom(agent);
	if (safeRoom != agent->getCurrentRoom())
		navigateToRoom(agent, safeRoom);
}

void IdleSupport::Execute(BaseAgent* agent)
{
	if (!agent->isAlive()) return;

	if (needsSelfMaintain(agent))
	{
		agent->setCurrentState(new SelfMaintain());
		return;
	}

	Fighter* client = findMostUrgentClient(agent);
	if (client)
	{
		acceptClient(agent, client);
		agent->setCurrentState(new MoveToClient());
		return;
	}

	Medic* medic = dynamic_cast<Medic*>(agent);
	if (medic && medic->hasSupply())
	{
		BaseAgent* injured = findInjuredTeammate(agent);
		if (injured)
		{
			if (injured->getCurrentRoom() == agent->getCurrentRoom())
			{
				double dist = sqrt(pow(agent->getX() - injured->getX(), 2) +
				                   pow(agent->getY() - injured->getY(), 2));
				if (dist <= SUPPORT_INTERACT_RANGE)
				{
					medic->healTarget(injured);
					return;
				}
				else if (!agent->getIsMoving())
				{
					agent->moveToward(injured->getX(), injured->getY());
				}
			}
			else if (!agent->getIsMoving())
			{
				navigateToRoom(agent, injured->getCurrentRoom());
			}
			return;
		}
	}

	if (!agent->getIsMoving() && teams[agent->getTeamId()])
	{
		std::vector<Fighter*>& fighters = teams[agent->getTeamId()]->getFighters();
		Fighter* nearest = nullptr;
		double minDist = 999999;
		for (size_t i = 0; i < fighters.size(); i++)
		{
			if (!fighters[i]->isAlive()) continue;
			double dist = sqrt(pow(agent->getX() - fighters[i]->getX(), 2) +
			                   pow(agent->getY() - fighters[i]->getY(), 2));
			if (dist < minDist)
			{
				minDist = dist;
				nearest = fighters[i];
			}
		}
		if (nearest && nearest->getCurrentRoom() != agent->getCurrentRoom())
			navigateToRoom(agent, nearest->getCurrentRoom());
	}
}

void IdleSupport::OnExit(BaseAgent* agent)
{
	agent->setIsMoving(false);
}

void FleeState::OnEnter(BaseAgent* agent)
{
	clearClientAssignment(agent);
	int safeRoom = findSafestRoom(agent);
	navigateToRoom(agent, safeRoom);
}

void FleeState::Execute(BaseAgent* agent)
{
	if (!agent->isAlive()) return;

	if (!agent->getIsMoving())
	{
		BaseAgent* enemy = findEnemyInRoom(agent);
		if (enemy)
		{
			int safeRoom = findSafestRoom(agent);
			navigateToRoom(agent, safeRoom);
		}
		else
		{
			agent->setCurrentState(new IdleSupport());
		}
	}
}

void FleeState::OnExit(BaseAgent* agent)
{
	agent->setIsMoving(false);
}

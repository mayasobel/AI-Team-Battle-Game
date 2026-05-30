#include "Team.h"
#include "BaseAgent.h"
#include "Fighter.h"
#include "Medic.h"
#include "Supplier.h"

Team::Team(int id)
{
	teamId = id;
	medic = nullptr;
	supplier = nullptr;
}

Team::~Team()
{
	for (size_t i = 0; i < allAgents.size(); i++)
		delete allAgents[i];
}

void Team::addFighter(Fighter* f)
{
	fighters.push_back(f);
	allAgents.push_back(f);
}

void Team::setMedic(Medic* m)
{
	medic = m;
	allAgents.push_back(m);
}

void Team::setSupplier(Supplier* s)
{
	supplier = s;
	allAgents.push_back(s);
}

bool Team::isDefeated()
{
	for (size_t i = 0; i < fighters.size(); i++)
		if (fighters[i]->isAlive()) return false;
	return true;
}

int Team::aliveCount()
{
	int count = 0;
	for (size_t i = 0; i < allAgents.size(); i++)
		if (allAgents[i]->isAlive()) count++;
	return count;
}

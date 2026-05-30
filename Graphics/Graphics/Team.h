#pragma once
#include <vector>

class BaseAgent;
class Fighter;
class Medic;
class Supplier;

class Team
{
private:
	int teamId;
	std::vector<BaseAgent*> allAgents;
	std::vector<Fighter*> fighters;
	Medic* medic;
	Supplier* supplier;

public:
	Team(int id);
	~Team();
	void addFighter(Fighter* f);
	void setMedic(Medic* m);
	void setSupplier(Supplier* s);
	bool isDefeated();
	int aliveCount();

	int getTeamId() { return teamId; }
	std::vector<BaseAgent*>& getAllAgents() { return allAgents; }
	std::vector<Fighter*>& getFighters() { return fighters; }
	Medic* getMedic() { return medic; }
	Supplier* getSupplier() { return supplier; }
};

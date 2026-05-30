#pragma once
#include "State.h"
#include "Globals.h"
#include <vector>

class BaseAgent;

bool hasLineOfSight(double x1, double y1, double x2, double y2);
BaseAgent* findNearestEnemy(BaseAgent* agent);
BaseAgent* findEnemyInRoom(BaseAgent* agent);
int findSafestRoom(BaseAgent* agent);
int findNearestDepot(BaseAgent* agent, DepotType type);
BaseAgent* findNearestTeammate(BaseAgent* agent, AgentType type);
std::vector<int> findRoomPath(int startRoom, int endRoom, double riskAversion);
void navigateToRoom(BaseAgent* agent, int targetRoom);

class SeekEnemy : public State
{
public:
	void OnEnter(BaseAgent* agent) override;
	void Execute(BaseAgent* agent) override;
	void OnExit(BaseAgent* agent) override;
	bool requiresFighter() override { return true; }
	const char* getName() override { return "SeekEnemy"; }
};

class EngageCombat : public State
{
public:
	void OnEnter(BaseAgent* agent) override;
	void Execute(BaseAgent* agent) override;
	void OnExit(BaseAgent* agent) override;
	bool requiresFighter() override { return true; }
	const char* getName() override { return "EngageCombat"; }
};

class MoveToMedic : public State
{
public:
	void OnEnter(BaseAgent* agent) override;
	void Execute(BaseAgent* agent) override;
	void OnExit(BaseAgent* agent) override;
	bool requiresFighter() override { return true; }
	const char* getName() override { return "MoveToMedic"; }
};

class MoveToSupplier : public State
{
public:
	void OnEnter(BaseAgent* agent) override;
	void Execute(BaseAgent* agent) override;
	void OnExit(BaseAgent* agent) override;
	bool requiresFighter() override { return true; }
	const char* getName() override { return "MoveToSupplier"; }
};

class ReceiveHeal : public State
{
public:
	void OnEnter(BaseAgent* agent) override;
	void Execute(BaseAgent* agent) override;
	void OnExit(BaseAgent* agent) override;
	bool requiresFighter() override { return true; }
	const char* getName() override { return "ReceiveHeal"; }
};

class ReceiveAmmo : public State
{
public:
	void OnEnter(BaseAgent* agent) override;
	void Execute(BaseAgent* agent) override;
	void OnExit(BaseAgent* agent) override;
	bool requiresFighter() override { return true; }
	const char* getName() override { return "ReceiveAmmo"; }
};

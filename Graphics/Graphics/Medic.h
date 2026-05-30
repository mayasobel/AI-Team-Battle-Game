#pragma once
#include "BaseAgent.h"

class Medic : public BaseAgent
{
private:
	int healCooldown;
	int healSupply;
	int currentClientId;

public:
	Medic(int team, int id, int startRoom, double startX, double startY);
	void show() override;
	void update() override;
	AgentType getType() override { return AGENT_MEDIC; }
	double getSpeed() override { return SUPPORT_SPEED; }

	void healTarget(BaseAgent* target);
	int getHealSupply() { return healSupply; }
	bool hasSupply() { return healSupply > 0; }
	void refillSupply() { healSupply = MEDIC_MAX_SUPPLY; }

	int getCurrentClientId() { return currentClientId; }
	void setCurrentClientId(int id) { currentClientId = id; }
	void clearClient() { currentClientId = -1; }
};

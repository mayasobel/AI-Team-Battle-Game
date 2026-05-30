#pragma once
#include "BaseAgent.h"

class Fighter;

class Supplier : public BaseAgent
{
private:
	int supplyCooldown;
	int ammoInventory;
	int currentClientId;

public:
	Supplier(int team, int id, int startRoom, double startX, double startY);
	void show() override;
	void update() override;
	AgentType getType() override { return AGENT_SUPPLIER; }
	double getSpeed() override { return SUPPORT_SPEED; }

	void supplyTarget(Fighter* target);
	int getAmmoInventory() { return ammoInventory; }
	bool hasSupply() { return ammoInventory > 0; }
	void refillSupply() { ammoInventory = SUPPLIER_MAX_SUPPLY; }

	int getCurrentClientId() { return currentClientId; }
	void setCurrentClientId(int id) { currentClientId = id; }
	void clearClient() { currentClientId = -1; }
};

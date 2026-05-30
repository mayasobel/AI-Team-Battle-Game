#pragma once
#include "BaseAgent.h"

class Fighter : public BaseAgent
{
private:
	int ammo;
	int attackCooldown;

	// for visual projectile rendering in main
	bool justFired;
	bool lastWasGrenade;
	double lastTargetX, lastTargetY;

	// support request flags
	bool requestingHeal;
	bool requestingAmmo;
	int assignedMedicId;
	int assignedSupplierId;

public:
	Fighter(int team, int id, int startRoom, double startX, double startY);
	void show() override;
	void update() override;
	AgentType getType() override { return AGENT_FIGHTER; }
	bool canAttack() override { return true; }

	void fireAtTarget(BaseAgent* target);
	void throwGrenadeInRoom();
	void addAmmo(int amount);
	int getAmmo() { return ammo; }
	int getAttackCooldown() { return attackCooldown; }

	bool getJustFired() { return justFired; }
	bool getLastWasGrenade() { return lastWasGrenade; }
	double getLastTargetX() { return lastTargetX; }
	double getLastTargetY() { return lastTargetY; }
	void clearCombatFlags() { justFired = false; lastWasGrenade = false; }

	bool isRequestingHeal() { return requestingHeal; }
	bool isRequestingAmmo() { return requestingAmmo; }
	void setRequestingHeal(bool v) { requestingHeal = v; }
	void setRequestingAmmo(bool v) { requestingAmmo = v; }
	int getAssignedMedicId() { return assignedMedicId; }
	int getAssignedSupplierId() { return assignedSupplierId; }
	void setAssignedMedicId(int id) { assignedMedicId = id; }
	void setAssignedSupplierId(int id) { assignedSupplierId = id; }
	void clearHealRequest() { requestingHeal = false; assignedMedicId = -1; }
	void clearAmmoRequest() { requestingAmmo = false; assignedSupplierId = -1; }
};

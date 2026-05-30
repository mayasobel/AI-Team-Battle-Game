#pragma once
#include "Globals.h"
#include "Personality.h"
#include "State.h"
#include <vector>
#include <utility>

class Room;

class BaseAgent
{
protected:
	double x, y;
	double targetX, targetY;
	double dirX, dirY;
	int currentRoom;
	int targetRoom;
	double health;
	int teamId;
	int agentId;
	bool alive;
	bool isMoving;
	Personality personality;
	State* pCurrentState;
	std::vector<int> roomPath;
	int pathIndex;
	std::vector<std::pair<int, int>> cellPath;
	int cellPathIdx;

public:
	BaseAgent(int team, int id, int startRoom, double startX, double startY);
	virtual ~BaseAgent();

	virtual void show() = 0;
	virtual void update() = 0;
	virtual AgentType getType() = 0;
	virtual bool canAttack() { return false; }
	virtual double getSpeed() { return AGENT_SPEED; }

	void moveToward(double tx, double ty);
	void computeCellPath(int endRow, int endCol);
	void doMovement();
	void takeDamage(double damage);
	void takeDamage(double damage, int attackerTeam);
	int findCurrentRoom();

	// Getters
	double getX() { return x; }
	double getY() { return y; }
	int getCurrentRoom() { return currentRoom; }
	int getTargetRoom() { return targetRoom; }
	double getHealth() { return health; }
	int getTeamId() { return teamId; }
	int getAgentId() { return agentId; }
	bool isAlive() { return alive; }
	bool getIsMoving() { return isMoving; }
	Personality& getPersonality() { return personality; }
	State* getCurrentState() { return pCurrentState; }

	// Setters
	void setCurrentRoom(int room) { currentRoom = room; }
	void setTargetRoom(int room) { targetRoom = room; }
	void setIsMoving(bool value) { isMoving = value; }
	void setCurrentState(State* s);
	void setRoomPath(std::vector<int> path) { roomPath = path; pathIndex = 0; }
	void setHealth(double h) { health = h; if (health > MAX_HEALTH) health = MAX_HEALTH; }
};

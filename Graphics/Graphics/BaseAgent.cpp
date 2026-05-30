#include "BaseAgent.h"
#include "Room.h"
#include "Cell.h"
#include "CompareCells.h"
#include "SafetyMap.h"
#include <math.h>
#include <queue>
#include <vector>
#include <algorithm>


extern Room* rooms[NUM_ROOMS];
extern int maze[MSZ][MSZ];
extern SafetyMap safetyMap;

BaseAgent::BaseAgent(int team, int id, int startRoom, double startX, double startY)
{
	teamId = team;
	agentId = id;
	currentRoom = startRoom;
	targetRoom = startRoom;
	x = startX;
	y = startY;
	targetX = startX;
	targetY = startY;
	dirX = dirY = 0;
	health = MAX_HEALTH;
	alive = true;
	isMoving = false;
	pCurrentState = nullptr;
	pathIndex = 0;
	cellPathIdx = 0;
	personality.randomize();
}

BaseAgent::~BaseAgent()
{
	delete pCurrentState;
}

// A* on the grid, cost factors in the safety map
void BaseAgent::computeCellPath(int endRow, int endCol)
{
	cellPath.clear();
	cellPathIdx = 0;

	int sr = (int)y;
	int sc = (int)x;

	if (sr < 0) sr = 0; if (sr >= MSZ) sr = MSZ - 1;
	if (sc < 0) sc = 0; if (sc >= MSZ) sc = MSZ - 1;
	if (endRow < 0) endRow = 0; if (endRow >= MSZ) endRow = MSZ - 1;
	if (endCol < 0) endCol = 0; if (endCol >= MSZ) endCol = MSZ - 1;

	if (sr == endRow && sc == endCol) return;

	// snap to nearest open cell if we're inside a wall
	if (maze[sr][sc] == WALL)
	{
		bool found = false;
		for (int rad = 1; rad <= 5 && !found; rad++)
			for (int r = sr - rad; r <= sr + rad && !found; r++)
				for (int c = sc - rad; c <= sc + rad && !found; c++)
					if (r >= 0 && r < MSZ && c >= 0 && c < MSZ && maze[r][c] == SPACE)
					{ sr = r; sc = c; found = true; }
		if (!found) return;
	}

	if (maze[endRow][endCol] == WALL)
	{
		bool found = false;
		for (int rad = 1; rad <= 5 && !found; rad++)
			for (int r = endRow - rad; r <= endRow + rad && !found; r++)
				for (int c = endCol - rad; c <= endCol + rad && !found; c++)
					if (r >= 0 && r < MSZ && c >= 0 && c < MSZ && maze[r][c] == SPACE)
					{ endRow = r; endCol = c; found = true; }
		if (!found) return;
	}

	static int localMap[MSZ][MSZ];
	for (int i = 0; i < MSZ; i++)
		for (int j = 0; j < MSZ; j++)
			localMap[i][j] = WHITE;

	std::priority_queue<Cell*, std::vector<Cell*>, CompareCells> pq;
	std::vector<Cell> grays;
	std::vector<Cell*> allocated;

	Cell* start = new Cell(sr, sc, endRow, endCol);
	allocated.push_back(start);
	localMap[sr][sc] = GRAY;
	grays.push_back(*start);
	pq.push(start);

	Cell* found = nullptr;
	const int dr[] = { 1, -1, 0, 0 };
	const int dc[] = { 0, 0, -1, 1 };

	while (!pq.empty())
	{
		Cell* cur = pq.top();

		if (cur->getRow() == endRow && cur->getCol() == endCol)
		{
			found = cur;
			break;
		}

		pq.pop();
		std::vector<Cell>::iterator itg = std::find(grays.begin(), grays.end(), *cur);
		if (itg == grays.end()) continue;
		grays.erase(itg);

		int r = cur->getRow();
		int c = cur->getCol();
		localMap[r][c] = BLACK;

		for (int d = 0; d < 4; d++)
		{
			int nr = r + dr[d];
			int nc = c + dc[d];
			if (nr < 0 || nr >= MSZ || nc < 0 || nc >= MSZ) continue;
			if (maze[nr][nc] == WALL) continue;
			if (localMap[nr][nc] == BLACK) continue;

			double cost = 1.0 + safetyMap.getCellRisk(nr, nc) * personality.riskAversion * 10.0;
			Cell* nb = new Cell(nr, nc, cur, cost);
			allocated.push_back(nb);

			if (localMap[nr][nc] == WHITE)
			{
				pq.push(nb);
				localMap[nr][nc] = GRAY;
				grays.push_back(*nb);
			}
			else if (localMap[nr][nc] == GRAY)
			{
				std::vector<Cell>::iterator it = std::find(grays.begin(), grays.end(), *nb);
				if (it != grays.end() && nb->getF() < it->getF())
				{
					grays.erase(it);
					grays.push_back(*nb);
					std::vector<Cell*> tmp;
					while (!pq.empty() && !(*(pq.top()) == *nb))
					{
						tmp.push_back(pq.top());
						pq.pop();
					}
					if (!pq.empty())
					{
						pq.pop();
						pq.push(nb);
					}
					while (!tmp.empty())
					{
						pq.push(tmp.back());
						tmp.pop_back();
					}
				}
			}
		}
	}

	if (found)
	{
		std::vector<std::pair<int, int>> full;
		Cell* pc = found;
		while (pc != nullptr)
		{
			full.push_back(std::make_pair(pc->getRow(), pc->getCol()));
			pc = pc->getParent();
		}

		std::vector<std::pair<int, int>> ordered;
		for (int i = (int)full.size() - 1; i >= 0; i--)
			ordered.push_back(full[i]);

		if (ordered.size() <= 2)
		{
			cellPath = ordered;
		}
		else
		{
			// only keep corners (where direction changes)
			cellPath.push_back(ordered[0]);
			for (size_t i = 1; i < ordered.size() - 1; i++)
			{
				int pdr = ordered[i].first  - ordered[i - 1].first;
				int pdc = ordered[i].second - ordered[i - 1].second;
				int ndr = ordered[i + 1].first  - ordered[i].first;
				int ndc = ordered[i + 1].second - ordered[i].second;
				if (pdr != ndr || pdc != ndc)
					cellPath.push_back(ordered[i]);
			}
			cellPath.push_back(ordered.back());
		}
	}

	for (size_t i = 0; i < allocated.size(); i++)
		delete allocated[i];
}

void BaseAgent::moveToward(double tx, double ty)
{
	targetX = tx;
	targetY = ty;

	computeCellPath((int)ty, (int)tx);

	if (cellPath.size() > 1)
	{
		cellPathIdx = 1;
		isMoving = true;
		double wpX = cellPath[cellPathIdx].second + 0.5;
		double wpY = cellPath[cellPathIdx].first  + 0.5;
		double dx = wpX - x;
		double dy = wpY - y;
		double len = sqrt(dx * dx + dy * dy);
		if (len > 0.01) { dirX = dx / len; dirY = dy / len; }
	}
	else
	{
		isMoving = false;
	}
}

void BaseAgent::doMovement()
{
	currentRoom = findCurrentRoom();
	if (!isMoving || !alive) return;

	if (cellPath.empty() || cellPathIdx >= (int)cellPath.size())
	{
		isMoving = false;
		currentRoom = findCurrentRoom();

		if (!roomPath.empty() && pathIndex < (int)roomPath.size())
		{
			if (currentRoom == roomPath[pathIndex])
			{
				pathIndex++;
				if (pathIndex < (int)roomPath.size())
				{
					int nextRoom = roomPath[pathIndex];
					moveToward(rooms[nextRoom]->getCenterCol(),
					           rooms[nextRoom]->getCenterRow());
				}
			}
		}
		return;
	}

	double wpX = cellPath[cellPathIdx].second + 0.5;
	double wpY = cellPath[cellPathIdx].first  + 0.5;
	double dx = wpX - x;
	double dy = wpY - y;
	double dist = sqrt(dx * dx + dy * dy);

	double spd = getSpeed();
	if (dist < spd + 0.15)
	{
		x = wpX;
		y = wpY;
		cellPathIdx++;

		if (cellPathIdx >= (int)cellPath.size())
		{
			isMoving = false;
			currentRoom = findCurrentRoom();

			if (!roomPath.empty() && pathIndex < (int)roomPath.size())
			{
				if (currentRoom == roomPath[pathIndex])
				{
					pathIndex++;
					if (pathIndex < (int)roomPath.size())
					{
						int nextRoom = roomPath[pathIndex];
						moveToward(rooms[nextRoom]->getCenterCol(),
						           rooms[nextRoom]->getCenterRow());
					}
				}
			}
		}
		else
		{
			wpX = cellPath[cellPathIdx].second + 0.5;
			wpY = cellPath[cellPathIdx].first  + 0.5;
			dx = wpX - x;
			dy = wpY - y;
			dist = sqrt(dx * dx + dy * dy);
			if (dist > 0.01)
			{
				dirX = dx / dist;
				dirY = dy / dist;
			}
		}
	}
	else
	{
		dirX = dx / dist;
		dirY = dy / dist;
		x += spd * dirX;
		y += spd * dirY;
	}
}

void BaseAgent::takeDamage(double damage)
{
	if (!alive) return;
	health -= damage;
	if (health <= 0)
	{
		health = 0;
		alive = false;
	}
}

// ignores damage from own team
void BaseAgent::takeDamage(double damage, int attackerTeam)
{
	if (!alive) return;
	if (attackerTeam == teamId) return;
	health -= damage;
	if (health <= 0)
	{
		health = 0;
		alive = false;
	}
}

int BaseAgent::findCurrentRoom()
{
	for (int i = 0; i < NUM_ROOMS; i++)
		if (rooms[i] && rooms[i]->isInside(x, y))
			return i;
	return currentRoom;
}

// won't let medic/supplier enter combat states
void BaseAgent::setCurrentState(State* s)
{
	if (s && s->requiresFighter() && getType() != AGENT_FIGHTER)
	{
		delete s;
		return;
	}

	if (pCurrentState)
	{
		pCurrentState->OnExit(this);
		delete pCurrentState;
	}
	pCurrentState = s;
	if (pCurrentState)
		pCurrentState->OnEnter(this);
}

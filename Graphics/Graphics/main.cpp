#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <vector>
#include <queue>
#include <algorithm>
#include <cstdio>
#include "glut.h"
#include "Globals.h"
#include "Room.h"
#include "Cell.h"
#include "CompareCells.h"
#include "Bullet.h"
#include "Grenade.h"
#include "SafetyMap.h"
#include "Team.h"
#include "Fighter.h"
#include "Medic.h"
#include "Supplier.h"

using namespace std;

int maze[MSZ][MSZ] = { 0 };
int astarMap[MSZ][MSZ];
Room* rooms[NUM_ROOMS] = { 0 };
Team* teams[2] = { nullptr, nullptr };
SafetyMap safetyMap;

bool gameOver = false;
int winningTeam = -1;

// just for rendering, damage already happened in Fighter
struct VisualBullet {
	double x, y, dirx, diry;
	int timer;
	int team;
};

struct ActiveGrenade {
	Grenade* grenade;
	int timer;
};

vector<VisualBullet> visualBullets;
vector<ActiveGrenade> activeGrenades;

void generateDungeon();
void setupTeams();

void init()
{
	srand((unsigned)time(0));
	glClearColor(0.3f, 0.3f, 0.3f, 0);
	glOrtho(0, MSZ, 0, MSZ, -1, 1);
	generateDungeon();
	setupTeams();
}

bool overlap(int index, int row, int col, int w, int h)
{
	int hdist, vdist, gap = 5;
	bool hasOverlap = false;

	for (int i = 0; !hasOverlap && i < index; i++)
	{
		vdist = abs(rooms[i]->getCenterRow() - row);
		hdist = abs(rooms[i]->getCenterCol() - col);
		if (vdist - gap < h / 2 + rooms[i]->getHeight() / 2 &&
			hdist - gap < w / 2 + rooms[i]->getWidth() / 2)
			hasOverlap = true;
	}
	return hasOverlap;
}

void addRoom(Room* prm)
{
	int start_row = prm->getCenterRow() - prm->getHeight() / 2;
	int start_col = prm->getCenterCol() - prm->getWidth() / 2;
	int end_row = prm->getCenterRow() + prm->getHeight() / 2;
	int end_col = prm->getCenterCol() + prm->getWidth() / 2;

	for (int i = start_row; i <= end_row; i++)
		for (int j = start_col; j <= end_col; j++)
			maze[i][j] = SPACE;
}

void createAstarMap()
{
	for (int i = 0; i < MSZ; i++)
		for (int j = 0; j < MSZ; j++)
			astarMap[i][j] = WHITE;
}

// going through walls has higher cost so corridors reuse open areas when possible
void AddNeighbor(int r, int c, Cell* pCurrent,
	priority_queue<Cell*, vector<Cell*>, CompareCells>& pq,
	vector<Cell>& grays)
{
	Cell* pNeighbor = nullptr;
	double cost = 0;
	vector<Cell>::iterator itg;

	if (astarMap[r][c] == WHITE || astarMap[r][c] == GRAY)
	{
		if (maze[r][c] == SPACE) cost = 0.1;
		else if (maze[r][c] == WALL) cost = 2.6;
		pNeighbor = new Cell(r, c, pCurrent, cost);

		if (astarMap[r][c] == WHITE)
		{
			pq.push(pNeighbor);
			astarMap[r][c] = GRAY;
			grays.push_back(*pNeighbor);
		}
		else if (astarMap[r][c] == GRAY)
		{
			itg = find(grays.begin(), grays.end(), *pNeighbor);
			if (itg != grays.end() && pNeighbor->getF() < itg->getF())
			{
				grays.erase(itg);
				grays.push_back(*pNeighbor);
				vector<Cell*> tmp;
				while (!pq.empty() && !(*(pq.top()) == *pNeighbor))
				{
					tmp.push_back(pq.top());
					pq.pop();
				}
				if (!pq.empty())
				{
					pq.pop();
					pq.push(pNeighbor);
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

// turns the A* result into actual walkable corridor
void RestorePath(Cell* pc)
{
	while (pc != nullptr)
	{
		maze[pc->getRow()][pc->getCol()] = SPACE;
		pc = pc->getParent();
	}
}

// uses A* to dig a corridor between two rooms
void BuildPath(int index1, int index2)
{
	int r, c;
	Cell* pCurrent = nullptr;
	createAstarMap();
	priority_queue<Cell*, vector<Cell*>, CompareCells> pq;
	vector<Cell> grays;
	vector<Cell>::iterator itg;

	Cell* pc = new Cell(rooms[index1]->getCenterRow(), rooms[index1]->getCenterCol(),
		rooms[index2]->getCenterRow(), rooms[index2]->getCenterCol());
	astarMap[rooms[index1]->getCenterRow()][rooms[index1]->getCenterCol()] = GRAY;
	grays.push_back(*pc);
	pq.push(pc);

	while (!pq.empty())
	{
		pCurrent = pq.top();
		if (pCurrent->getH() < 0.1)
		{
			RestorePath(pCurrent);
			rooms[index1]->addConnection(index2);
			rooms[index2]->addConnection(index1);
			return;
		}
		else
		{
			pq.pop();
			itg = find(grays.begin(), grays.end(), *pCurrent);
			if (itg == grays.end()) return;

			grays.erase(itg);
			r = pCurrent->getRow();
			c = pCurrent->getCol();
			astarMap[r][c] = BLACK;

			if (r < MSZ - 1) AddNeighbor(r + 1, c, pCurrent, pq, grays);
			if (r > 0) AddNeighbor(r - 1, c, pCurrent, pq, grays);
			if (c > 0) AddNeighbor(r, c - 1, pCurrent, pq, grays);
			if (c < MSZ - 1) AddNeighbor(r, c + 1, pCurrent, pq, grays);
		}
	}
}

void generateDungeon()
{
	int r, c, w, h;
	int gap = 6;
	int hrange, vrange;

	// keep trying random sizes/positions until the room doesn't overlap
	for (int i = 0; i < NUM_ROOMS; i++)
	{
		do
		{
			w = 8 + rand() % 15;
			h = 8 + rand() % 15;
			vrange = MSZ - 2 * (gap + h / 2);
			hrange = MSZ - 2 * (gap + w / 2);
			r = gap + h / 2 + rand() % vrange;
			c = gap + w / 2 + rand() % hrange;
		} while (overlap(i, r, c, w, h));

		rooms[i] = new Room(r, c, w, h);
		addRoom(rooms[i]);
	}

	// connect every pair so you can always walk between any two rooms
	for (int i = 0; i < NUM_ROOMS; i++)
		for (int j = i + 1; j < NUM_ROOMS; j++)
			BuildPath(i, j);

	for (int i = 0; i < NUM_ROOMS; i++)
		rooms[i]->addObstacles(maze);

	// 2 ammo depots + 2 medical depots, placed randomly
	vector<int> indices;
	for (int i = 0; i < NUM_ROOMS; i++) indices.push_back(i);
	for (int i = (int)indices.size() - 1; i > 0; i--)
	{
		int j = rand() % (i + 1);
		swap(indices[i], indices[j]);
	}

	rooms[indices[0]]->setDepot(DEPOT_AMMO);
	rooms[indices[1]]->setDepot(DEPOT_AMMO);
	rooms[indices[2]]->setDepot(DEPOT_MEDICAL);
	rooms[indices[3]]->setDepot(DEPOT_MEDICAL);
}

// left half = team 0, right half = team 1
void setupTeams()
{
	teams[0] = new Team(0);
	teams[1] = new Team(1);

	vector<pair<int, int>> roomsByCol;
	for (int i = 0; i < NUM_ROOMS; i++)
		roomsByCol.push_back(make_pair(rooms[i]->getCenterCol(), i));
	sort(roomsByCol.begin(), roomsByCol.end());

	int team0Rooms[4], team1Rooms[4];
	for (int i = 0; i < 4 && i < NUM_ROOMS; i++)
		team0Rooms[i] = roomsByCol[i].second;
	for (int i = 0; i < 4 && i + 4 < NUM_ROOMS; i++)
		team1Rooms[i] = roomsByCol[i + 4].second;

	int aid = 0;

	for (int i = 0; i < 2; i++)
	{
		int rm = team0Rooms[i];
		double sx = rooms[rm]->getCenterCol() + (i == 0 ? -2.0 : 2.0);
		double sy = rooms[rm]->getCenterRow();
		teams[0]->addFighter(new Fighter(0, aid++, rm, sx, sy));
	}
	{
		int rm = team0Rooms[2];
		teams[0]->setMedic(new Medic(0, aid++, rm,
			rooms[rm]->getCenterCol(), rooms[rm]->getCenterRow()));
	}
	{
		int rm = team0Rooms[3];
		teams[0]->setSupplier(new Supplier(0, aid++, rm,
			rooms[rm]->getCenterCol(), rooms[rm]->getCenterRow()));
	}

	for (int i = 0; i < 2; i++)
	{
		int rm = team1Rooms[i];
		double sx = rooms[rm]->getCenterCol() + (i == 0 ? -2.0 : 2.0);
		double sy = rooms[rm]->getCenterRow();
		teams[1]->addFighter(new Fighter(1, aid++, rm, sx, sy));
	}
	{
		int rm = team1Rooms[2];
		teams[1]->setMedic(new Medic(1, aid++, rm,
			rooms[rm]->getCenterCol(), rooms[rm]->getCenterRow()));
	}
	{
		int rm = team1Rooms[3];
		teams[1]->setSupplier(new Supplier(1, aid++, rm,
			rooms[rm]->getCenterCol(), rooms[rm]->getCenterRow()));
	}
}

void DrawDungeon()
{
	for (int i = 0; i < MSZ; i++)
		for (int j = 0; j < MSZ; j++)
		{
			double risk = safetyMap.getCellRisk(i, j);
			switch (maze[i][j])
			{
			case WALL:
				glColor3d(0.25, 0.15, 0.1);
				break;
			case SPACE:
				// floor gets darker where there was recent fighting
				glColor3d(0.85 - risk * 0.5, 0.85 - risk * 0.5, 0.85 - risk * 0.3);
				break;
			}
			glBegin(GL_POLYGON);
			glVertex2d(j, i);
			glVertex2d(j, i + 1);
			glVertex2d(j + 1, i + 1);
			glVertex2d(j + 1, i);
			glEnd();
		}
}

void DrawDepots()
{
	for (int i = 0; i < NUM_ROOMS; i++)
	{
		if (rooms[i]->getDepot() == DEPOT_NONE) continue;

		double cx = rooms[i]->getCenterCol();
		double cy = rooms[i]->getCenterRow();
		double off = rooms[i]->getHeight() / 2.0 - 2;

		if (rooms[i]->getDepot() == DEPOT_AMMO)
		{
			glColor3d(0.9, 0.9, 0.0);
			glBegin(GL_POLYGON);
			glVertex2d(cx - 1.5, cy - off - 1.5);
			glVertex2d(cx - 1.5, cy - off);
			glVertex2d(cx + 1.5, cy - off);
			glVertex2d(cx + 1.5, cy - off - 1.5);
			glEnd();
			glColor3d(0, 0, 0);
			glRasterPos2d(cx - 0.4, cy - off - 1.2);
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'A');
		}
		else if (rooms[i]->getDepot() == DEPOT_MEDICAL)
		{
			glColor3d(0.0, 0.8, 0.0);
			glBegin(GL_POLYGON);
			glVertex2d(cx - 1.5, cy - off - 1.5);
			glVertex2d(cx - 1.5, cy - off);
			glVertex2d(cx + 1.5, cy - off);
			glVertex2d(cx + 1.5, cy - off - 1.5);
			glEnd();
			glColor3d(1, 1, 1);
			glRasterPos2d(cx - 0.4, cy - off - 1.2);
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, '+');
		}
	}
}

void DrawRoomLabels()
{
	glColor3d(0.4, 0.4, 0.4);
	for (int i = 0; i < NUM_ROOMS; i++)
	{
		glRasterPos2d(rooms[i]->getCenterCol() - 0.5,
		              rooms[i]->getCenterRow() + rooms[i]->getHeight() / 2.0 - 1);
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, '0' + i);
	}
}

void DrawAgents()
{
	for (int t = 0; t < 2; t++)
		for (size_t i = 0; i < teams[t]->getAllAgents().size(); i++)
			if (teams[t]->getAllAgents()[i]->isAlive())
				teams[t]->getAllAgents()[i]->show();
}

void DrawText(double x, double y, const char* text)
{
	glRasterPos2d(x, y);
	for (const char* c = text; *c; c++)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
}

void DrawHUD()
{
	char buf[128];

	glColor3d(0.3, 0.3, 1.0);
	sprintf(buf, "BLUE: %d alive", teams[0]->aliveCount());
	DrawText(2, MSZ - 3, buf);

	glColor3d(1.0, 0.3, 0.3);
	sprintf(buf, "RED: %d alive", teams[1]->aliveCount());
	DrawText(MSZ - 22, MSZ - 3, buf);

	if (gameOver)
	{
		glColor3d(1, 1, 0);
		if (winningTeam == -2)
			DrawText(MSZ / 2.0 - 15, MSZ / 2.0, "AMMO FINISHED - GAME OVER!");
		else if (winningTeam == 0)
			DrawText(MSZ / 2.0 - 12, MSZ / 2.0, "BLUE TEAM WINS!");
		else if (winningTeam == 1)
			DrawText(MSZ / 2.0 - 12, MSZ / 2.0, "RED TEAM WINS!");
		else
			DrawText(MSZ / 2.0 - 5, MSZ / 2.0, "DRAW!");
	}
}

void DrawProjectiles()
{
	for (size_t i = 0; i < visualBullets.size(); i++)
	{
		if (visualBullets[i].team == 0)
			glColor3d(0.2, 0.5, 1.0);
		else
			glColor3d(1.0, 0.4, 0.2);

		double bx = visualBullets[i].x;
		double by = visualBullets[i].y;
		glBegin(GL_POLYGON);
		glVertex2d(bx - 0.3, by);
		glVertex2d(bx, by + 0.3);
		glVertex2d(bx + 0.3, by);
		glVertex2d(bx, by - 0.3);
		glEnd();
	}

	for (size_t i = 0; i < activeGrenades.size(); i++)
		activeGrenades[i].grenade->show();
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);
	DrawDungeon();
	DrawDepots();
	DrawRoomLabels();
	DrawAgents();
	DrawProjectiles();
	DrawHUD();
	glutSwapBuffers();
}

void updateGame()
{
	if (gameOver) return;

	safetyMap.decayRisk();

	for (int t = 0; t < 2; t++)
		for (size_t i = 0; i < teams[t]->getAllAgents().size(); i++)
			if (teams[t]->getAllAgents()[i]->isAlive())
				teams[t]->getAllAgents()[i]->update();

	// if both teams have someone in the same room, that room gets riskier
	for (int i = 0; i < NUM_ROOMS; i++)
	{
		bool has0 = false, has1 = false;
		for (size_t a = 0; a < teams[0]->getAllAgents().size(); a++)
			if (teams[0]->getAllAgents()[a]->isAlive() &&
				teams[0]->getAllAgents()[a]->getCurrentRoom() == i)
				has0 = true;
		for (size_t a = 0; a < teams[1]->getAllAgents().size(); a++)
			if (teams[1]->getAllAgents()[a]->isAlive() &&
				teams[1]->getAllAgents()[a]->getCurrentRoom() == i)
				has1 = true;

		if (has0 && has1)
		{
			safetyMap.addCombatRisk(i, rooms[i], maze);

			// moving agents also leave a small risk trail around them
			for (int t = 0; t < 2; t++)
				for (size_t a = 0; a < teams[t]->getAllAgents().size(); a++)
				{
					BaseAgent* ag = teams[t]->getAllAgents()[a];
					if (ag->isAlive() && ag->getCurrentRoom() == i && ag->getIsMoving())
					{
						int cr = (int)ag->getY();
						int cc = (int)ag->getX();
						if (cr >= 0 && cr < MSZ && cc >= 0 && cc < MSZ)
						{
							for (int dr = -2; dr <= 2; dr++)
								for (int dc = -2; dc <= 2; dc++)
								{
									int nr = cr + dr, nc = cc + dc;
									if (nr >= 0 && nr < MSZ && nc >= 0 && nc < MSZ)
										safetyMap.cellRisk[nr][nc] += SECURITY_SCORE * 0.5;
								}
						}
					}
				}
		}
	}

	// spawn visual projectiles
	for (int t = 0; t < 2; t++)
	{
		vector<Fighter*>& fighters = teams[t]->getFighters();
		for (size_t i = 0; i < fighters.size(); i++)
		{
			Fighter* f = fighters[i];
			if (!f->isAlive() || !f->getJustFired()) continue;

			if (f->getLastWasGrenade())
			{
				// grenade explosion at the target location, not the shooter
				ActiveGrenade ag;
				ag.grenade = new Grenade(f->getLastTargetX(), f->getLastTargetY(), t);
				ag.grenade->setIsExploding(true);
				ag.grenade->startExploding();
				ag.timer = 25;
				activeGrenades.push_back(ag);
			}
			else
			{
				VisualBullet vb;
				vb.x = f->getX();
				vb.y = f->getY();
				double dx = f->getLastTargetX() - f->getX();
				double dy = f->getLastTargetY() - f->getY();
				double len = sqrt(dx * dx + dy * dy);
				if (len > 0.01) { vb.dirx = dx / len; vb.diry = dy / len; }
				else { vb.dirx = 1; vb.diry = 0; }
				vb.timer = 20;
				vb.team = t;
				visualBullets.push_back(vb);
			}
			f->clearCombatFlags();
		}
	}

	// move bullets
	for (size_t i = 0; i < visualBullets.size(); i++)
	{
		visualBullets[i].x += visualBullets[i].dirx * BULLET_SPEED;
		visualBullets[i].y += visualBullets[i].diry * BULLET_SPEED;
		visualBullets[i].timer--;
		int ix = (int)visualBullets[i].x;
		int iy = (int)visualBullets[i].y;
		if (ix < 0 || ix >= MSZ || iy < 0 || iy >= MSZ || maze[iy][ix] == WALL)
			visualBullets[i].timer = 0;
	}
	for (auto it = visualBullets.begin(); it != visualBullets.end();)
	{
		if (it->timer <= 0) it = visualBullets.erase(it);
		else ++it;
	}

	// move grenades
	for (size_t i = 0; i < activeGrenades.size(); i++)
	{
		activeGrenades[i].grenade->explode(maze);
		activeGrenades[i].timer--;
	}
	for (auto it = activeGrenades.begin(); it != activeGrenades.end();)
	{
		if (it->timer <= 0)
		{
			delete it->grenade;
			it = activeGrenades.erase(it);
		}
		else ++it;
	}

	// check win
	bool d0 = teams[0]->isDefeated();
	bool d1 = teams[1]->isDefeated();
	if (d0 && d1) { gameOver = true; winningTeam = -1; }
	else if (d0) { gameOver = true; winningTeam = 1; }
	else if (d1) { gameOver = true; winningTeam = 0; }

	// game over if nobody has ammo left
	if (!gameOver)
	{
		bool allOutOfAmmo = true;
		for (int t = 0; t < 2 && allOutOfAmmo; t++)
		{
			Supplier* sup = teams[t]->getSupplier();
			bool supplierCanProvide = sup && sup->isAlive() && sup->hasSupply();
			if (supplierCanProvide) { allOutOfAmmo = false; break; }

			vector<Fighter*>& f = teams[t]->getFighters();
			for (size_t i = 0; i < f.size(); i++)
			{
				if (f[i]->isAlive() && f[i]->getAmmo() > 0)
				{ allOutOfAmmo = false; break; }
			}
		}
		if (allOutOfAmmo) { gameOver = true; winningTeam = -2; }
	}
}

void timer(int value)
{
	updateGame();
	glutPostRedisplay();
	glutTimerFunc(16, timer, 0);
}

void idle()
{
}

void restartGame()
{
	for (int t = 0; t < 2; t++)
	{
		if (teams[t]) { delete teams[t]; teams[t] = nullptr; }
	}
	for (int i = 0; i < NUM_ROOMS; i++)
	{
		if (rooms[i]) { delete rooms[i]; rooms[i] = nullptr; }
	}

	visualBullets.clear();
	for (size_t i = 0; i < activeGrenades.size(); i++)
		delete activeGrenades[i].grenade;
	activeGrenades.clear();

	for (int r = 0; r < MSZ; r++)
		for (int c = 0; c < MSZ; c++)
			maze[r][c] = 0;

	safetyMap = SafetyMap();
	gameOver = false;
	winningTeam = -1;

	generateDungeon();
	setupTeams();
	glutPostRedisplay();
}

void menu(int choice)
{
	switch (choice)
	{
	case 1:
		restartGame();
		break;
	}
}

void main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(W_WIDTH, W_HEIGHT);
	glutInitWindowPosition(400, 100);
	glutCreateWindow("AI Combat Simulation");

	glutDisplayFunc(display);
	glutIdleFunc(idle);

	glutCreateMenu(menu);
	glutAddMenuEntry("Restart Game", 1);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	init();

	glutTimerFunc(16, timer, 0);
	glutMainLoop();
}

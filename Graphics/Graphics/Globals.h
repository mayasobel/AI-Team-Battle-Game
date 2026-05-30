#pragma once

// Maze grid constants
const int MSZ = 100;
const int SPACE = 1;
const int WALL = 0;
const int OBSTACLE = 2;

// Pathfinding cell states
const int WHITE = 0;
const int GRAY = 1;
const int BLACK = 2;

// Display and layout
const int W_WIDTH = 600;
const int W_HEIGHT = 600;
const int NUM_ROOMS = 8;

// Movement speeds
const double BULLET_SPEED = 0.3;
const double AGENT_SPEED = 0.08;
const double SUPPORT_SPEED = 0.14;
const int CRITICAL_HP_THRESHOLD = 25;

// Combat
const double BULLET_DAMAGE = 10.0;
const double GRENADE_DAMAGE = 5.0;
const double SECURITY_SCORE = 0.002;

// Fighter and grenade limits
const int NUM_GRENADE_BULLETS = 20;
const int MAX_HEALTH = 100;
const int FIGHTER_MAX_AMMO = 30;
const int FIGHTER_START_AMMO = 15;
const int GRENADE_AMMO_COST = 3;
const int ATTACK_COOLDOWN = 15;

// Support agent limits
const int MEDIC_MAX_SUPPLY = 30;
const int SUPPLIER_MAX_SUPPLY = 30;
const int SUPPLY_RESTOCK_THRESHOLD = 3;
const double SUPPORT_INTERACT_RANGE = 3.0;

enum DepotType { DEPOT_NONE = 0, DEPOT_AMMO = 1, DEPOT_MEDICAL = 2 };
enum AgentType { AGENT_FIGHTER = 0, AGENT_MEDIC = 1, AGENT_SUPPLIER = 2 };

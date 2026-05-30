#pragma once
#include "Globals.h"

class Cell
{
private:
	int row, col;
	int target_row, target_col;
	double f, g, h;
	Cell* parent;
public:
	Cell(int r, int c, int tr, int tc);
	Cell(int r, int c, Cell* p, double cost);
	double ComputeH();
	double ComputeF();
	double getF() { return f; }
	double getH() { return h; }
	int getRow() { return row; }
	int getCol() { return col; }
	Cell* getParent() { return parent; }
	bool operator ==(const Cell& other) { return row == other.row && col == other.col; }
};

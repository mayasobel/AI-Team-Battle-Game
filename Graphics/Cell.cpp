#include <math.h>
#include "Cell.h"

Cell::Cell(int r, int c, int tr, int tc)
{
	row = r;
	col = c;
	target_row = tr;
	target_col = tc;
	parent = nullptr;
	g = 0;
	h = ComputeH();
	f = ComputeF();

}

Cell::Cell(int r, int c, Cell* p,double cost)
{
	row = r;
	col = c;
	parent = p;
	target_row = p->target_row;
	target_col = p->target_col;
	g = p->g+cost;
	h = ComputeH();
	f = ComputeF();
}

// Manhattan Distance
double Cell::ComputeH()
{
	return fabs(target_row-row)+fabs(target_col-col);
}

double Cell::ComputeF()
{
	return g + h;
}

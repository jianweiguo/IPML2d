#ifndef TURTLE_H
#define TURTLE_H
#include <stack>
#include "R2/R2.h"
#include "R3/R3.h"
#include <qmath.h>
#include <vector>
//#include "R3Mesh.h"
using namespace std;

struct t_line{
	R2Vector p_start;
	R2Vector p_end;
	int group_id;
	R2Vector direction;
	int class_id;
	float width;
	int iteration;
	int branch_status; //0: main branch, 1: left(+), 2: right(-)
	int strahler_number;

	t_line(R2Vector s, R2Vector t) :p_start(s), p_end(t)
	{
		iteration = 1;
		branch_status = 0;
		strahler_number = 0;
	}
	t_line(R2Vector s, R2Vector t, int id, R2Vector dir) :p_start(s), p_end(t), group_id(id), direction(dir)
	{
		iteration = 1;
		branch_status = 0;
		strahler_number = 0;
	}
	t_line(R2Vector s, R2Vector t, int id, int cid, R2Vector dir) :p_start(s), p_end(t), group_id(id), class_id(cid), direction(dir)
	{
		iteration = 1;
		branch_status = 0;
		strahler_number = 0;
	}
	t_line(R2Vector s, R2Vector t, int id, int cid, R2Vector dir, float w) :p_start(s), p_end(t), group_id(id), class_id(cid), direction(dir), width(w)
	{
		iteration = 1;
		branch_status = 0;
		strahler_number = 0;
	}
};

inline double random_number(double value, int thres){
	double rand_val = (rand() % (thres*2))-thres;
	double new_val = rand_val + value;
	return new_val;
}

inline int randInt(int low, int high)
{
	// Random number between low and high
	return qrand() % ((high + 1) - low) + low;
}


struct Turtle
{
	R3Vector position;
	R3Vector direction;
	R3Vector right;
	float scalar;
	int class_id;

	int iter_num;
	int branch_status; //0: main branch, 1: left(+), 2: right(-)
	int strahler_number;

	/*R2Vector position;
	R2Vector direction;
	R2Vector right;*/

	float thickness;
	float reduction;
	Turtle();
	void turnRight(float angle);
	void turnLeft(float angle);
	void pitchDown(float angle);
	void pitchUp(float angle);
	void rollLeft(float angle);
	void rollRight(float angle);
	void move(float distance);
	void turn180(float temp);
	void thicken(float param);
	void narrow(float param);
	void setThickness(float param);
	void setReduction(float param);
	void setScale(float param);
	void setCalssID(int param);
	void set_strahler_number(int param);

};
class TurtleSystem : public Turtle
{
	stack<Turtle> state;
	vector<t_line> tree2d;
	//R3Mesh *mesh;
	int group_id;
	stack<float> state_scalar;
	stack<float> state_thickness;
	stack<int> state_iter_num;

	bool use_random_rule;

public:
	//TurtleSystem(R3Mesh * m);
	TurtleSystem();
	void save();
	void restore();
	void save_scalar();
	void restore_scalar();
	void save_thickness();
	void restore_thickness();
	void draw(float param);
	void clear();
	void add_group_id();
	int number_groups() { return group_id; }
	//void drawLeaf(float param);
	vector<t_line> resultedTree(){ return tree2d; }
	void set_random_rule(bool random) { use_random_rule = random; }

	void add_iter_num(int offset);
	void set_branch_status(int indicator);
	void save_iter_num();
	void restore_iter_num();
};

#endif
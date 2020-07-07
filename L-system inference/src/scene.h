#ifndef _SCENE_H_
#define _SCENE_H_

// STL
#include <set>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

//Qt
#include <QtOpenGL>
#include <QString>

//// local
#include "lsystem.h"
#include "tree_structure.h"

struct Bounding_box{
	double x_min;
	double x_max;
	double y_min;
	double y_max;
	int group_id;
	QPointF center, l_t_corner, r_t_corner, r_b_corner, l_b_corner;
	//std::vector<QPointF> corners_rectified;
	double angleFromY;
	bool set_angle;
	double width, height;
	int class_id;
	int iteration;
	int branch_status; //0: main branch, 1: left(+), 2: right(-)
	Bounding_box()
	{
		x_max = -100000; x_min = 100000; y_max = -100000; y_min = 100000;
		group_id=-1;
		angleFromY = 0;
		class_id = 0;
		set_angle = false;
		branch_status = 0;
		iteration = 1;
	}
	Bounding_box(double xmin, double xmax, double ymin, double ymax, int id) :x_min(xmin), x_max(xmax), y_min(ymin), y_max(ymax), group_id(id)
	{
		angleFromY = 0;
		class_id = 0;
		set_angle = false;
		branch_status = 0;
		iteration = 1;
	}
};

class Scene
{
private:
	LSystem *m_lsystem;
    
public:
    Scene()
    {
		m_lsystem = new LSystem();
		m_random_rules = false;
    }
    
    ~Scene()
    {
    }    
    
	LSystem* lSystem() { return m_lsystem; }

	void set_random_rules(bool random);

    // IO //
	void load_rules_conduct(const QString& filename);
	void load_rules(const QString& filename);
	void draw_rules();
	void draw_rules_random();

	vector<t_line> m_tree2d;
	QPointF m_tree_center;
	double m_tree_height;
	double m_tree_width;
	double m_tree_maxlen;
	double m_tree_diagonal;
	int number_groups;
	double m_tree_scale_factor;
	vector<Bounding_box> m_bbx;
	bool m_random_rules;
};

#endif

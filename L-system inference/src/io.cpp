#include <QColor>
#include <string>
#include "scene.h"

void Scene::load_rules_conduct(const QString& filename){
	std::string fs = filename.toStdString();
	const char *descriptor_filename = fs.c_str();

	string lsystem = m_lsystem->generateFromFile(descriptor_filename, 0);
	m_lsystem->draw(lsystem);
	m_tree2d.clear();
	m_tree2d = m_lsystem->resultedTree();
	number_groups = m_lsystem->number_groups();
	for (int i = 0; i < number_groups; i++){
		Bounding_box bbx;
		bbx.group_id = i;
		m_bbx.push_back(bbx);
	}
	//get the information of the boundxing box of the tree
	double xmax = -100000, xmin = 100000, ymax = -100000, ymin = 100000;
	for (int i = 0; i < m_tree2d.size(); i++){
		t_line line = m_tree2d[i];
		if (line.p_start.X() > xmax) xmax = line.p_start.X();
		if (line.p_start.X() < xmin) xmin = line.p_start.X();
		if (-line.p_start.Y() > ymax) ymax = -line.p_start.Y(); //we reverse the y value to make the coordinate system same to the QWidget
		if (-line.p_start.Y() < ymin) ymin = -line.p_start.Y();

		if (line.p_end.X() > xmax) xmax = line.p_end.X();
		if (line.p_end.X() < xmin) xmin = line.p_end.X();
		if (-line.p_end.Y() > ymax) ymax = -line.p_end.Y();
		if (-line.p_end.Y() < ymin) ymin = -line.p_end.Y();

		int gid = line.group_id;
		if (line.p_start.X() > m_bbx[gid].x_max) m_bbx[gid].x_max = line.p_start.X();
		if (line.p_start.X() < m_bbx[gid].x_min) m_bbx[gid].x_min = line.p_start.X();
		if (-line.p_start.Y() > m_bbx[gid].y_max) m_bbx[gid].y_max = -line.p_start.Y(); //we reverse the y value to make the coordinate system same to the QWidget
		if (-line.p_start.Y() < m_bbx[gid].y_min) m_bbx[gid].y_min = -line.p_start.Y();

		if (line.p_end.X() > m_bbx[gid].x_max) m_bbx[gid].x_max = line.p_end.X();
		if (line.p_end.X() < m_bbx[gid].x_min) m_bbx[gid].x_min = line.p_end.X();
		if (-line.p_end.Y() > m_bbx[gid].y_max) m_bbx[gid].y_max = -line.p_end.Y();
		if (-line.p_end.Y() < m_bbx[gid].y_min) m_bbx[gid].y_min = -line.p_end.Y();
	}
	m_tree_center = QPointF((xmax + xmin)*0.5, (ymax + ymin)*0.5);
	m_tree_height = ymax - ymin;
	m_tree_width = xmax - xmin;
	m_tree_maxlen = (m_tree_height > m_tree_width) ? m_tree_height : m_tree_width;
	//qDebug() << tree_center;
	qDebug() << "tree height and width: " << m_tree_height << "  " << m_tree_width;

}

void Scene::load_rules(const QString& filename){
	std::string fs = filename.toStdString();
	const char *descriptor_filename = fs.c_str();

	if (m_random_rules){
		m_lsystem->loadRulesFromFile_random(descriptor_filename, 0);
	}
	else {
		m_lsystem->loadRulesFromFile(descriptor_filename, 0);
	}
	
}

void Scene::draw_rules(){
	string lsystem;
	if (m_random_rules){
		lsystem = m_lsystem->reproduce_random();
	}
	else{
		lsystem = m_lsystem->reproduce();
	}
	
	m_lsystem->draw(lsystem);
	m_tree2d.clear();
	m_tree2d = m_lsystem->resultedTree();
	number_groups = m_lsystem->number_groups();
	for (int i = 0; i < number_groups; i++){
		Bounding_box bbx;
		bbx.group_id = i;
		m_bbx.push_back(bbx);
	}
	//get the information of the boundxing box of the tree
	double xmax = -100000, xmin = 100000, ymax = -100000, ymin = 100000;
	for (int i = 0; i < m_tree2d.size(); i++){
		t_line line = m_tree2d[i];
		if (line.p_start.X() > xmax) xmax = line.p_start.X();
		if (line.p_start.X() < xmin) xmin = line.p_start.X();
		if (-line.p_start.Y() > ymax) ymax = -line.p_start.Y(); //we reverse the y value to make the coordinate system same to the QWidget
		if (-line.p_start.Y() < ymin) ymin = -line.p_start.Y();

		if (line.p_end.X() > xmax) xmax = line.p_end.X();
		if (line.p_end.X() < xmin) xmin = line.p_end.X();
		if (-line.p_end.Y() > ymax) ymax = -line.p_end.Y();
		if (-line.p_end.Y() < ymin) ymin = -line.p_end.Y();

		int gid = line.group_id;
		if (line.p_start.X() > m_bbx[gid].x_max) m_bbx[gid].x_max = line.p_start.X();
		if (line.p_start.X() < m_bbx[gid].x_min) m_bbx[gid].x_min = line.p_start.X();
		if (-line.p_start.Y() > m_bbx[gid].y_max) m_bbx[gid].y_max = -line.p_start.Y(); //we reverse the y value to make the coordinate system same to the QWidget
		if (-line.p_start.Y() < m_bbx[gid].y_min) m_bbx[gid].y_min = -line.p_start.Y();

		if (line.p_end.X() > m_bbx[gid].x_max) m_bbx[gid].x_max = line.p_end.X();
		if (line.p_end.X() < m_bbx[gid].x_min) m_bbx[gid].x_min = line.p_end.X();
		if (-line.p_end.Y() > m_bbx[gid].y_max) m_bbx[gid].y_max = -line.p_end.Y();
		if (-line.p_end.Y() < m_bbx[gid].y_min) m_bbx[gid].y_min = -line.p_end.Y();
	}
	m_tree_center = QPointF((xmax + xmin)*0.5, (ymax + ymin)*0.5);
	m_tree_height = ymax - ymin;
	m_tree_width = xmax - xmin;
	m_tree_maxlen = (m_tree_height > m_tree_width) ? m_tree_height : m_tree_width;
	m_tree_diagonal = std::sqrt(m_tree_height*m_tree_height + m_tree_width*m_tree_width);
	m_tree_scale_factor = (512 / m_tree_maxlen)*0.8;
	//m_tree_scale_factor = 1.0;
	//qDebug() << tree_center;
	qDebug() << "tree height and width, scale factor: " << m_tree_height << "  " << m_tree_width << " " << m_tree_scale_factor;

}

void Scene::draw_rules_random(){
	string lsystem;
	if (m_random_rules){
		m_lsystem->select_rules_random_new();
		lsystem = m_lsystem->reproduce_random_new();

		/*m_lsystem->select_rules_random();
		lsystem = m_lsystem->reproduce_random();*/
	}
	else{
		lsystem = m_lsystem->reproduce();
	}

	m_lsystem->draw(lsystem);
	m_tree2d.clear();
	m_tree2d = m_lsystem->resultedTree();
	number_groups = m_lsystem->number_groups();
	for (int i = 0; i < number_groups; i++){
		Bounding_box bbx;
		bbx.group_id = i;
		m_bbx.push_back(bbx);
	}
	//get the information of the boundxing box of the tree
	double xmax = -100000, xmin = 100000, ymax = -100000, ymin = 100000;
	for (int i = 0; i < m_tree2d.size(); i++){
		t_line line = m_tree2d[i];
		if (line.p_start.X() > xmax) xmax = line.p_start.X();
		if (line.p_start.X() < xmin) xmin = line.p_start.X();
		if (-line.p_start.Y() > ymax) ymax = -line.p_start.Y(); //we reverse the y value to make the coordinate system same to the QWidget
		if (-line.p_start.Y() < ymin) ymin = -line.p_start.Y();

		if (line.p_end.X() > xmax) xmax = line.p_end.X();
		if (line.p_end.X() < xmin) xmin = line.p_end.X();
		if (-line.p_end.Y() > ymax) ymax = -line.p_end.Y();
		if (-line.p_end.Y() < ymin) ymin = -line.p_end.Y();

		int gid = line.group_id;
		if (line.p_start.X() > m_bbx[gid].x_max) m_bbx[gid].x_max = line.p_start.X();
		if (line.p_start.X() < m_bbx[gid].x_min) m_bbx[gid].x_min = line.p_start.X();
		if (-line.p_start.Y() > m_bbx[gid].y_max) m_bbx[gid].y_max = -line.p_start.Y(); //we reverse the y value to make the coordinate system same to the QWidget
		if (-line.p_start.Y() < m_bbx[gid].y_min) m_bbx[gid].y_min = -line.p_start.Y();

		if (line.p_end.X() > m_bbx[gid].x_max) m_bbx[gid].x_max = line.p_end.X();
		if (line.p_end.X() < m_bbx[gid].x_min) m_bbx[gid].x_min = line.p_end.X();
		if (-line.p_end.Y() > m_bbx[gid].y_max) m_bbx[gid].y_max = -line.p_end.Y();
		if (-line.p_end.Y() < m_bbx[gid].y_min) m_bbx[gid].y_min = -line.p_end.Y();
	}
	m_tree_center = QPointF((xmax + xmin)*0.5, (ymax + ymin)*0.5);
	m_tree_height = ymax - ymin;
	m_tree_width = xmax - xmin;
	m_tree_maxlen = (m_tree_height > m_tree_width) ? m_tree_height : m_tree_width;
	m_tree_diagonal = std::sqrt(m_tree_height*m_tree_height + m_tree_width*m_tree_width);
	m_tree_scale_factor = (512 / m_tree_maxlen)*0.9;
	//m_tree_scale_factor = 100.0;
	//qDebug() << tree_center;
	qDebug() << "tree height and width, scale factor, diagonal: " << m_tree_height << "  " << m_tree_width << " " << m_tree_scale_factor << " " << m_tree_diagonal;

}

void Scene::set_random_rules(bool random) {
	m_random_rules = random; 
	m_lsystem->set_random_rule(random);
}

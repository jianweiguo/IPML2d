#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <unordered_map>
#include <unordered_set>
#include <iomanip>
// Qt
#include <QGLWidget>
#include <QPaintEvent>
#include <QtPrintSupport/QPrinter>
#include <QtSvg>

// local
#include "scene.h"
#include "udgcd_cycle_detector.hpp"
#include "SuffixTree.h"
//#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
//#include <boost/graph/prim_minimum_spanning_tree.hpp>
//#include <boost/pending/indirect_cmp.hpp>
//#include <boost/range/irange.hpp>

typedef boost::property<boost::edge_weight_t, double> EdgeWeightProperty;
typedef boost::adjacency_list<boost::listS, boost::vecS, boost::undirectedS, boost::no_property, EdgeWeightProperty> UndirectedGraph;
typedef boost::graph_traits<UndirectedGraph>::edge_iterator edge_iterator;
typedef boost::graph_traits<UndirectedGraph>::vertex_descriptor vertex_t;
typedef boost::graph_traits<UndirectedGraph>::edge_descriptor edge_descriptor;

struct repetition_node{
	int oocur_time;
	int last_groups_numer;
	int group_node_size;
	std::vector<TreeNode *> parent_node;
	//std::unordered_set<TreeNode *> parent_node;
	repetition_node() : oocur_time(0), last_groups_numer(0){}
};

struct Nary_repetition_node{
	int oocur_time;
	int last_groups_numer;
	int group_node_size;
	std::vector<Nary_TreeNode *> parent_node;
	//std::unordered_set<TreeNode *> parent_node;
	Nary_repetition_node() : oocur_time(0), last_groups_numer(0){}
};

/*
Simulated Annealing
*/
struct Repeated_substring{
	string value;
	//int occur_frequency;
	std::vector<int> index;
	bool operator < (const Repeated_substring &s)const {
		return value.length() < s.value.length();
	}
	Repeated_substring(string v, std::vector<int> idx) : value(v), index(idx) {}
	Repeated_substring() : value("") {}
};

/*
Genetic Algorithm
*/
struct chromo_typ //  define a data structure which will define a chromosome
{
	string    bits; //the binary bit string is held in a std::string
	float     fitness;
	chromo_typ() : bits(""), fitness(0.0f){};
	chromo_typ(string bts, float ftns) : bits(bts), fitness(ftns){}
};

class GlViewer : public QGLWidget
{
	Q_OBJECT

private:
	// toggles
	bool m_show_tree;
	bool m_show_tree_render;
	bool m_render;
	int  m_image_id;
	bool m_show_image;
	bool m_showSampledPoints;

	// rendering options
	double m_line_thickness;
	double m_point_size;

	double m_rotate_angle;
	int m_xStep;
	int m_yStep;
	int m_xSteps_all;
	int m_ySteps_all;
	double scale_factor_;

	// camera
	double m_scale;
	double m_center_x;
	double m_center_y;

	// mouse
	QPoint m_mouse_click;
	QPoint m_mouse_move;
	//Point m_mouse_pick;

	QBrush m_background;

	vector<Bounding_box> m_bbx;
	QImage m_parse_img;
	vector<Bounding_box_parse> m_bbx_parse;

	newAssociativeArray m_rules;
	int m_alphabet_pointer;
	std::set<string> m_selected_repetitions;
	std::set<TreeNode*> m_selected_repetitions_nodes;
	std::unordered_map<string, string> m_used_symbols;
	int m_iteration;

	int m_bbxID;

	std::string m_save_image_name;

	// using graph
	UndirectedGraph m_graph;
	double m_weight_thrs;
	double m_centerDis_thrs;
	double m_cornerDis_thrs;
	int m_root_id;
	std::set<Nary_TreeNode*> m_selected_repetitions_nary_nodes;
	double m_mianBranchAngle_thrs;
	double m_min_len;
	int m_tree_node_size_;

	double m_final_grammar_length_;
	int m_tree_node_size_angle_;
	double m_average_branch_angle_;
	//double m_average_scalar_;

	string m_optAlgorithm_;
public:
	Scene* m_scene;

	GlViewer(QWidget *parent);

	void set_scene(Scene* pScene) { m_scene = pScene; }

	void set_camera(const double x, const double y, const double s)
	{
		m_center_x = x;
		m_center_y = y;
		m_scale = s;
	}

	// options
	double& line_thickness() { return m_line_thickness; }
	const double& line_thickness() const { return m_line_thickness; }
	int& set_bbxID() { return m_bbxID; }

	double& set_weight_thrs() { return m_weight_thrs; }
	double& set_centerDis_thrs() { return m_centerDis_thrs; }
	double& set_cornerDis_thrs() { return m_cornerDis_thrs; }
	double& set_mianBranchAngle_thrs() { return m_mianBranchAngle_thrs; }
	string& set_optAlgorithm(){ return m_optAlgorithm_; }

	// toggles
	void set_show_tree_render(bool show) { m_show_tree_render = show; }
	void set_show_tree(bool show) { m_show_tree = show; }
	void set_show_image(bool show) { m_show_image = show; }
	void set_show_sampledPoints(bool show) {m_showSampledPoints = show; }

	/////////////////////////////////Data Generation/////////////////////////////////////////////////////
	void render_images_all_FRCNN();
	void render_images_all_FRCNN_test();
	void render_images_all_FRCNN_random();
	void render_images_all_FRCNN_random_test();
	void render_current_image();
	void render_images_all_RetinaNet();

	/////////////////////////////////Grammar inference/////////////////////////////////////////////////////
	void parsing_image(const QString& filename);
	bool check_bbox_intersect(Bounding_box_parse b1, Bounding_box_parse b2);
	void build_graph();
	void remove_cycles();
	void extract_minimal_spanning_tree();
	double compute_relative_distance(Bounding_box_parse b1, Bounding_box_parse b2);
	void build_NaryTree();
	void Nary_print_tree(Nary_TreeNode *root, int spaces);
	string Nary_write_grammar(Nary_TreeNode *root, bool with_paras = true);
	string Nary_write_grammar_forPaper(Nary_TreeNode *root, bool with_paras = true);
	double Nary_get_scalar_para(Nary_TreeNode *root);
	void Nary_generate_conformal_grammar(Nary_TreeNode *root, unordered_map<string, Nary_repetition_node>& m);
	string Nary_find_repetitions(Nary_TreeNode* node, unordered_map<string, Nary_repetition_node>& m);
	string Nary_select_prefer_repetition(unordered_map<string, Nary_repetition_node>& m, double weight, bool& find);
	ruleSuccessor Nary_write_rules(Nary_TreeNode *root, bool new_tree = false);
	bool Nary_update_cluster_infomation(Nary_TreeNode *node, int cluster_id, string symb);
	void Nary_perform_clustring(Nary_TreeNode *node, bool find = false);
	bool Nary_perform_collaps_leaf(Nary_TreeNode *node, bool find = false);
	bool Nary_perform_move_leaf(Nary_TreeNode *node, bool find);
	void grammar_induction();
	double edit_distance_DP(string str1, string str2);
	bool merge_two_rules(std::vector<ruleProductions> &productions, int ri, int rj, std::unordered_map<string, int> &symbols_info);
	int compute_grammar_length(std::vector<ruleProductions> &productions, std::unordered_map<string, int> &symbols_info);
	void Nary_compute_strahler_number(Nary_TreeNode *root);

	/////////////////////////////////Simulated Annealing/////////////////////////////////////////////////////
	int Nary_generate_compact_grammar_simulatedAnnealing(string left_NonT, string str);
	std::unordered_map<string, std::vector<int>> find_all_repeating_substring(string str);
	void generate_new_rule_SA(string left_NonT, string str, Repeated_substring select_substring, newAssociativeArray& rules);
	double compute_compact_energy(newAssociativeArray rules, double weight);
	void print_rules_SA(newAssociativeArray& rules);

	/////////////////////////////////Genetic Algorithms/////////////////////////////////////////////////////
	int Nary_generate_compact_grammar_geneticAlgorithms(string left_NonT, string str);
	//void    PrintGeneSymbol(int val);
	string  GetRandomBits(int length);
	int     BinToDec(string bits);
	//float   AssignFitness(string bits);
	//void    PrintChromo(string bits);
	//void    PrintGeneSymbol(int val);
	//int     ParseBits(string bits, int* buffer);
	string  Roulette(int total_fitness, chromo_typ* Population);
	void    Mutate(string &bits);
	void    Crossover(string &offspring1, string &offspring2, int chromo_length);

protected:

    // GL
	void paintEvent(QPaintEvent *event);
    void initializeGL();
    void resizeGL(int width, int height);

    // mouse
    void wheelEvent(QWheelEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void move_camera(const QPoint& p0, const QPoint& p1);
};

#endif

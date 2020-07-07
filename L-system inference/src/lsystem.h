#include <iostream>
#include <stack>
#include "R2/R2.h"
#include "R3/R3.h"
#include "turtle.h"
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
using namespace std;
typedef unordered_map<string, vector<string> > AssociativeArray;

struct ruleSuccessor{
	string successor;
	vector<double> paramters;
	ruleSuccessor() :successor(""){
	}
	ruleSuccessor(string s) :successor(s){
	}
};

struct ruleProductions{
	string precessor;
	string successor;
	vector<double> parameters;
	//bool self_recursive;
	ruleProductions() :precessor(""), successor(""){
	}
	ruleProductions(string left, string right) :precessor(left), successor(right){
	}
};

typedef unordered_map<string, vector<ruleSuccessor> > newAssociativeArray;

class LSystem 
{
protected:
	TurtleSystem turtle;
	void replaceAll(string& str, const string& from, const string& to, int iteration);
	string produce(const string axiom, const AssociativeArray rules, int iteration);
	virtual void run(const char command,const float param, const float scalar, const int class_id, const int iter_num, const int strahler_number);
	float defaultCoefficient;

	float numbers_[3];
	string axiom_;
	AssociativeArray rules_;
	
	std::vector<int> iter_num_indicator_;

	//*****for random rules********
	std::vector<ruleProductions> start_rules;
	std::vector<ruleProductions> grow_rules;
	bool use_random_rules_;
	bool use_set_branchAngle_;
	//bool use_random_angle_;
	double random_angle_perturbation_;

	double average_branch_angle_;
	double average_scalar_;
	AssociativeArray actual_used_rules_;

public:

	int iterations_;
	double branchingAngle_;

	LSystem()
		:turtle()
	{
		branchingAngle_ = 30.0;
		iterations_ = 2;
		use_random_rules_ = false;
		use_set_branchAngle_ = true;
		//use_random_angle_ = false;
		random_angle_perturbation_ = 0.0;
	}

	string reproduce(const string axiom,const AssociativeArray rules, const int iterations=1);
	virtual string generateFromFile(const char * filename, const int iterations=0 );
	void draw(const string data);
	vector<t_line> resultedTree(){ return turtle.resultedTree(); }
	int number_groups() { return turtle.number_groups(); }

	//*****our newly added*********
	void set_branchingAngle(double angle) { branchingAngle_ = angle; }
	void set_iterations(int iter) { iterations_ = iter; }
	virtual void loadRulesFromFile(const char * filename, const int iterations = 0);
	string reproduce(); 
	void clear();

	void use_set_branchAngle(bool show) { use_set_branchAngle_ = show; }
	//void use_random_angle(bool random) { use_random_angle_ = random; }
	void set_angle_perturbation(double angle) { random_angle_perturbation_ = angle; }

	double get_average_branch_angle() { return average_branch_angle_; }
	double get_average_scalar() { return average_scalar_; }
	int get_used_grammar_length();
	AssociativeArray get_used_grammar(){ return actual_used_rules_; }

	//*****for random rules********
	void loadRulesFromFile_random(const char * filename, const int iterationsOverride);
	void select_rules_random();
	string reproduce_random();
	string reproduce_random(const string axiom, const AssociativeArray rules, const int iterations);
	string produce_random(const string axiom, const AssociativeArray rules, int iteration);
	void replaceAll_random(string& str, const string& from, const string& to, int iteration);
	void set_random_rule(bool random);

	void select_rules_random_new();
	string reproduce_random_new();
	string reproduce_random_new(const string axiom, const AssociativeArray rules, const int iterations);
	string produce_random_new(const string axiom, const AssociativeArray rules, int iteration);
	void replaceAll_random_new(string& str, const string& from, std::vector<string> to_all, int iteration);

};
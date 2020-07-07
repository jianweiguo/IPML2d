#include "lsystem.h"
#include <fstream>
#include <algorithm>
#include <ctime>
using namespace std;

void LSystem::replaceAll(string& str, const string& from, const string& to, int iteration)
{
	if(from.empty())
		return;
	size_t start_pos = 0;

	//while((start_pos = str.find(from, start_pos)) != string::npos) 
	//{
	//	str.replace(start_pos, from.length(), to);
 //   	start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
 //   }

	while ((start_pos = str.find(from, start_pos)) != string::npos)
	{
		if (iteration == 0){
			//str = "{" + str;
			str.insert(start_pos,"{");
			start_pos += 1;
		}
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
		if (iteration == 0){
			//str += "}";
			str.insert(start_pos, "}");
		}
	}
}

string LSystem::produce(const string axiom, const AssociativeArray rules, int iteration)
{
	string t=axiom;
	AssociativeArray::const_iterator iter;
	for (iter=rules.begin(); iter!=rules.end();++iter)
	{
		string key=iter->first;
		vector<string> value=iter->second;
		int index=rand()%value.size();
		// printf("Selected %d out of %d : %s\n",index,value.size(),value[index].c_str());
		replaceAll(t, key, value[index], iteration);
	}
	return t;
}

string LSystem::reproduce(const string axiom,const AssociativeArray rules, const int iterations)
{
	//for (int i = 0; i < axiom.size(); i++) {
	//	if(axiom[i] == 'F' || axiom[i] == 'f')
	//	iter_num_indicator_.push_back();
	//}
	
	if (iterations>0)
		return reproduce(produce(axiom, rules, iterations-1), rules, iterations - 1);
	return axiom;
}
string LSystem::generateFromFile(const char * filename,const int iterationsOverride )
{
	cout <<"Generating L-System data..."<<endl;
	ifstream file(filename);
	if (!file)
	{
		cout <<"Could not open L file "<<filename<<endl;
		return "";
	}
	int numbersRead=0;
	float numbers[3];
	string axiom;
	AssociativeArray rules;
	while (!file.eof())
	{
		string temp;
		char c;
		file>>c;
		if (c=='#') 
		{
			getline(file,temp);
			continue;
		}
		file.putback(c);
		if (numbersRead<3)
		{
			file >>numbers[numbersRead++];
			continue;
		}
		if (c=='@') break;
		getline(file,temp);
		int equalSignPos=temp.find("=");
		if (equalSignPos==string::npos)
		{
			axiom=temp;
		}
		else
		{

			rules[temp.substr(0,equalSignPos)].push_back(temp.substr(equalSignPos+1));
			cout <<temp.substr(0,equalSignPos)<<"  "<<temp.substr(equalSignPos+1)<<endl;
		}
	}

	int iterations=(int)numbers[0];
	if (iterationsOverride)
		iterations=iterationsOverride;
	defaultCoefficient=numbers[1];
	float thickness=numbers[2];
	turtle.thickness=thickness/100;
	turtle.clear();
	return reproduce(axiom,rules,iterations);

}

void LSystem::run(const char command, const float param, const float scalar, const int class_id, const int iter_num, const int strahler_number)
{
	float co=defaultCoefficient;
	float num=param;
	if (num==1)
		num*=co;
	switch (command)
	{
		case '+':
			turtle.turnLeft(num);
			//turtle.set_branch_status(1);
			if (num < 16) {
				turtle.set_branch_status(0);
			}
			else {
				turtle.set_branch_status(1);
			}
			break;
		case '-':
			turtle.turnRight(num);
			//turtle.set_branch_status(2);
			if (num < 16) {
				turtle.set_branch_status(0);
			}
			else {
				turtle.set_branch_status(2);
			}
			break;
		case '*':
			turtle.setScale(scalar);
			break;
		case '%':
			turtle.setThickness(scalar);
			break;
		case 'b':
			turtle.setCalssID(class_id);
			break;
		case '/':
			turtle.add_iter_num(iter_num);
			break;
		case '&':
			turtle.set_strahler_number(strahler_number);
			break;
		/*case '&':
			turtle.pitchDown(num);
			break;
		case '^':
			turtle.pitchUp(num);
			break;*/
		//case '<': //increase diameter
		//case '\\':
		//	turtle.rollLeft(num);
		//	break;
		//case '/':
		//case '>':
		//	turtle.rollRight(num);
		//	break;
		case '|':
			turtle.turn180(param);
			break;
		case 'F':
		case 'f':
			turtle.draw(param);
			turtle.set_branch_status(0);
		// case 'G':
		case 'g':
			turtle.move(param);
			break;
		case '[':
			turtle.save();
			break;
		case ']':
			turtle.restore();
			break;
		case '{':
			if (use_random_rules_){
				////turtle.save();
			}
			//turtle.add_iter_num(1);
			break;
		case '}':
			if (use_random_rules_){
				turtle.add_group_id();
			}
			//turtle.add_iter_num(-1);
			break;
		case '<':
			turtle.save_scalar();
			turtle.save_thickness();
			turtle.save_iter_num();
			break;
		case '>':
			turtle.restore_scalar();
			turtle.restore_thickness();
			turtle.restore_iter_num();
			break;
		default:
		;
	}
}
void LSystem::draw(const string tree)
{
	char paramBuf[1024];
	int bufIndex=0;
	string data=tree;
	//std::cout << data << std::endl;
	float param = 0, scalar = 1.0, thickness=1.0;
	bool getParam=false,checkParam=false;
	bool scalar_set = false;
	bool classe_set = false;
	bool thickness_set = false;
	bool iter_num_set = false;
	bool strahler_number_set = false;

	char command;
	int class_id = 0;
	int iter_num_add = 1;
	int strahler_number = 1;

	average_branch_angle_ = 0;
	average_scalar_ = 0;
	int count_angle = 0;
	int count_scalar = 0;

	for (int i=0;i<data.size();++i)
	{
		char c=data[i];
		if (getParam)
		{
			if (c==')')
			{
				paramBuf[bufIndex]=0;
				bufIndex=0;
				//param=atof(paramBuf);
				if (scalar_set){
					scalar = atof(paramBuf);
					run(command, 1.0, scalar, 0, 0, 1);

					average_scalar_ += scalar;
					count_scalar++;

					scalar_set = false;
				}
				else if (thickness_set){
					thickness = atof(paramBuf);
					run(command, 1.0, thickness, 0, 0, 1);
					thickness_set = false;
				}
				else if (classe_set){
					class_id = atoi(paramBuf);
					run(command, 1.0, 1.0, class_id, 0, 1);
					classe_set = false;
				}
				else if (iter_num_set) {
					iter_num_add = atoi(paramBuf);
					run(command, 1.0, 1.0, 0, iter_num_add, 1);
					iter_num_set = false;
				}
				else if (strahler_number_set) {
					strahler_number = atoi(paramBuf);
					run(command, 1.0, 1.0, 0, 0, strahler_number);
					strahler_number_set = false;
				}
				else{
					if (use_set_branchAngle_){
						param = branchingAngle_;
					}
					else{
						param = atof(paramBuf);
					}
					if (random_angle_perturbation_ > 0.02){
						param = random_number(param, random_angle_perturbation_);
					}
					average_branch_angle_ += param;
					count_angle++;

					run(command, param, 1.0, 0, 0, 1);
				}
				
				getParam=false;
			}
			else
				paramBuf[bufIndex++]=c;
			continue;
		}
		if (checkParam)
		{
			checkParam=false;
			if (c=='(')
			{
				param=0;	
				getParam=true;
				continue;
			}
			run(command,1,1,0, 0, 1);

		}
		if (c == '*'){
			scalar_set = true;
		}
		if (c == '%'){
			thickness_set = true;
		}
		if (c == 'b'){
			classe_set = true;
		}
		if (c =='/'){
			iter_num_set = true;
		}
		if (c == '&') {
			strahler_number_set = true;
		}
		command=c;
		checkParam=true;
	}
	if (checkParam)
		run(command,1,1,0,0, 1);

	cout <<data<<endl;

	average_branch_angle_ /= count_angle;
	average_scalar_ /= count_scalar;
}

//****************************************************************
void LSystem::clear(){
	axiom_ = "";
	rules_.clear();
}
void LSystem::loadRulesFromFile(const char * filename, const int iterationsOverride)
{
	//clear();
	rules_.clear();
	actual_used_rules_.clear();

	cout << "Loading L-System data..." << endl;
	ifstream file(filename);
	if (!file)
	{
		cout << "Could not open L file " << filename << endl;
		return;
	}
	int numbersRead = 0;
	while (!file.eof())
	{
		string temp;
		char c;
		file >> c;
		if (c == '#')
		{
			getline(file, temp);
			continue;
		}
		file.putback(c);
		if (numbersRead<3)
		{
			file >> numbers_[numbersRead++];
			continue;
		}
		if (c == '@') break;
		getline(file, temp);
		int equalSignPos = temp.find("=");
		if (equalSignPos == string::npos)
		{
			axiom_ = temp;
		}
		else
		{
			rules_[temp.substr(0, equalSignPos)].push_back(temp.substr(equalSignPos + 1));
			cout << temp.substr(0, equalSignPos) << "  " << temp.substr(equalSignPos + 1) << endl;

			if (temp.at(equalSignPos + 1) != '<'){
				actual_used_rules_[temp.substr(0, equalSignPos)].push_back(temp.substr(equalSignPos + 1));
			}
		}
	}

	int iterations = (int)numbers_[0];
	if (iterationsOverride)
		iterations = iterationsOverride;
	defaultCoefficient = numbers_[1];
	float thickness = numbers_[2];
	turtle.thickness = thickness / 100;
	turtle.clear();
	//return reproduce(axiom_, rules_, iterations);
}

void LSystem::set_random_rule(bool random) {
	use_random_rules_ = random;
	turtle.set_random_rule(random);
}

string LSystem::reproduce()
{
	turtle.clear();
	string outs = axiom_;
	/*if (iterations_>0)
		return reproduce(produce(outs, rules_, iterations_ - 1), rules_, iterations_ - 1);
	return outs;*/

	return reproduce(outs, rules_, iterations_);
}

//**********************for random rules and parameters******************************************

void LSystem::loadRulesFromFile_random(const char * filename, const int iterationsOverride)
{
	//clear();
	start_rules.clear();
	grow_rules.clear();

	cout << "Loading L-System rules..." << endl;
	ifstream file(filename);
	if (!file)
	{
		cout << "Could not open L file " << filename << endl;
		return;
	}
	int numbersRead = 0;
	while (!file.eof())
	{
		string temp;
		char c;
		file >> c;
		if (c == '#')
		{
			getline(file, temp);
			continue;
		}
		file.putback(c);
		if (numbersRead<3)
		{
			file >> numbers_[numbersRead++];
			continue;
		}
		if (c == '@') break;
		getline(file, temp);
		int equalSignPos = temp.find("=");
		if (equalSignPos == string::npos)
		{
			axiom_ = temp;
			cout << "Axiom: " << axiom_ << endl;
		}
		else
		{
			string left = temp.substr(0, equalSignPos);
			string right = temp.substr(equalSignPos + 1);
			ruleProductions rule(left, right);
			if (left == "X"){
				start_rules.push_back(rule);
			}
			if (left == "F" || left == "A"){
				grow_rules.push_back(rule);
			}
			cout << temp.substr(0, equalSignPos) << "  " << temp.substr(equalSignPos + 1) << endl;
		}
	}

	int iterations = (int)numbers_[0];
	if (iterationsOverride)
		iterations = iterationsOverride;
	defaultCoefficient = numbers_[1];
	float thickness = numbers_[2];
	turtle.thickness = thickness / 100;
	turtle.clear();
	//return reproduce(axiom_, rules_, iterations);
}

void LSystem::select_rules_random()
{
	std::srand(unsigned(std::time(0)));

	int start_rule_num = start_rules.size();
	int grow_rule_num = grow_rules.size();

	rules_.clear();
	rules_[grow_rules[grow_rule_num - 1].precessor].push_back(grow_rules[grow_rule_num - 1].successor); // we first put "F(s)=F(s*R)"

	vector<int> temp, temp2;
	for (int i = 0; i < grow_rule_num - 1; ++i)
	{
		temp2.push_back(i);
	}
	random_shuffle(temp2.begin(), temp2.end());
	for (int i = 0; i < std::min(3, grow_rule_num - 1); i++){
		//int random2 = temp2[i];
		int random2 = rand() % (grow_rule_num - 1);
		rules_[grow_rules[random2].precessor].push_back(grow_rules[random2].successor); // we select two rules "F=...."
		//cout << grow_rules[random2].precessor << "  " << grow_rules[random2].successor << endl;
	}

	//
	for (int i = 0; i < start_rule_num; ++i)
	{
		temp.push_back(i);
	}
	random_shuffle(temp.begin(), temp.end());
	
	//int random1 = rand() % start_rule_num;
	//rules_[start_rules[random1].precessor].push_back(start_rules[random1].successor); // we select one rules "X=...."
	
	for (int i = 0; i < 3; i++){
		//int random1 = temp2[i];
		int random1 = rand() % start_rule_num;
		rules_[start_rules[random1].precessor].push_back(start_rules[random1].successor); // we select two rules "F=...."
	}

	//cout << start_rules[random1].precessor << "  " << start_rules[random1].successor << endl;
}


string LSystem::reproduce_random()
{
	turtle.clear();

	string outs = axiom_; // axiom is "X"
	//return reproduce(produce(outs, rules_, iterations_ - 1), rules_, iterations_ - 1);
	return reproduce_random(outs, rules_, iterations_);

	return outs;
}

string LSystem::reproduce_random(const string axiom, const AssociativeArray rules, const int iterations)
{
	if (iterations>0)
		return reproduce_random(produce_random(axiom, rules, iterations - 1), rules, iterations - 1);
	return axiom;
}

string LSystem::produce_random(const string axiom, const AssociativeArray rules, int iteration)
{
	string t = axiom;

	/*vector<int> temp2;
	int grow_rule_num = grow_rules.size();
	for (int i = 0; i < grow_rule_num - 1; ++i)
	{
		temp2.push_back(i+1);
	}
	random_shuffle(temp2.begin(), temp2.end());

	AssociativeArray::const_iterator iter;
	int count = 0;
	for (iter = rules.begin(); iter != rules.end(); ++iter)
	{
		if (count == temp2[0] || count == temp2[1]){
			string key = iter->first;
			vector<string> value = iter->second;
			int index = rand() % value.size();
			replaceAll(t, key, value[index], iteration);
		}
		count++;
	}*/

	AssociativeArray::const_iterator iter;
	int rule_size = rules.size();
	int count = 0;
	for (iter = rules.begin(); iter != rules.end(); ++iter)
	{
		string key = iter->first;
		vector<string> value = iter->second;

		if (key=="X"){
			//replaceAll_random(t, key, value[0], iteration);

			//int index = rand() % (value.size() - 1);
			int index = rand() % value.size();
			replaceAll_random(t, key, value[index], iteration);
		}
		else if (rule_size == 2){
			replaceAll_random(t, key, value[0], iteration);
			int index = rand() % (value.size()-1);
			replaceAll_random(t, key, value[index + 1], iteration);
		}
		else if (rule_size == 3 && key == "A"){
			int index = rand() % value.size();
			replaceAll_random(t, key, value[index], iteration);
		}
		else if (rule_size == 3 && key == "F"){
			replaceAll_random(t, key, value[0], iteration);
		}
		else{
			int index = rand() % value.size();
			replaceAll_random(t, key, value[index], iteration);
		}
		/*for (int i = 0; i < value.size(); i++){
			replaceAll(t, key, value[i], iteration);
		}*/

		count++;
	}
	return t;
}

void LSystem::replaceAll_random(string& str, const string& from, const string& to, int iteration)
{
	if (from.empty())
		return;
	size_t start_pos = 0;

	//while((start_pos = str.find(from, start_pos)) != string::npos) 
	//{
	//	str.replace(start_pos, from.length(), to);
	//   	start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	//   }

	while ((start_pos = str.find(from, start_pos)) != string::npos)
	{
		if (from == "A"){
			//str = "{" + str;
			str.insert(start_pos, "{");
			start_pos += 1;
			str.replace(start_pos, from.length(), to);
			start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
			str.insert(start_pos, "}");
		}
		else{
			str.replace(start_pos, from.length(), to);
			start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
		}

		//if (iteration == 0){
		//	//str = "{" + str;
		//	str.insert(start_pos, "{");
		//	start_pos += 1;
		//}
		//str.replace(start_pos, from.length(), to);
		//start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
		//if (iteration == 0){
		//	//str += "}";
		//	str.insert(start_pos, "}");
		//}
	}
}


void LSystem::select_rules_random_new()
{
	std::srand(unsigned(std::time(0)));

	int start_rule_num = start_rules.size();
	int grow_rule_num = grow_rules.size();

	rules_.clear();
	rules_[grow_rules[grow_rule_num - 1].precessor].push_back(grow_rules[grow_rule_num - 1].successor); // we first put "F(s)=F(s*R)"

	actual_used_rules_.clear();

	vector<int> temp, temp2;
	for (int i = 0; i < grow_rule_num - 1; ++i)
	{
		temp2.push_back(i);
	}
	random_shuffle(temp2.begin(), temp2.end());
	for (int i = 0; i < std::min(3, grow_rule_num - 1); i++){
		//int random2 = temp2[i];
		int random2 = rand() % (grow_rule_num - 1);
		rules_[grow_rules[random2].precessor].push_back(grow_rules[i].successor); // we select two rules "F=...."
	}

	//
	for (int i = 0; i < start_rule_num; ++i)
	{
		temp.push_back(i);
	}
	random_shuffle(temp.begin(), temp.end());

	//int random1 = rand() % start_rule_num;
	//rules_[start_rules[random1].precessor].push_back(start_rules[random1].successor); // we select one rules "X=...."

	for (int i = 0; i < 1; i++){
		//int random1 = temp2[i];
		int random1 = rand() % start_rule_num;
		rules_[start_rules[random1].precessor].push_back(start_rules[random1].successor);
	}

	//cout << start_rules[random1].precessor << "  " << start_rules[random1].successor << endl;
}

string LSystem::reproduce_random_new()
{
	turtle.clear();

	string outs = axiom_; // axiom is "X"
	//return reproduce(produce(outs, rules_, iterations_ - 1), rules_, iterations_ - 1);
	return reproduce_random_new(outs, rules_, iterations_);

	return outs;
}

string LSystem::reproduce_random_new(const string axiom, const AssociativeArray rules, const int iterations)
{
	if (iterations>0)
		return reproduce_random_new(produce_random_new(axiom, rules, iterations - 1), rules, iterations - 1);
	return axiom;
}

string LSystem::produce_random_new(const string axiom, const AssociativeArray rules, int iteration)
{
	string t = axiom;

	AssociativeArray::const_iterator iter;
	int rule_size = rules.size();
	int count = 0;
	for (iter = rules.begin(); iter != rules.end(); ++iter)
	{
		string key = iter->first;
		vector<string> value = iter->second;

		if (key == "X"){
			//replaceAll_random(t, key, value[0], iteration);
			//int index = rand() % (value.size() - 1);
			//int index = rand() % value.size();
			replaceAll_random_new(t, key, value, iteration);
		}
		else if (key == "A" || key == "B"){
			//int index = rand() % value.size();
			replaceAll_random_new(t, key, value, iteration);
		}
		else if (key == "F"){
			replaceAll_random(t, key, value[0], iteration);
		}
		else{
			//int index = rand() % value.size();
			replaceAll_random_new(t, key, value, iteration);
		}
		/*for (int i = 0; i < value.size(); i++){
		replaceAll(t, key, value[i], iteration);
		}*/

		count++;
	}
	return t;
}

void LSystem::replaceAll_random_new(string& str, const string& from, std::vector<string> to_all, int iteration)
{
	if (from.empty())
		return;
	size_t start_pos = 0;

	while ((start_pos = str.find(from, start_pos)) != string::npos)
	{
		int index = rand() % to_all.size();

		if (from == "A" || from == "B"){
			//str = "{" + str;
			str.insert(start_pos, "{");
			start_pos += 1;
			str.replace(start_pos, from.length(), to_all[index]);
			start_pos += to_all[index].length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
			str.insert(start_pos, "}");
		}
		else{
			str.replace(start_pos, from.length(), to_all[index]);
			start_pos += to_all[index].length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
		}

		if (from == "X"){
			std::vector<string> out = actual_used_rules_[from];
			std::vector<string>::iterator it = std::find(out.begin(), out.end(), to_all[index]);
			if (it == out.end()){
				actual_used_rules_[from].push_back(to_all[index]);
			}
		}
	}
}

int LSystem::get_used_grammar_length(){
	if (actual_used_rules_.size() ==0) return 0;
}
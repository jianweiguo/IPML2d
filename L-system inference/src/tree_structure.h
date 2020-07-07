/**
**/

#ifndef DEG2RAD
#define DEG2RAD(x) ((x)*M_PI/180.)
#endif // DEG2RAD

#ifndef RAD2DEG
#define RAD2DEG(x) ((x)*180./M_PI)
#endif // RAD2DEG

#ifndef _TREE_STRUCT_H_
#define _TREE_STRUCT_H_

//== INCLUDES =================================================================

#include <string>
#include <vector>
#include <stack>
#include <queue>
#include "R2/R2.h"

struct Bounding_box_parse{
	R2Vector center_position, l_t_corner, r_t_corner, r_b_corner, l_b_corner, direction, direction_opp; //Coordinates in images
	R2Vector center_position_LC, l_t_corner_LC, r_t_corner_LC, r_b_corner_LC, l_b_corner_LC, direction_LC, direction_LC_opp; //Coordinates in the L-system
	double width, height;
	double angleFromY, angleFromY_LC;
	double angleFromY_opp, angleFromY_LC_opp;
	double angleFromParent;
	int group_id;
	int label_id;
	
	Bounding_box_parse()
	{
		group_id = -1;
		angleFromY = 0;
		label_id = 0;
	}
	Bounding_box_parse(int id) : group_id(id)
	{
		angleFromY = 0;
		label_id = 0;
	}
};

//Binary Tree
struct TreeNode
{
	TreeNode *left;
	TreeNode *middle;
	TreeNode *right;
	TreeNode *parent;
	Bounding_box_parse bbx;
	std::vector<int> intersect_nodes;
	int bbx_index;
	//int child_num;  // the number of its childrens
	bool main_branch;
	int type_id, cluster_id, cluster_size, cluster_level; //
	string turn_indicator; //"0" for main branch, "1" means turn left, "-1" means turn right
	string alphabet_symbol;
	bool old_repetition;

	TreeNode() : left(NULL), right(NULL), parent(NULL){
		bbx_index = -1;
		main_branch = false;
		type_id = -1;
		cluster_id = -1;
		cluster_size = 0;
		cluster_level = 0;
		turn_indicator = "";
		alphabet_symbol = "F";
		old_repetition = false;
	}

	TreeNode(Bounding_box_parse bb) : bbx(bb), left(NULL), right(NULL), parent(NULL) {
		bbx_index = -1;
		main_branch = false;
		type_id = -1;
		cluster_id = -1;
		cluster_size = 0;
		cluster_level = 0;
		turn_indicator = "";
		alphabet_symbol = "F";
		old_repetition = false;
	}
};

static inline TreeNode *CreateTreeNode(Bounding_box_parse bbx);
static inline void ConnectTreeNodes(TreeNode *pParent, TreeNode *pLeft, TreeNode *pRight);
static inline void ConnectTreeNodes(TreeNode *pParent, TreeNode *pLeft, TreeNode *pMiddle, TreeNode *pRight);
static inline void DestroyTree(TreeNode *pRoot);
static inline vector<vector<TreeNode *> > levelOrderNodeBottom(TreeNode *root);
//static inline vector<TreeNode *> getLeafNodes(TreeNode *root);

//Create new node
static inline TreeNode *CreateTreeNode(Bounding_box_parse bbx)
{
	TreeNode *pNode = new TreeNode(bbx);
	return pNode;
}

//connnect the parent and children nodes
static inline void ConnectTreeNodes(TreeNode *pParent, TreeNode *pLeft, TreeNode *pRight)
{
	if (pParent != NULL)
	{
		pParent->left = pLeft;
		pParent->right = pRight;
		if (pParent->left != NULL){
			pParent->left->parent = pParent;
		}
		if (pParent->right != NULL){
			pParent->right->parent = pParent;
		}
	}
}

//connnect the parent and children nodes
static inline void ConnectTreeNodes(TreeNode *pParent, TreeNode *pLeft, TreeNode *pMiddle, TreeNode *pRight)
{
	if (pParent != NULL)
	{
		pParent->left = pLeft;
		pParent->right = pRight;
		pParent->middle = pMiddle;
		if (pParent->left != NULL){
			pParent->left->parent = pParent;
		}
		if (pParent->middle != NULL){
			pParent->middle->parent = pParent;
		}
		if (pParent->right != NULL){
			pParent->right->parent = pParent;
		}
	}
}

static inline void DestroyTree(TreeNode *pRoot)
{
	if (pRoot != NULL)
	{
		TreeNode *pLeft = pRoot->left;
		TreeNode *pRight = pRoot->right;
		//TreeNode *pMiddle = pRoot->middle;

		delete pRoot;
		pRoot = NULL;

		DestroyTree(pLeft);
		//DestroyTree(pMiddle);
		DestroyTree(pRight);
	}
}

static inline vector<vector<TreeNode*> > levelOrderNodeBottom(TreeNode *root)
{
	vector<vector<TreeNode*> > matrix;
	if (root == NULL)
	{
		return matrix;
	}

	stack<vector<TreeNode*> > sv;
	vector<TreeNode*> temp;
	temp.push_back(root);
	sv.push(temp);

	vector<TreeNode *> path;
	path.push_back(root);

	int count = 1;
	while (!path.empty())
	{
		if (path[0]->left != NULL)
		{
			path.push_back(path[0]->left);
		}
		if (path[0]->right != NULL)
		{
			path.push_back(path[0]->right);
		}
		path.erase(path.begin());
		count--;
		if (count == 0)
		{
			/* vector<TreeNode*> tmp;
			vector<TreeNode *>::iterator it = path.begin();
			for(; it != path.end(); ++it)
			{
			tmp.push_back((*it));
			}*/
			sv.push(path);
			count = path.size();
		}
	}
	while (!sv.empty())
	{
		if (sv.top().size() > 0)
		{
			matrix.push_back(sv.top());
		}
		sv.pop();
	}
	return matrix;
}


//n-ary tree
struct Nary_TreeNode
{
	std::vector<Nary_TreeNode*> children;
	Nary_TreeNode *parent;
	Bounding_box_parse bbx;
	//std::vector<int> intersect_nodes;
	int bbx_index;
	//int child_num;  // the number of its childrens
	bool main_branch;
	int type_id, cluster_id, cluster_size, cluster_level; //
	string turn_indicator; //"0" for main branch, "1" means turn left, "-1" means turn right
	string alphabet_symbol;
	bool old_repetition;
	int strahler_number; 

	Nary_TreeNode() : parent(NULL){
		bbx_index = -1;
		main_branch = false;
		type_id = -1;
		cluster_id = -1;
		cluster_size = 0;
		cluster_level = 0;
		turn_indicator = "";
		alphabet_symbol = "F";
		old_repetition = false;
		strahler_number = 0;
	}

	Nary_TreeNode(Bounding_box_parse bb) : bbx(bb), parent(NULL) {
		bbx_index = -1;
		main_branch = false;
		type_id = -1;
		cluster_id = -1;
		cluster_size = 0;
		cluster_level = 0;
		turn_indicator = "";
		alphabet_symbol = "F";
		old_repetition = false;
		strahler_number = 0;
	}
};

static inline Nary_TreeNode *CreateNaryTreeNode(Bounding_box_parse bbx);
static inline void DestroyTree(Nary_TreeNode *pRoot);

static inline Nary_TreeNode *CreateNaryTreeNode(Bounding_box_parse bbx)
{
	Nary_TreeNode *pNode = new Nary_TreeNode(bbx);
	return pNode;
}

static inline void DestroyTree(Nary_TreeNode *pRoot){
	if (pRoot != NULL)
	{
		std::vector<Nary_TreeNode*> children_copy;
		for (int i = 0; i < pRoot->children.size(); i++){
			children_copy.push_back(pRoot->children[i]);
		}

		delete pRoot;
		pRoot = NULL;

		for (int i = 0; i < children_copy.size(); i++){
			DestroyTree(children_copy[i]);
		}
	}
}

//connnect the parent and children nodes
static inline void ConnectTreeNodes(Nary_TreeNode *pParent, Nary_TreeNode *pChild)
{
	if (pParent != NULL)
	{
		std::vector<Nary_TreeNode*> children;

		pParent->children.push_back(pChild);
		if (pParent->children[pParent->children.size()-1] != NULL){
			pParent->children[pParent->children.size() - 1]->parent = pParent;
		}
	}
}

#endif /*_TREE_STRUCT_H_*/
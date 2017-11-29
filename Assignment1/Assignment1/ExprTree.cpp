#include "ExprTree.h"
#include <sstream>

/*
* Helper function that tests whether a string is a non-negative integer.
*/
bool isdigit(const char & c) {
	switch (c) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9': return true;
	}
	return false;
}

/*
* Returns true if the string is a number. False otherwise.
*/
bool is_number(const std::string & s)
{
	std::string::const_iterator it = s.begin();
	while (it != s.end() && isdigit(*it)) ++it;
	return !s.empty() && it == s.end();
}

/*
* Helper function that converts a string to an int.
*/
int to_number(const std::string & s) {
	return atoi(s.c_str());
}

/*
* Helper function that creates a TreeNode with the appropriate operator
* when given a string that's "+", "-", "*" or "/". If the string is wrong
* it gives a NoOp value.
*/
TreeNode * createOperatorNode(const string & op) {
	if (op == "+") return new TreeNode(Plus);
	if (op == "-") return new TreeNode(Minus);
	if (op == "*") return new TreeNode(Times);
	if (op == "/") return new TreeNode(Divide);
	return new TreeNode(NoOp);
}

string charToString(const char & c)
{
	std::stringstream ss;
	string s;
	ss << c;
	ss >> s;
	return s;
}

string removeSpaces(string expression) {
	string expressionNoSpaces;
	for (int i = 0; i < expression.size(); i++)
		if (expression[i] != ' ')
			expressionNoSpaces += expression[i];
	return expressionNoSpaces;
}

/*
* Helper function that converts a vector of strings (expression tokens) into a vector of TreeNodes.
*/
vector<TreeNode *> convertToVectorOfNodes(vector<string> tokens) {
	vector<TreeNode *> nodes;
	for (int i = 0; i < tokens.size(); i++)
		nodes.push_back((is_number(tokens[i]) ? new TreeNode(to_number(tokens[i])) : createOperatorNode(tokens[i])));
	return nodes;
}

/*
* Helper function that calculates the size of (how many nodes in) an Expression Tree.
*/
int calculateTreeSize(TreeNode * node) {
	if (node == 0)
		return 0;
	return calculateTreeSize(node->getLeftChild()) + calculateTreeSize(node->getRightChild()) + 1;
}

/*
* Returns a string representing a Binary Expression Tree in postfix notation.
*/
string postOrderTraversalString(ExprTree & tree, TreeNode * node) {
	if (node == 0)
		return "";
	return postOrderTraversalString(tree, node->getLeftChild()) + postOrderTraversalString(tree, node->getRightChild()) + node->toString() + (node == tree.getRoot() ? "" : " ");
}

int precedence(Operator op) {
	if (op == Plus || op == Minus)
		return 1;
	return 2;
}

/*
* Converts a vector of expression tokens in infix notation to a queue of nodes in postfix notation.
*/
queue<TreeNode *> shuntingYard(vector<string> infixTokens) {
	vector<TreeNode *> infixNodes = convertToVectorOfNodes(infixTokens);
	stack<TreeNode *> operators;
	queue<TreeNode *> postfixNodes;
	for (int i = 0; i < infixNodes.size(); i++) {
		if (infixNodes[i]->isValue()) // token is a number
			postfixNodes.push(infixNodes[i]);
		else if (infixTokens[i] == "(")
			operators.push(infixNodes[i]);
		else if (infixTokens[i] == ")") {
			while (!operators.empty() && operators.top()->getOperator() != NoOp) {
				postfixNodes.push(operators.top());
				operators.pop();
			}
			operators.pop();
		}
		else { // token is an operator 
			while (!operators.empty() && operators.top()->isOperator() && 
				precedence(infixNodes[i]->getOperator()) <= precedence(operators.top()->getOperator())) {
				postfixNodes.push(operators.top());
				operators.pop();
			}
			operators.push(infixNodes[i]);
		}
	}
	while (!operators.empty()) {
		postfixNodes.push(operators.top());
		operators.pop();
	}
	return postfixNodes;
}

/*
* Helper function that converts a queue of TreeNodes to a vector of TreeNodes.
*/
vector<TreeNode *> queueToVector(queue<TreeNode *> inputQueue) {
	vector<TreeNode *> outputVector;
	while (!inputQueue.empty()) {
		outputVector.push_back(inputQueue.front());
		inputQueue.pop();
	}
	return outputVector;
}

/*
* Helper function that converts a number to a string.
*/
string to_string(const int & n) {
	std::stringstream stream;
	stream << n;
	return stream.str();
}

/*
* Basic constructor that sets up an empty Expr Tree.
*/
ExprTree::ExprTree() {
	root = 0;
	_size = 0;
}

/*
* Constructor that takes a TreeNode and sets up an ExprTree with that node at the root.
*/
ExprTree::ExprTree(TreeNode * r) {
	root = r;
	_size = calculateTreeSize(r);
}

/*
* Destructor to clean up the tree.
*/
ExprTree::~ExprTree() {}

/*
* This function takes a string representing an arithmetic expression and breaks
* it up into components (number, operators, parentheses).
* It returns the broken up expression as a vector of strings.
*/
vector<string> ExprTree::tokenise(string expression) {
	string charHolder;
	string expressionNoSpaces = removeSpaces(expression);
	vector<string> tokens;
	for (int i = 0; i < expressionNoSpaces.size(); i++)
		if (!isdigit(expressionNoSpaces[i])) {
			if (!charHolder.empty())
				tokens.push_back(charHolder);
			tokens.push_back(charToString(expressionNoSpaces[i]));
			charHolder = "";
		}
		else
			charHolder += expressionNoSpaces[i];
	if (!charHolder.empty())
		tokens.push_back(charHolder);
	return tokens;
}

/*
* This function takes a vector of strings representing an expression (as produced
* by tokenise(string)), and builds an ExprTree representing the same expression.
*/
ExprTree ExprTree::buildTree(vector<string> tokens) {
	vector<TreeNode *> postfixNodes = queueToVector(shuntingYard(tokens));
	vector<TreeNode *> nodesForTree;
	for (int i = 0; i < postfixNodes.size(); i++) {
		if (postfixNodes[i]->isValue())
			nodesForTree.push_back(postfixNodes[i]);
		else {
			TreeNode * operatorNode = postfixNodes[i];
			operatorNode->setRightChild(nodesForTree.back());
			nodesForTree.pop_back();
			operatorNode->setLeftChild(nodesForTree.back());
			nodesForTree.pop_back();
			nodesForTree.push_back(operatorNode);
		}
	}
	return ExprTree(nodesForTree.front());
}

/*
* This function takes a TreeNode and does the maths to calculate
* the value of the expression it represents.
*/
int ExprTree::evaluate(TreeNode * n) {
	if (n->isValue()) // n is a leaf node (has no child nodes)
		return n->getValue(); 
	switch (n->getOperator()) { // n is a branch node (has child nodes)
		case Plus: return evaluate(n->getLeftChild()) + evaluate(n->getRightChild());
		case Minus: return evaluate(n->getLeftChild()) - evaluate(n->getRightChild());
		case Times: return evaluate(n->getLeftChild()) * evaluate(n->getRightChild());
		case Divide: return evaluate(n->getLeftChild()) / evaluate(n->getRightChild());
	}
}

/*
* When called on an ExprTree, this function calculates the value of the
* expression represented by the whole tree.
*/
int ExprTree::evaluateWholeTree() {
	return evaluate(getRoot());
}

/*
* Given an ExprTree t, this function returns a string
* that represents that same expression as the tree in
* prefix notation.
*/
string ExprTree::prefixOrder(const ExprTree & t) {
	int nodeCount = 0;
	string prefixString;
	stack<TreeNode *> nodeStack;
	TreeNode * currentNode = t.root;
	while (currentNode != 0) {
		nodeCount++;
		prefixString += currentNode->toString() + (nodeCount == t._size ? "" : " ");
		if (currentNode->getRightChild() != 0)
			nodeStack.push(currentNode->getRightChild());
		if (currentNode->getLeftChild() != 0)
			nodeStack.push(currentNode->getLeftChild());
		currentNode = (!nodeStack.empty() ? nodeStack.top() : 0);
		nodeStack.pop();
	}
	return prefixString;
}

/*
* Given an ExprTree t, this function returns a string
* that represents that same expression as the tree in
* infix notation.
*/
string ExprTree::infixOrder(const ExprTree & t) {
	int nodeCount = 0;
	string infixString;
	stack<TreeNode *> nodeStack;
	TreeNode * currentNode = t.root;
	while (currentNode != 0 || !nodeStack.empty()) {
		if (currentNode != 0) {
			nodeCount++;
			nodeStack.push(currentNode);
			currentNode = currentNode->getLeftChild();
		}
		else {
			currentNode = nodeStack.top();
			nodeStack.pop();
			infixString += currentNode->toString() + (nodeCount == t._size ? "" : " ");
			currentNode = currentNode->getRightChild();
		}
	}
	return infixString;
}

/*
* Given an ExprTree t, this function returns a string
* that represents that same expression as the tree in
* postfix notation.
*/
string ExprTree::postfixOrder(const ExprTree & t) {
	ExprTree tree = t;
	return postOrderTraversalString(tree, t.root);
}

/*
* Returns the size of the tree. (i.e. the number of nodes in it)
*/
int ExprTree::size() { return _size; }

/*
* Returns true if the tree contains no nodes. False otherwise.
*/
bool ExprTree::isEmpty() { return _size == 0; }

/*
* Returns the root of the tree.
*/
TreeNode * ExprTree::getRoot() { return root; }
﻿#pragma once

#include <fstream>
#include <string>
#include <list>

using namespace std;

class SyntaxNode
{
private:
	string synIdentifier;
	string lexIdentifier;
	string lexValue;
	list<SyntaxNode*> childList;
	void writeLexical(ofstream& output);
	void writeSyntax(ofstream& output);

public:
	SyntaxNode();
	SyntaxNode(string synIdentifier);
	SyntaxNode(string lexIdentifier, string lexValue);
	void setSynIdentifier(string synIdentifier);
	bool isLeaf();
	bool isLexEmpty();
	void addChild(SyntaxNode* child);
	string getSynIdentifier();
	string getLexIdentifier();
	string getLexValue();
	list<SyntaxNode*> getChildList();
	void print(ofstream& output);
};

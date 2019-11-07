﻿#include "parser.h"

ParserAnalyser::ParserAnalyser(list<struct Lexeme>::iterator& iter, list<struct Lexeme>::iterator& iterEnd) {
	this->iter = iter;
	this->iterEnd = iterEnd;
}

void ParserAnalyser::CountIterator(int step) {
	if (step > 0) {
		while (step--) {
			iter++;
		}
	} else {
		while (step++) {
			iter--;
		}
	}
}

void ParserAnalyser::AddSymbol(string define) {
	symbolMap.insert({ iter->value, define });
}

string ParserAnalyser::FindSymbol() {
	return symbolMap.find(iter->value)->second;
}

bool ParserAnalyser::IsThisIdentifier(string identifier) {
	return iter->identifier == identifier;
}

bool ParserAnalyser::IsPlusOrMinu() {
	return IsThisIdentifier(PLUS) || IsThisIdentifier(MINU);
}

bool ParserAnalyser::IsMultOrDiv() {
	return IsThisIdentifier(MULT) || IsThisIdentifier(DIV);
}

void ParserAnalyser::AddChild(SyntaxNode* node) {
	node->addChild(new SyntaxNode(iter->identifier, iter->value));
	CountIterator(+1);
}

void ParserAnalyser::AddCommaChild(SyntaxNode* node) {
	if (IsThisIdentifier(COMMA)) {
		AddChild(node);
	}
}

SyntaxNode* ParserAnalyser::AddSyntaxChild(string syntaxName, SyntaxNode* node) {
	SyntaxNode* subRoot = new SyntaxNode(syntaxName);
	node->addChild(subRoot);
	return subRoot;
}

void ParserAnalyser::AnalyzeInteger(SyntaxNode* node) {
	if (IsPlusOrMinu()) {
		AddChild(node);
	}

	AddChild(AddSyntaxChild(UNSIGNINT, node));		// INTCON
}

void ParserAnalyser::AnalyzeConstDefine(SyntaxNode* node) {
	AddChild(node);	// INTTK or CHARTK
	while (IsThisIdentifier(IDENFR)) {
		AddSymbol(CONST_DEFINE);
		AddChild(node);	// IDENFR
		AddChild(node);	// ASDSIGN
		if (IsThisIdentifier(CHARCON)) {
			AddChild(node);	// CHARCON
		} else {
			AnalyzeInteger(AddSyntaxChild(INTEGER, node));
		}
		AddCommaChild(node);	// COMMA
	}
}

void ParserAnalyser::AnalyzeConstDeclare(SyntaxNode* node) {
	while (IsThisIdentifier(CONSTTK)) {
		AddChild(node);	// CONSTTK
		AnalyzeConstDefine(AddSyntaxChild(CONST_DEFINE, node));
		AddChild(node);	// SEMICN
	}
}

void ParserAnalyser::AnalyzeVariableDefine(SyntaxNode* node) {
	AddChild(node);	// INTTK or CHARTK
	while (IsThisIdentifier(IDENFR)) {
		AddSymbol(VARIABLE_DEFINE);
		AddChild(node);	// IDENFR
		if (IsThisIdentifier(LBRACK)) {
			AddChild(node);	// LBRACK
			AddChild(AddSyntaxChild(UNSIGNINT, node));	// INTCON
			AddChild(node);	// RBRACK
		}
		AddCommaChild(node);	// COMMA
	}
}

bool ParserAnalyser::isVariableDefine() {
	if (IsThisIdentifier(INTTK) || IsThisIdentifier(CHARTK)) {
		CountIterator(+2);
		if (IsThisIdentifier(LPARENT)) {
			CountIterator(-2);
			return false;
		} else {
			CountIterator(-2);
			return true;
		}
	} else {
		return false;
	}
}

void ParserAnalyser::AnalyzeVariableDeclare(SyntaxNode* node) {
	while (isVariableDefine()) {
		AnalyzeVariableDefine(AddSyntaxChild(VARIABLE_DEFINE, node));
		AddChild(node);	// SEMICN
	}
}

void ParserAnalyser::AnalyzeValuePrameterTable(SyntaxNode* node) {
	while (!IsThisIdentifier(RPARENT)) {
		AnalyzeExpression(AddSyntaxChild(EXPRESSION, node));
		while (IsThisIdentifier(COMMA)) {
			AddChild(node);	// COMMA
			AnalyzeExpression(AddSyntaxChild(EXPRESSION, node));
		}
	}
}

void ParserAnalyser::AnalyzeReturnCallSentence(SyntaxNode* node) {
	AddChild(node);	// IDENFR
	AddChild(node);	// LPARENT
	AnalyzeValuePrameterTable(AddSyntaxChild(VALUE_PARAMETER_TABLE, node));
	AddChild(node);	// RPARENT
}

void ParserAnalyser::AnalyzeNoReturnCallSentence(SyntaxNode* node) {
	AnalyzeReturnCallSentence(node);
}

void ParserAnalyser::AnalyzeFactor(SyntaxNode* node) {
	if (IsThisIdentifier(IDENFR)) {
		if (FindSymbol() == RETURN_FUNCTION) {
			AnalyzeReturnCallSentence(AddSyntaxChild(RETURN_CALL_SENTENCE, node));
		} else {
			AddChild(node);	// LPARENT
			if (IsThisIdentifier(LBRACK)) {
				AddChild(node);	// LBRACK
				AnalyzeExpression(AddSyntaxChild(EXPRESSION, node));
				AddChild(node);	// RBRACK
			}
		}
	} else if (IsThisIdentifier(LPARENT)) {
		AddChild(node);	// LPARENT
		AnalyzeExpression(AddSyntaxChild(EXPRESSION, node));
		AddChild(node);	// RPARENT
	} else if (IsThisIdentifier(CHARCON)) {
		AddChild(node);
	} else {
		AnalyzeInteger(AddSyntaxChild(INTEGER, node));
	}
}

void ParserAnalyser::AnalyzeItem(SyntaxNode* node) {
	AnalyzeFactor(AddSyntaxChild(FACTOR, node));
	while (IsMultOrDiv()) {
		AddChild(node);
		AnalyzeFactor(AddSyntaxChild(FACTOR, node));
	}
}

void ParserAnalyser::AnalyzeExpression(SyntaxNode* node) {
	if (IsPlusOrMinu()) {
		AddChild(node);	// PLUS or MINU
	}
	AnalyzeItem(AddSyntaxChild(ITEM, node));
	while (IsPlusOrMinu()) {
		AddChild(node);
		AnalyzeItem(AddSyntaxChild(ITEM, node));
	}
}

void ParserAnalyser::AnalyzeCondition(SyntaxNode* node) {
	AnalyzeExpression(AddSyntaxChild(EXPRESSION, node));
	if (IsThisIdentifier(LSS)
		|| IsThisIdentifier(LEQ)
		|| IsThisIdentifier(GRE)
		|| IsThisIdentifier(GEQ)
		|| IsThisIdentifier(EQL)
		|| IsThisIdentifier(NEQ)) {
		AddChild(node);
		AnalyzeExpression(AddSyntaxChild(EXPRESSION, node));
	}
}

void ParserAnalyser::AnalyzeIfSentence(SyntaxNode* node) {
	AddChild(node);	// IFTK
	AddChild(node);	// LPARENT
	AnalyzeCondition(AddSyntaxChild(CONDITION, node));
	AddChild(node);	// RPARENT
	AnalyzeSentence(AddSyntaxChild(SENTENCE, node));
	if (IsThisIdentifier(ELSETK)) {
		AddChild(node);	// ELSETK
		AnalyzeSentence(AddSyntaxChild(SENTENCE, node));
	}
}

void ParserAnalyser::AnalyzeStep(SyntaxNode* node) {
	AddChild(AddSyntaxChild(UNSIGNINT, node));	// INTCON
}

void ParserAnalyser::AnalyzeLoopSentence(SyntaxNode* node) {
	if (IsThisIdentifier(WHILETK)) {
		AddChild(node);	// WHILETK
		AddChild(node);	// LPARENT
		AnalyzeCondition(AddSyntaxChild(CONDITION, node));
		AddChild(node);	// RPARENT
		AnalyzeSentence(AddSyntaxChild(SENTENCE, node));
	} else if (IsThisIdentifier(DOTK)) {
		AddChild(node);	// DOTK
		AnalyzeSentence(AddSyntaxChild(SENTENCE, node));
		AddChild(node);	// WHILETK
		AddChild(node);	// LPARENT
		AnalyzeCondition(AddSyntaxChild(CONDITION, node));
		AddChild(node);	// RPARENT
	} else if (IsThisIdentifier(FORTK)) {
		AddChild(node);	// FORTK
		AddChild(node);	// LPARENT
		AddChild(node);	// IDENFR
		AddChild(node);	// ASSIGN
		AnalyzeExpression(AddSyntaxChild(EXPRESSION, node));
		AddChild(node);	// SEMICN
		AnalyzeCondition(AddSyntaxChild(CONDITION, node));
		AddChild(node);	// SEMICN
		AddChild(node);	// IDENFR
		AddChild(node);	// ASSIGN
		AddChild(node);	// IDENFR
		AddChild(node);	// PLUS or MINU
		AnalyzeStep(AddSyntaxChild(STEP, node));
		AddChild(node);	// RPARENT
		AnalyzeSentence(AddSyntaxChild(SENTENCE, node));
	}
}

void ParserAnalyser::AnalyzeAssignSentence(SyntaxNode* node) {
	AddChild(node);	// IDENFR
	if (IsThisIdentifier(LBRACK)) {
		AddChild(node);	// LBRACK
		AnalyzeExpression(AddSyntaxChild(EXPRESSION, node));
		AddChild(node);	// RBRACK
	}
	AddChild(node);	// ASSIGN
	AnalyzeExpression(AddSyntaxChild(EXPRESSION, node));
}

void ParserAnalyser::AnalyzeScanfSentence(SyntaxNode* node) {
	AddChild(node);	// SCANFTK
	AddChild(node);	// LPARENT

	AddChild(node);	// IDENFR
	while (IsThisIdentifier(COMMA)) {
		AddChild(node);	// COMMA
		AddChild(node);	// IDENFR
	}

	AddChild(node);	// RPARENT
}

void ParserAnalyser::AnalyzePrintfSentence(SyntaxNode* node) {
	AddChild(node);	// PRINTFTK
	AddChild(node);	// LPARENT

	if (IsThisIdentifier(STRCON)) {
		AddChild(AddSyntaxChild(STRING, node));
		if (IsThisIdentifier(COMMA)) {
			AddChild(node);	// COMMA
			AnalyzeExpression(AddSyntaxChild(EXPRESSION, node));
		}
	} else {
		AnalyzeExpression(AddSyntaxChild(EXPRESSION, node));
	}

	AddChild(node);	// RPARENT
}

void ParserAnalyser::AnalyzeReturnSentence(SyntaxNode* node) {
	AddChild(node);	// RETURNTK
	if (IsThisIdentifier(LPARENT)) {
		AddChild(node);	// LPARENT
		AnalyzeExpression(AddSyntaxChild(EXPRESSION, node));
		AddChild(node);	// RPARENT
	}
}

void ParserAnalyser::AnalyzeSentence(SyntaxNode* node) {
	if (IsThisIdentifier(IFTK)) {
		AnalyzeIfSentence(AddSyntaxChild(IF_SENTENCE, node));
	} else if (IsThisIdentifier(WHILETK)
		|| IsThisIdentifier(DOTK)
		|| IsThisIdentifier(FORTK)) {
		AnalyzeLoopSentence(AddSyntaxChild(LOOP_SENTENCE, node));
	} else if (IsThisIdentifier(LBRACE)) {
		AddChild(node);	// LBRACE
		AnalyzeSentenceCollection(AddSyntaxChild(SENTENCE_COLLECTION, node));
		AddChild(node);	// RBRACE
	} else if (IsThisIdentifier(IDENFR)) {
		if (FindSymbol() == RETURN_FUNCTION) {
			AnalyzeReturnCallSentence(AddSyntaxChild(RETURN_CALL_SENTENCE, node));
			AddChild(node);	// SEMICN
		} else if (FindSymbol() == NO_RETURN_FUNCTION) {
			AnalyzeReturnCallSentence(AddSyntaxChild(NO_RETURN_CALL_SENTENCE, node));
			AddChild(node);	// SEMICN
		} else {
			AnalyzeAssignSentence(AddSyntaxChild(ASSIGN_SENTENCE, node));
			AddChild(node);	// SEMICN
		}
	} else if (IsThisIdentifier(SCANFTK)) {
		AnalyzeScanfSentence(AddSyntaxChild(SCANF_SENTENCE, node));
		AddChild(node);	// SEMICN
	} else if (IsThisIdentifier(PRINTFTK)) {
		AnalyzePrintfSentence(AddSyntaxChild(PRINTF_SENTENCE, node));
		AddChild(node);	// SEMICN
	} else if (IsThisIdentifier(SEMICN)) {
		AddChild(node);	// SEMICN
	} else if (IsThisIdentifier(RETURNTK)) {
		AnalyzeReturnSentence(AddSyntaxChild(RETURN_SENTENCE, node));
		AddChild(node);	// SEMICN
	}
}

void ParserAnalyser::AnalyzeSentenceCollection(SyntaxNode* node) {
	while (!IsThisIdentifier(RBRACE)) {
		AnalyzeSentence(AddSyntaxChild(SENTENCE, node));
	}
}

void ParserAnalyser::AnalyzeCompositeSentence(SyntaxNode* node) {
	if (IsThisIdentifier(CONSTTK)) {
		AnalyzeConstDeclare(AddSyntaxChild(CONST_DECLARE, node));
	}
	if (IsThisIdentifier(INTTK) || IsThisIdentifier(CHARTK)) {
		AnalyzeVariableDeclare(AddSyntaxChild(VARIABLE_DECLARE, node));
	}
	AnalyzeSentenceCollection(AddSyntaxChild(SENTENCE_COLLECTION, node));
}

void ParserAnalyser::AnalyzeMain(SyntaxNode* node) {
	AddChild(node);	// VOIDTK
	AddChild(node);	// MAINTK
	AddChild(node);	// LPARENT
	AddChild(node);	// RPARENT
	AddChild(node);	// LBRACE

	AnalyzeCompositeSentence(AddSyntaxChild(COMPOSITE_SENTENCE, node));

	AddChild(node);	// RBRACE
}

void ParserAnalyser::AnalyzeParameterTable(SyntaxNode* node) {
	while (IsThisIdentifier(INTTK) || IsThisIdentifier(CHARTK)) {
		AddChild(node);	// INTTK or CHARTK
		AddSymbol(VARIABLE_DEFINE);
		AddChild(node);	// IDENTFR
		AddCommaChild(node);
	}
}

void ParserAnalyser::AnalyzeVoidFunc(SyntaxNode* node) {
	AddChild(node);	// VOIDTK
	AddSymbol(NO_RETURN_FUNCTION);
	AddChild(node);	// IDENFR
	AddChild(node);	// LPARENT
	AnalyzeParameterTable(AddSyntaxChild(PARAMETER_TABLE, node));
	AddChild(node);	// RPARENT
	AddChild(node);	// LBRACE
	AnalyzeCompositeSentence(AddSyntaxChild(COMPOSITE_SENTENCE, node));
	AddChild(node);	// RBRACE
}

void ParserAnalyser::AnalyzeHeadState(SyntaxNode* node) {
	AddChild(node);	// INTTK or CHARTK
	AddSymbol(RETURN_FUNCTION);
	AddChild(node);	// IDENFR
}

void ParserAnalyser::AnalyzeFunc(SyntaxNode* node) {
	AnalyzeHeadState(AddSyntaxChild(HEAD_STATE, node));
	AddChild(node);	// LPARENT
	AnalyzeParameterTable(AddSyntaxChild(PARAMETER_TABLE, node));
	AddChild(node);	// RPARENT
	AddChild(node);	// LBRACE
	AnalyzeCompositeSentence(AddSyntaxChild(COMPOSITE_SENTENCE, node));
	AddChild(node);	// RBRACE
}

void ParserAnalyser::BuildSyntaxTree(SyntaxNode* root) {
	string flag = CONST_DECLARE;

	while (iter != iterEnd) {
		if (IsThisIdentifier(CONSTTK)) {
			AnalyzeConstDeclare(AddSyntaxChild(CONST_DECLARE, root));
		} else if (IsThisIdentifier(INTTK) || IsThisIdentifier(CHARTK)) {
			if (flag == CONST_DECLARE) {
				CountIterator(+2);
				if (IsThisIdentifier(LPARENT)) {
					flag = RETURN_FUNCTION;
				} else {
					flag = VARIABLE_DECLARE;
				}
				CountIterator(-2);
			}
			if (flag == RETURN_FUNCTION) {
				AnalyzeFunc(AddSyntaxChild(RETURN_FUNCTION, root));
			} else {
				AnalyzeVariableDeclare(AddSyntaxChild(VARIABLE_DECLARE, root));
				flag = RETURN_FUNCTION;
			}
		} else {
			if (IsThisIdentifier(VOIDTK)) {
				CountIterator(+1);
			}
			if (IsThisIdentifier(MAINTK)) {
				CountIterator(-1);
				AnalyzeMain(AddSyntaxChild(MAIN_FUNCTION, root));
			} else {
				CountIterator(-1);
				AnalyzeVoidFunc(AddSyntaxChild(NO_RETURN_FUNCTION, root));
			}
		}
	}

}

void ParserAnalyser::AnalyzeParse(ofstream& output) {
	SyntaxNode* root = new SyntaxNode(PROGRAM);
	BuildSyntaxTree(root);
	root->print(output);
}

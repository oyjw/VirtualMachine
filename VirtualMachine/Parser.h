#ifndef _PARSER_H_
#define _PARSER_H_
#include "Tokenizer.h"
#include "SymbolTable.h"
#include "Object.h"

#define WORDSIZE 2

class ByteCode;
typedef std::shared_ptr<ByteCode> ByteCodePtr;

class ByteCode{
private:
	ByteCodePtr next;
public:
	ByteCode() = default;
	ByteCode(ByteCodePtr n) :next(n) {}
	std::vector<char> v;
	void push(char code){
		v.push_back(code);
	}
	ByteCodePtr getNext(){
		return next;
	}
};

typedef std::vector<std::pair<size_t,size_t>> LabelList;

struct LoopLabel;
typedef std::shared_ptr<LoopLabel> LoopLabelPtr;

struct LoopLabel{
	size_t start;
	std::vector<size_t> breaks;
	std::vector<size_t> continues;
	LoopLabelPtr next;
	LoopLabel() = default;
	LoopLabel(LoopLabelPtr ptr) :next(ptr) {}
};

class Parser{
public:
	Parser(Tokenizer* t,SymPtr tab):isGlobal(true),byteCodePtr(new ByteCode()),byteCode(&byteCodePtr->v) {
		symTab=tab;
		tokenizer=t;
	}
	~Parser() {
		delete tokenizer;
	}
	ByteCodePtr getByteCodePtr(){
		return byteCodePtr;
	}
	void declarations();
	void decl();
	int declList();
	void term();
	void term2();
	void factor();
	void factor2();
	void basic();
	
	void elem();
	void stmt();
	void stmts();
	void stmtList();
	void returnStmt();
	void printStmt();
	void assignStmt();
	void ifStmt();
	void whileStmt();
	void breakStmt();
	void continueStmt();

	void orExpr(LabelList& orLabel,LabelList& andLabel);
	void orExpr2(LabelList& orLabel,LabelList& andLabel);
	void andExpr(LabelList& orLabel,LabelList& andLabel);
	void andExpr2(LabelList& orLabel,LabelList& andLabel);
	void notExpr(LabelList& orLabel,LabelList& andLabel,bool notFlag);
	void Expr();
	void relaExpr();
	void function();
	void program();
	void functioncall();
private:
	void match(TokenType type);
	void match1(TokenType type);
	void pushWord(int n){
		CodeWord code;
		code.word = (unsigned short)n;
		byteCode->push_back(code.c.c1);
		byteCode->push_back(code.c.c2);
	}
	void setWord(std::vector<char>::size_type pos,int n){
		CodeWord code;
		code.word = (short)n;
		(*byteCode)[pos]=code.c.c1;
		(*byteCode)[pos+1]=code.c.c2;
	}
	void pushFloat(float f){
		CodeFloat code;
		code.f = f;
		byteCode->push_back(code.c.c1);
		byteCode->push_back(code.c.c2);
		byteCode->push_back(code.c.c3);
		byteCode->push_back(code.c.c4);
	}
	ByteCodePtr byteCodePtr;
	std::vector<char> *byteCode;
	Tokenizer *tokenizer;
	SymPtr symTab;
	
	LoopLabelPtr loopLabelPtr;
	bool isGlobal;
};

#endif

#ifndef _PARSER_H_
#define _PARSER_H_
#include "Tokenizer.h"
#include "SymbolTable.h"
#include "Object.h"

#define WORDSIZE 2

class ByteCode;
typedef std::shared_ptr<ByteCode> ByteCodePtr;

typedef std::vector<std::pair<size_t,size_t>> labelList;

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
	void orExpr(labelList& orLabel,labelList& andLabel);
	void orExpr2(labelList& orLabel,labelList& andLabel);
	void andExpr(labelList& orLabel,labelList& andLabel);
	void andExpr2(labelList& orLabel,labelList& andLabel);
	void notExpr(labelList& orLabel,labelList& andLabel,bool notFlag);
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
		code.word = (unsigned short)n;
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
	
	bool isGlobal;
};

#endif

#ifndef _PARSER_H_
#define _PARSER_H_
#include "Tokenizer.h"
#include "SymbolTable.h"
#include "Object.h"

class Parser{
public:
	Parser(Tokenizer* t,SymPtr tab):isGlobal(true) {
		symTab=tab;
		tokenizer=t;
	}
	~Parser() {
		delete tokenizer;
	}
	std::vector<char>& getByteCode(){
		return byteCode;
	}
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
	void ifstmt();
	void orExpr();
	void orExpr2();
	void andExpr();
	void andExpr2();
	void notExpr();
	void Expr();
	void rvalue();
	void function();
	void program();
	void functioncall();
private:
	void match(TokenType type);
	void match1(TokenType type);
	void pushWord(int n){
		CodeWord code;
		code.word = (short)n;
		byteCode.push_back(code.c.c1);
		byteCode.push_back(code.c.c2);
	}
	void setWord(int pos,int n){
		CodeWord code;
		code.word = (short)n;
		byteCode[pos]=code.c.c1;
		byteCode[pos+1]=code.c.c2;
	}
	void pushFloat(float f){
		CodeFloat code;
		code.f = f;
		byteCode.push_back(code.c.c1);
		byteCode.push_back(code.c.c2);
		byteCode.push_back(code.c.c3);
		byteCode.push_back(code.c.c4);
	}
	std::vector<char> byteCode;
	Tokenizer *tokenizer;
	SymPtr symTab;
	bool isGlobal;
};

#endif

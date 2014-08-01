#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include "slang.h"
enum TokenType{
	IDEN, NUM, STRING,FUNCTION, SEMICOLON, PERIOD, NEW, STAR, SLASH, MINUS, PLUS, MOD, ENDOF, ASSIGN, EQ, NOTEQ ,LT,GT, LE ,GE ,LPAREN, 
	RPAREN, LBRACKET, RBRACKET, LBRACE, RBRACE, COMMA, RETURN, FOR, IF, ELSE, NOT, AND, OR, PRINT,VAR ,WHILE,BREAK, CONTINUE, 
	CLASS, LINESTART, INITMETHOD, COLON
};

class Token{
public:
	int line;
	int num;
	std::string str;
	int type;
};


class Tokenizer{
public:
	Tokenizer(const std::string& fileName);
	Token* getToken(int index = 0);
	void advance(int step = 1);
	std::string& getlinestr() { return curLine; }
	int getlinenum() { return line; }
	const std::string& getFileName() {return fileName; }
	void expectedError(int,Token* token);
	void error(const std::string& message, int col = 0, int errorType = SYNTAXERROR);
	bool isAssignStmt;
private:
	void scan();
	
	std::string fileName;
	std::vector<Token> tokenVec;
	int vecPos;
	size_t line;
	size_t num;
	std::string curLine;
	std::ifstream ifs;
	std::unordered_map<std::string, int> reservedWord;
};

#endif
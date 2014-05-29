#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
enum TokenType{
	IDEN, NUM, STRING,FUNCTION, SEMICOLON, COLON, NEW, STAR, SLASH, MINUS, PLUS, ENDOF, ASSIGN, EQ, NOTEQ ,LT,GT, LE ,GE ,LPAREN, 
	RPAREN, LBRACE, RBRACE, COMMA, RETURN, FOR, IF, ELSE, NOT, AND, OR, PRINT,VAR ,WHILE,BREAK, CONTINUE, CLASS, SELF, INITFUNC
};

class Token{
public:
	int line;
	int num;
	std::string str;
	int type;
};

#define SYNTAXERROR 0
#define SYMBOLERROR 1
#define TYPEERROR 2
#define TOKENERROR 3

class Tokenizer{
public:
	Tokenizer(const std::string& fileName);
	Token* getToken(int index = 0);
	void advance(int step = 1);
	std::string& getlinestr() { return curLine; }
	int getlinenum() { return line; }
	const std::string& getFileName() {return fileName; }
	void expectedError(int,Token* token);
	void error(const std::string& message,Token* token = 0,int errorType = 0);
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
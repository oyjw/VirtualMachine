#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
enum TokenType{
	IDEN, NUM, FUNCTION, SEMICOLON, STAR, SLASH, MINUS, PLUS, ENDOF, ASSIGN, EQ, NOTEQ ,LT,GT, LE ,GE ,LPAREN, 
	RPAREN, LBRACE, RBRACE, COMMA, RETURN, FOR, IF, WHILE, ELSE, NOT, AND, OR, PRINT,VAR
};

class Token{
public:
	int line;
	int num;
	std::string str;
	TokenType type;
};


class Tokenizer{
public:
	Tokenizer(const std::string& fileName);
	Token* getToken(int index = 0);
	void advance(int step = 1);
	std::string& getlinestr() { return curLine; }
	int getlinenum() { return line; }
	const std::string& getFileName() {return fileName; }
private:
	void scan();

	std::string fileName;
	std::vector<Token> tokenVec;
	int vecPos;
	int line;
	int num;
	std::string curLine;
	std::ifstream ifs;
	std::unordered_map<std::string, TokenType> reservedWord;
};

#endif
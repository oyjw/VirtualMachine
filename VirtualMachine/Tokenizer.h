#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
enum TokenType{
	IDEN, NUM, FUNCTION, SEMICOLON, STAR, SLASH, MINUS, PLUS, ENDOF, ASSIGN, EQ, LPAREN, 
	RPAREN, LBRACKET, RBRACKET, COMMA, RETURN, FOR, IF, WHILE, ELSE, ASSIGN,NOT,AND,OR
};

class Token{
public:
	int line;
	int num;
	std::string str;
	TokenType type;
	Token* dup(){
		Token* token = new Token(this);
		return token;
	}
};



class Tokenizer{
public:
	Tokenizer();
	Token* getToken(int index = 0);
	void advance(int step = 1);
	std::string& getlinestr() { return curLine; }
	int getlinenum() { return line; }
private:
	void scan();

	std::vector<Token> tokenVec;
	int vecPos;
	std::function<bool(std::string&)> readline;
	int line;
	int num;
	std::string curLine;

	std::unordered_map<std::string, TokenType> reservedWord;
};

#endif
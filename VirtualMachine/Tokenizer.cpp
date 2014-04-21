#include "Tokenizer.h"
#include "Exceptions.h"
#include <string>
#include <vector>
#include <cassert>
#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>

bool readline(std::string& str){
	return std::getline(std::cin, str) ? true : false;
}

bool readfile(const std::string& filename, std::string& str){
	std::ifstream ifs(filename);
	if (!ifs.is_open())
		return false;
	return std::getline(ifs, str) ? true : false;
}

Tokenizer::Tokenizer():vecPos(0),line(1),num(1){
	reservedWord["for"] = FOR;
	reservedWord["if"] = IF;
	reservedWord["while"] = WHILE;
	reservedWord["else"] = ELSE;
}

Token* Tokenizer::getToken(int index){
	if (vecPos+index >= tokenVec.size())
		scan();
	return &tokenVec[vecPos + index];
}

void Tokenizer::scan(){
	tokenVec.clear();
	vecPos = 0;
	num = 1;
	char c;
	Token token;
	bool readchar = true;
	bool pushtoken = false;
	int state = 0;
	std::string input;
	if (!readline(input)) {
		token.type = ENDOF;
		tokenVec.push_back(token);
		return;
	}
	input += '\n';
	curLine = input;
	c = input[num - 1];
	while (1){
		switch (state){
		case 0:{
				   if (isalpha(c) || c == '_'){
					   state = 1;
					   token.str += c;
				   }
				   else if (isdigit(c)){
					   state = 2;
					   token.str += c;
				   }
				   else if (c == '+' || c == '-' || c == '*' || c == '/'){
					   state = 4;
					   readchar = false;
				   }
				   else if (c == '=')
					   state = 5;
				   else if (c == '\n')
					   state = 6;
				   if (state!=0) {
					   token.line = line;
					   token.num = num;
				   }
				   break;
		}
		case 1:{
				   if (isalnum(c) || c == '_'){
					   token.str += c;
				   }
				   else {
						 token.type = IDEN;
						 pushtoken = true;
						 readchar = false;
				   }
				   break;
		}
		case 2:{
				   if (isdigit(c)) {
					   token.str += c;
				   }
				   else if (c == '.'){
					   token.str += c;
					   state = 3;
				   }
				   else  {
					   token.type = NUM;
					   pushtoken = true;
					   readchar = false;
				   }
				   break;
		}
		case 3:{
				   if (isdigit(c)) {
					   token.str += c;
				   }
				   else  {
					   if (token.str[token.str.size() - 1] == '.'){
						   std::ostringstream oss;
						   oss << token.line << ":" << token.num << "  Number format error:" << token.str<<std::endl;
						   throw TokenError(oss.str());
					   }
					   token.type = NUM;
					   pushtoken = true;
					   readchar = false;
				   }
				   break;
		}
		case 4:{
				   switch (c){
				   case '+':token.type = PLUS; break;
				   case '-':token.type = MINUS; break;
				   case '*':token.type = STAR; break; 
				   case '/':token.type = SLASH; break;
				   default:assert(0); break;
				   }
				   pushtoken = true;
				   break;
		}
		case 5:{
				   if (c == '='){
					   token.type = EQ;
				   }
				   else {
					   token.type = ASSIGN;
					   readchar = false;
				   }
				   pushtoken = true;
		}
		case 6:{
				   Token& lastToken = tokenVec[tokenVec.size() - 1];
				   if (lastToken.type != SEMICOLON && lastToken.type != LPAREN &&lastToken.type != LBRACKET){
					   token.type = SEMICOLON;
					   pushtoken = true;
				   }
		}
		default: break;
		}
		if (pushtoken){
			tokenVec.push_back(token);
			state = 0;
			pushtoken = false;
		}
		if (readchar){
			if (num == input.size()) {
				line++;
				break;
			}
			num++;
			c = input[num-1];
		}
		else
			readchar = true;
	}
	return;
}

void Tokenizer::advance(int step){
	vecPos += step;
}


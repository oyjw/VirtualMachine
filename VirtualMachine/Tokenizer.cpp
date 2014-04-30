#include "Tokenizer.h"
#include "Exceptions.h"
#include <string>
#include <vector>
#include <cassert>
#include <functional>
#include <fstream>
#include <sstream>
#include <exception>
#include <Windows.h>

#ifdef _DEBUG
#include <iostream>
#endif

Tokenizer::Tokenizer(const std::string& fileName) :ifs(fileName), vecPos(0), line(1), num(1){
	wchar_t buf[1024];
	GetCurrentDirectory(1024, buf);
	std::wcout<< buf <<std::endl;
	if (!ifs){
		std::ostringstream oss;
		oss<<"file open fail: "<<fileName<<std::endl;
		throw std::runtime_error(oss.str());
	}
	this->fileName=fileName;
	reservedWord["for"] = FOR;
	reservedWord["if"] = IF;
	reservedWord["while"] = WHILE;
	reservedWord["else"] = ELSE;
	reservedWord["print"] = PRINT;
	reservedWord["func"] = FUNCTION;
	reservedWord["return"] = RETURN;
	reservedWord["not"] = NOT;
	reservedWord["and"] = AND;
	reservedWord["or"] = OR;
}


Token* Tokenizer::getToken(int index){
	assert(vecPos<=(int)tokenVec.size());
	if (vecPos==(int) tokenVec.size())
		scan();
	return &tokenVec[vecPos + index];
}

void Tokenizer::scan(){
	tokenVec.clear();
	vecPos = 0;
	num = 1;
	char c;
	bool readchar = true;
	bool pushtoken = false;
	int state = 0;
	std::string input;
	while (1){
		std::getline(ifs, input); 

		if (ifs.fail()){
			std::ostringstream oss;
			oss << "file read fail: " << fileName << " : " << line << std::endl;
			throw std::runtime_error(oss.str());
		}
		if (input == "") continue;
		else 
			break;
	}
	input += '\n';
	curLine = input;
	c = input[num - 1];
	Token token;
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
			else if (c == '\n'){
				state = 6;
				readchar=false;
			}
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
				auto iter = reservedWord.find(token.str);
				if (iter != reservedWord.end())
					token.type=iter->second;
				else
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
			break;
		}
		case 6:{
			Token& lastToken = tokenVec[tokenVec.size() - 1];
			if (lastToken.type != SEMICOLON && lastToken.type != LPAREN &&lastToken.type != LBRACE){
				token.type = SEMICOLON;
				pushtoken = true;
			}
			break;
		}
		default: break;
		}
		if (pushtoken){
			tokenVec.push_back(token);
			state = 0;
			pushtoken = false;
			token.str = "";
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
	if (ifs.eof()){
		Token token;
		token.type=ENDOF;
		tokenVec.push_back(token);
	}
	return;
}

void Tokenizer::advance(int step){
	vecPos += step;
}


#include "Tokenizer.h"
#include "Exceptions.h"
#include <string>
#include <vector>
#include <cassert>
#include <fstream>
#include <sstream>
#include <exception>
#include <Windows.h>

#ifdef _DEBUG
#include <iostream>
#endif

Tokenizer::Tokenizer(const std::string& fileName) :ifs(fileName), vecPos(0), line(0), num(1), isAssignStmt(false){
	wchar_t buf[1024];
	GetCurrentDirectory(1024, buf);
	std::wcout<< buf <<std::endl;
	if (!ifs){
		std::ostringstream oss;
		oss<<"file open fail: "<<fileName<<std::endl;
		throw std::runtime_error(oss.str());
	}
	this->fileName=fileName;
	reservedWord["var"]=VAR;
	reservedWord["for"] = FOR;
	reservedWord["if"] = IF;
	reservedWord["while"] = WHILE;
	reservedWord["break"] = BREAK;
	reservedWord["continue"] = CONTINUE;
	reservedWord["else"] = ELSE;
	reservedWord["print"] = PRINT;
	reservedWord["func"] = FUNCTION;
	reservedWord["return"] = RETURN;
	reservedWord["not"] = NOT;
	reservedWord["and"] = AND;
	reservedWord["or"] = OR;

	reservedWord["class"] = CLASS;
}


Token* Tokenizer::getToken(int index){
	assert(vecPos+index<=(int)tokenVec.size());
	if (vecPos==(int) tokenVec.size())
		scan();
	Token* token = &tokenVec[vecPos + index];
	return token;
}

void Tokenizer::scan(){
	tokenVec.clear();
	isAssignStmt = false;
	vecPos = 0;
	num = 1;
	char c;
	bool readchar = true;
	bool pushtoken = false;
	int state = 0;
	std::string input;
	do{
		line++;
		std::getline(ifs, input); 
		if (ifs.bad() ){
			std::ostringstream oss;
			oss << "file read fail: " << fileName << " : " << line << std::endl;
			throw std::runtime_error(oss.str());
		}
	} while (input == "" && !ifs.eof());
	if (input == "" && ifs.eof()){
		Token token;
		token.type = ENDOF;
		tokenVec.push_back(token);
		return ;
	}
	Token token;
	/*token.line = line;
	token.type = LINESTART;
	tokenVec.push_back(token);*/
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
			else if (c == '+' || c == '-' || c == '*' || c == '/' || c=='(' || c== ')' || c=='{' || c=='}' ||
				c == ',' ){
				state = 4;
				readchar = false;
			}
			else if (c == '='){
				state = 5;
				isAssignStmt = true;
			}
			else if (c == '\n'){
				state = 6;
				readchar=false;
			}
			else if (c == '<'){
				state = 7;
			}
			else if (c == '>'){
				state = 8;
			}
			else if (c == '"'){
				state = 9;
			}
			else if (c == '!'){
				state = 10;
			}
			else if (isspace(c)){
				state = 0;
			}
			else if (c == ';'){
				token.type = SEMICOLON;
				token.line = line;
				token.num = num;
				pushtoken = true;
			}
			else {
				error("unknown character", num, TOKENERROR);
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
			case '(':token.type = LPAREN; break;
			case ')':token.type = RPAREN; break;
			case '{':token.type = LBRACE; break;
			case '}':token.type = RBRACE; break;
			case ',':token.type = COMMA; break;
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
			if (lastToken.type != SEMICOLON &&lastToken.type != LBRACE 
				&&lastToken.type != RBRACE){
				token.type = SEMICOLON;
				pushtoken = true;
			}
			break;
		}
		case 7:{
			if (c == '='){
				token.type = LE;
			}
			else {
				token.type = LT;
				readchar = false;
			}
			pushtoken = true;
			break;
		}
		case 8:{
			if (c == '='){
				token.type = GE;
			}
			else {
				token.type = GT;
				readchar = false;
			}
			pushtoken = true;
			break;
		}
		case 9:{
			if (c != '"'){
				token.str += c;
			}
			else {
				token.type = STRING;
				pushtoken = true;
			}
			break;
		}
		case 10:{
			if (c == '='){
				token.type = NOTEQ;
				pushtoken = true;
			}
			else {
				error("unknown token", num, TOKENERROR);
			}
			break;
		}
		default: assert(0);
		}
		if (pushtoken){
			tokenVec.push_back(token);
			state = 0;
			pushtoken = false;
			token.str = "";
		}
		if (readchar){
			if (num == input.size()) {
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

std::string getTypeStr(int type){
	switch (type){
	case ASSIGN:return "=";
	case IDEN:return "identifier";
	case LPAREN:return "(";
	case RPAREN:return ")";
	case LBRACE:return "{";
	case SEMICOLON:return ";";
	default: assert(0);
	}
}

void Tokenizer::expectedError(int type, Token* token){
	std::string message = token->str+" doesn't match , miss " + getTypeStr(type);
	error(message, token->num);
}

void Tokenizer::error(const std::string& message, int col, int errorType){
	std::ostringstream oss;
	oss << fileName << ":" << line << ":";
	if (col != 0){
		 oss<< col << ":";
	}
	oss << message << "\n" << curLine << std::endl;
	if (errorType == 0)
 		throw SyntaxError(oss.str());
	else if (errorType == 1)
		throw SymbolError(oss.str());
	else if (errorType == 2)
		throw TypeError(oss.str());
	else if (errorType == 3)
		throw TokenError(oss.str());
	else assert(0);
}


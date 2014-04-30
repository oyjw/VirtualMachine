#include "tokenizer.h"
#include "Parser.h"
#include "Exceptions.h"
#include "OpCode.h"
#include "SymbolTable.h"
#include <sstream>

#define NOP 0

void Parser::term(){
	factor();
	term2();
}

void Parser::term2(){
	if (tokenizer->getToken()->type == PLUS){
		match(PLUS);
		factor();
		byteCode.push_back((char)OP_ADD);
		term2();
	}
	else if (tokenizer->getToken()->type == MINUS){
		match(MINUS);
		factor();
		byteCode.push_back((char)OP_SUB);
		term2();
	}
}

void Parser::factor(){
	Token* token = tokenizer->getToken();
	if (token->type == LPAREN) {
		tokenizer->advance();
		term();
		match(RPAREN);
	}
	else if (token->type == NUM || token->type == IDEN) {
		basic();
	}
	else {
		//char err[100];
		fprintf(stderr, "%s:%d:%d  factor match fail\n", tokenizer->getFileName(), token->line, token->num);
		exit(1);
	}
	factor2();
}

void Parser::factor2(){
	if (tokenizer->getToken()->type == STAR){
		match(STAR);
		basic();
		byteCode.push_back((char)OP_MUL);
		factor2();
	}
	else if (tokenizer->getToken()->type == SLASH){
		match(SLASH);
		basic();
		byteCode.push_back((char)OP_DIV);
		factor2();
	}
}

void Parser::basic(){
	Token* token = tokenizer->getToken();
	if (token->type == NUM) {
		float val = (float)atof(token->str.c_str());
		byteCode.push_back(PUSHREAL);
		pushFloat(val);
	}
	else if (token->type == IDEN) {
		Token* nexttoken = tokenizer->getToken(1);
		if (nexttoken->type == LPAREN){
			functioncall();
			return;
		}
		int n = symTab->findSym(token->str);
		if (n == -1)
			throw SymbolError("");
		byteCode.push_back((char)(isGlobal ? PUSHGLOBAL : PUSHLOCAL));
		pushWord(n);
	}
	tokenizer->advance();
	/*else if (token->type == MINUS) {
		match(MINUS);
		token = tokenizer->getToken();
		match(NUM);
		double val = atof(token->str.c_str());
		val = -val;
	}*/
}



void Parser::program(){
	Token* token = tokenizer->getToken();
	if (token->type != ENDOF){
		elem();
		program();
	}
}

void Parser::elem(){
	Token* token = tokenizer->getToken();
	if (token->type == FUNCTION)
		function();
	else stmt();
}

void Parser::stmt(){
	Token* token = tokenizer->getToken();
	if (token->type == IDEN){
		Token* nexttoken = tokenizer->getToken(1);
		if (nexttoken->type == ASSIGN)
			assignStmt();
		else if (nexttoken->type == LPAREN)
			functioncall();
		else {
			std::ostringstream oss;
			oss<<tokenizer->getlinenum()<<"\tUnknown syntax\n\t"<< tokenizer->getlinestr();
			throw SyntaxError(oss.str());
		}
	}
	else if (token->type == PRINT){
		printStmt();
	}
	else if (token->type == RETURN){
		returnStmt();
	}
	match(SEMICOLON);
}

void Parser::printStmt(){

}

void Parser::returnStmt(){
	match(RETURN);
	term();
}

void Parser::assignStmt(){
	Token* token = tokenizer->getToken();
	std::string var = token->str;
	tokenizer->advance();
	match(ASSIGN);
	int n= symTab->putSym(var);
	term();
	byteCode.push_back(isGlobal ? STOREGLOBAL : STORELOCAL);
	pushWord(n);
}

void Parser::function(){
	match(FUNCTION);
	match1(IDEN);
	byteCode.push_back((char)JMP);
	int pos = (int)byteCode.size();
	pushWord(0);
	byteCode.push_back((char)BEGINFCALL);
	Token t = *tokenizer->getToken();
	if (symTab->findSym(t.str) != -1){
		std::ostringstream oss;
		oss << tokenizer->getlinenum() << "\tSymbol redefinition\n\t" << tokenizer->getlinestr();
		throw SymbolError(oss.str());
	}
	symTab->putSym(t.str);
	tokenizer->advance();
	match(LPAREN);
	Token* token = tokenizer->getToken();
	int nargs = 0;
	symTab = std::make_shared<SymbolTable>(symTab);
	while (1){
		if (token->type == IDEN){
			symTab->putSym(token->str);
			nargs++;
			tokenizer->advance();
			match(COMMA);
			token = tokenizer->getToken();
		}
		else if (token->type == RPAREN){
			tokenizer->advance();
			break;
		}
		else {
			std::ostringstream oss;
			oss << tokenizer->getlinenum() << "\tFunction definition error\n\t" << tokenizer->getlinestr();
			throw SymbolError(oss.str());
		}
	}
	match(LBRACE);
	isGlobal = true;
	token = tokenizer->getToken();
	while (token->type!=RBRACE)
		stmt();
	match(RBRACE);
	int n = byteCode.size() - (pos + 2);
	setWord(pos, n);
	byteCode.push_back((char)ADJUST);
	byteCode.push_back(symTab->getSymNum() - nargs);
	byteCode.push_back((char)ENDFCALL);

	isGlobal = false;
	symTab = symTab->getGlobalSym();
}

void Parser::functioncall(){
	Token* token = tokenizer->getToken();
	if (symTab->findSym(token->str) == -1){
		std::ostringstream oss;
		oss << tokenizer->getlinenum() << "\tUnknown symbol\n\t" << tokenizer->getlinestr();
		throw SymbolError(oss.str());
	}
	tokenizer->advance();
	match(LPAREN);
	token = tokenizer->getToken();
	byteCode.push_back((char)PUSHPC);
	int pos = (int)byteCode.size();
	pushWord(0);
	byteCode.push_back((char)PUSHBASE);
	while (1){
		if (token->type==NUM ||token->type==IDEN)
			basic();
		else{
			std::ostringstream oss;
			oss << tokenizer->getlinenum() << "\tFunction call error\n\t" << tokenizer->getlinestr();
			throw SyntaxError(oss.str());
		}
		token = tokenizer->getToken();
		if (token->type == RPAREN){
			match(RPAREN);
			break;
		}
		else if(token->type==COMMA){
			match(COMMA);
		}
		else{
			std::ostringstream oss;
			oss << tokenizer->getlinenum() << "\tFunction call error\n\t" << tokenizer->getlinestr();
			throw SyntaxError(oss.str());
		}
	}
	byteCode.push_back((char)CALLFUNC);
	setWord(pos, (int)byteCode.size());
}

void Parser::stmts(){
	Token* token = tokenizer->getToken();
	if (token->type == LBRACE){
		match(LBRACE);
		token = tokenizer->getToken();
		while (1){
			if (token->type == RBRACE){
				match(RBRACE);
				break;
			}
			else
				stmt();
		}
	}
	else{
		stmt();
	}
}

void Parser::ifstmt(){
	match(IF);
	orExpr();
	byteCode.push_back((char)JMPIFN);
	int pos = (int)byteCode.size();
	pushWord(0);
	stmts();
	Token* token = tokenizer->getToken();
	if (token->type == ELSE){
		match(ELSE);
		setWord(pos, (int)byteCode.size() + 2 - (pos + 2));
		byteCode.push_back((char)JMP);
		pos = (int)byteCode.size();
		pushWord(0);
		stmts();
		setWord(pos, (int)byteCode.size() - (pos + 2));
	}
	else{
		setWord(pos, (int)byteCode.size() - (pos + 2));
	}
}

void Parser::orExpr(){
	andExpr();
	orExpr2();
}

void Parser::orExpr2(){
	Token* token = tokenizer->getToken();
	if (token->type == OR){
		match(OR);
		andExpr();
		byteCode.push_back((char)OP_OR);
		orExpr2();
	}
}

void Parser::andExpr(){
	notExpr();
	andExpr2();
}

void Parser::andExpr2(){
	Token* token = tokenizer->getToken();
	if (token->type == AND){
		match(AND);
		notExpr();
		byteCode.push_back((char)OP_AND);
		andExpr2();
	}
}

void Parser::notExpr(){
	Token* token = tokenizer->getToken();
	bool flag = false;
	if (token->type == NOT){
		flag = true;
		match(NOT);
	}
	basic();
	if (flag)
		byteCode.push_back((char)OP_NOT);
}

void Parser::match(TokenType type){
	match1(type);
	tokenizer->advance();
}

void Parser::match1(TokenType type){
	Token* token = tokenizer->getToken();
	if (token->type != type) {
		std::ostringstream oss;
		oss << tokenizer->getlinenum() << "\tSyntax error\n\t" << tokenizer->getlinestr();
		throw SyntaxError(oss.str());
	}
}




















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
		byteCode->push_back((char)OP_ADD);
		term2();
	}
	else if (tokenizer->getToken()->type == MINUS){
		match(MINUS);
		factor();
		byteCode->push_back((char)OP_SUB);
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
		byteCode->push_back((char)OP_MUL);
		factor2();
	}
	else if (tokenizer->getToken()->type == SLASH){
		match(SLASH);
		basic();
		byteCode->push_back((char)OP_DIV);
		factor2();
	}
}

void Parser::basic(){
	Token* token = tokenizer->getToken();
	if (token->type == NUM) {
		float val = (float)atof(token->str.c_str());
		byteCode->push_back(PUSHREAL);
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
		byteCode->push_back(isGlobal ? PUSHGLOBAL : PUSHLOCAL);
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
	else {
		std::ostringstream oss;
		oss << tokenizer->getFileName() <<" : " << tokenizer->getlinenum() 
			<< "\tUnknown syntax\n\t" << tokenizer->getlinestr();
		throw SyntaxError(oss.str());
	}
	match(SEMICOLON);
}

void Parser::printStmt(){
	match(PRINT);
	Token* token = tokenizer->getToken();
	tokenizer->advance();
	int n = symTab->findSym(token->str);
	if (n == -1){
		std::ostringstream oss;
		oss << tokenizer->getFileName() << " : " << tokenizer->getlinenum() << "\tSymbol not found\n\t" << tokenizer->getlinestr();
		throw SymbolError(oss.str());
	}
	byteCode->push_back(PRINTFUNC);
	pushWord(n);
}


void Parser::returnStmt(){
	match(RETURN);
	term();
	byteCode->push_back(RETCODE);
}

void Parser::assignStmt(){
	Token* token = tokenizer->getToken();
	std::string var = token->str;
	tokenizer->advance();
	match(ASSIGN);
	int n= symTab->findSymLocal(var);
	if (n == -1){
		n = symTab->putSym(var);
	}
	term();
	byteCode->push_back(isGlobal ? STOREGLOBAL : STORELOCAL);
	pushWord(n);
}

void Parser::function(){
	match(FUNCTION);
	match1(IDEN);
	/*byteCode->push_back((char)JMP);
	int pos = (int)byteCode->size();
	pushWord(0);*/
	
	Token t = *tokenizer->getToken();
	int index=symTab->findSymLocal(t.str);
	if (index != -1){
		std::ostringstream oss;
		oss << tokenizer->getlinenum() << "\tSymbol redefinition\n\t" << tokenizer->getlinestr();
		throw SymbolError(oss.str());
	}
	index = symTab->putSym(t.str);
	tokenizer->advance();
	match(LPAREN);
	Token* token = tokenizer->getToken();
	int nargs = 0;
	symTab = std::make_shared<SymbolTable>(symTab);

	byteCodePtr = std::make_shared<ByteCode>(byteCodePtr);
	this->byteCode = &byteCodePtr->v;

	while (1){
		if (token->type == IDEN){
			if (-1 != symTab->findSymLocal(token->str)){
				std::ostringstream oss;
				oss << tokenizer->getlinenum() << "\tFunction definition error\n\t" << tokenizer->getlinestr();
				throw SyntaxError(oss.str());
			}
			symTab->putSym(token->str);
			nargs++;
			tokenizer->advance();
			token = tokenizer->getToken();
			if (token->type == COMMA){
				match(COMMA);
				token = tokenizer->getToken();
				continue;
			}
			else 
				break;
		}
		else {
			std::ostringstream oss;
			oss << tokenizer->getlinenum() << "\tFunction definition error\n\t" << tokenizer->getlinestr();
			throw SyntaxError(oss.str());
		}
	}
	match(RPAREN);
	match(LBRACE);
	isGlobal = false;
	token = tokenizer->getToken();
	while (token->type != RBRACE){
		stmt();
		token = tokenizer->getToken();
	}
	match(RBRACE);

	
	FunObj *obj = new FunObj;
	obj->nargs = nargs;
	obj->bytes = std::move(byteCodePtr->v);
	Object objectHolder;
	objectHolder.type = FUNOBJ;
	objectHolder.value.funObj = obj;
	isGlobal = true;
	symTab = symTab->getNext();
	symTab->putObj(index , objectHolder);
	
	byteCodePtr = byteCodePtr->getNext();
	this->byteCode = &byteCodePtr->v;
}

void Parser::functioncall(){
	Token* token = tokenizer->getToken();
	int index =symTab->findSymLocal(token->str);
	if (index == -1){
		std::ostringstream oss;
		oss << tokenizer->getlinenum() << "\tUnknown symbol\n\t" << tokenizer->getlinestr();
		throw SymbolError(oss.str());
	}
	byteCode->push_back(PUSHGLOBAL);
	pushWord(index);


	tokenizer->advance();
	match(LPAREN);
	token = tokenizer->getToken();
	int nargs=0;
	while (1){
		if (token->type == NUM || token->type == IDEN){
			basic();
			nargs++;
		}
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
	byteCode->push_back((char)CALLFUNC);
	byteCode->push_back((char)nargs);
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
	byteCode->push_back((char)JMPIFN);
	int pos = (int)byteCode->size();
	pushWord(0);
	stmts();
	Token* token = tokenizer->getToken();
	if (token->type == ELSE){
		match(ELSE);
		setWord(pos, (int)byteCode->size() + 2 - (pos + 2));
		byteCode->push_back((char)JMP);
		pos = (int)byteCode->size();
		pushWord(0);
		stmts();
		setWord(pos, (int)byteCode->size() - (pos + 2));
	}
	else{
		setWord(pos, (int)byteCode->size() - (pos + 2));
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
		byteCode->push_back((char)OP_OR);
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
		byteCode->push_back((char)OP_AND);
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
		byteCode->push_back((char)OP_NOT);
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




















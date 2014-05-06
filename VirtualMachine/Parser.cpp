#include "tokenizer.h"
#include "Parser.h"
#include "Exceptions.h"
#include "OpCode.h"
#include "SymbolTable.h"
#include <sstream>

void Parser::declarations(){
	match(VAR);
	std::vector<char>::size_type pos;
	if(!symTab->isGlobal){
		byteCode->push_back(ADJUST);
		pos = byteCode->size();
		byteCode->push_back(0);
	}
	int n=declList();
	if(!symTab->isGlobal)
		(*byteCode)[pos]=n;
}

int Parser::declList(){
	int n=0;
	decl();
	n++;
	Token* token = tokenizer->getToken();
	if (token->type == COMMA){
		match(COMMA);
		n+=declList();
	}
	return n;
}

void Parser::decl(){
	Token* token = tokenizer->getToken();
	if (token->type != IDEN){
		std::ostringstream oss;
		oss<<tokenizer->getFileName() << "\t"<<tokenizer->getlinenum()<<"\tExpecting identifier\n\t"<< tokenizer->getlinestr();
		throw SyntaxError(oss.str());
	}
	if (symTab->isSymExistLocal(token->str)){
		std::ostringstream oss;
		oss<<tokenizer->getFileName() << "\t"<<tokenizer->getlinenum()<<"\tSymbol redefinition\n\t"<< tokenizer->getlinestr();
		throw SymbolError(oss.str());
	}
	int n=symTab->putSym(token->str);
	n += symTab->nLocalVars;
	tokenizer->advance();
	token = tokenizer->getToken();
	if (token->type!=ASSIGN)
		return;
	match(ASSIGN);
	term();
	byteCode->push_back(symTab->isGlobal?STOREGLOBAL : STORELOCAL);
	pushWord(n);
}

void Parser::assignStmt(){
	Token* token = tokenizer->getToken();
	std::string var = token->str;
	tokenizer->advance();
	match(ASSIGN);
	int n;
	bool global;
	std::pair<bool,int> pair= symTab->findSym(var);
	n=pair.second;
	global = pair.first;
	if (!pair.first){
		SymPtr sp=symTab;
		while (sp->getNext() != NULL){
			sp = sp->getNext();
		}
		n=sp->putSym(var);
		global=true;
	}

	term();
	byteCode->push_back(global ? STOREGLOBAL : STORELOCAL);
	pushWord(n);
}


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
		auto pair = symTab->findSym(token->str);
		if (pair.second == -1)
			throw SymbolError("");
		byteCode->push_back(pair.first ? PUSHGLOBAL : PUSHLOCAL);
		pushWord(pair.second);
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
	bool matchSemi=true;
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
	else if (token->type == IF){
		ifStmt();
		matchSemi=false;
	}
	else if (token->type == VAR){
		declarations();
	}
	else {
		std::ostringstream oss;
		oss << tokenizer->getFileName() <<" : " << tokenizer->getlinenum() 
			<< "\tUnknown syntax\n\t" << tokenizer->getlinestr();
		throw SyntaxError(oss.str());
	}
	if(matchSemi)
		match(SEMICOLON);
}

void Parser::printStmt(){
	match(PRINT);
	term();
	byteCode->push_back(PRINTFUNC);
}


void Parser::returnStmt(){
	match(RETURN);
	term();
	byteCode->push_back(RETCODE);
}


void Parser::function(){
	match(FUNCTION);
	match1(IDEN);
	/*byteCode->push_back((char)JMP);
	int pos = (int)byteCode->size();
	pushWord(0);*/
	
	Token t = *tokenizer->getToken();
	
	if (symTab->isSymExistLocal(t.str)){
		std::ostringstream oss;
		oss << tokenizer->getlinenum() << "\tSymbol redefinition\n\t" << tokenizer->getlinestr();
		throw SymbolError(oss.str());
	}
	int index = symTab->putSym(t.str);
	tokenizer->advance();
	match(LPAREN);
	Token* token = tokenizer->getToken();
	int nargs = 0;
	symTab = std::make_shared<SymbolTable>(symTab,0);

	byteCodePtr = std::make_shared<ByteCode>(byteCodePtr);
	this->byteCode = &byteCodePtr->v;

	while (1){
		if (token->type == IDEN){
			if (symTab->isSymExistLocal(token->str)){
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
	auto p =symTab->findSym(token->str);
	if (p.second == -1){
		std::ostringstream oss;
		oss << tokenizer->getlinenum() << "\tUnknown symbol\n\t" << tokenizer->getlinestr();
		throw SymbolError(oss.str());
	}
	SymPtr sp=symTab;
	while (sp->getNext() != NULL){
		sp = sp->getNext();
	}
	Object& obj = sp->getObj(p.second);
	/*if (obj.type != FUNOBJ){
		std::ostringstream oss;
		oss << tokenizer->getlinenum() << "\tNot callable\n\t" << tokenizer->getlinestr();
		throw SymbolError(oss.str());
	}*/
	assert(p.first);
	byteCode->push_back(PUSHGLOBAL);
	pushWord(p.second);


	tokenizer->advance();
	match(LPAREN);
	token = tokenizer->getToken();
	int nargs=0;
	while (1){
		if (token->type == NUM || token->type == IDEN){
			term();
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
			token = tokenizer->getToken();
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
				token = tokenizer->getToken();
		}
	}
	else{
		stmt();
	}
}

void Parser::ifStmt(){
	match(IF);
	labelList orLabel;
	labelList andLabel;
	match(LPAREN);
	orExpr(orLabel,andLabel);
	match(RPAREN);
	byteCode->push_back((char)JMP);
	int pos = (int)byteCode->size();
	pushWord(0);

	for (auto& p : andLabel){
		setWord(p.first,p.second);
	}

	for (auto& i : orLabel){
		setWord(i.first, byteCode->size()-i.first-WORDSIZE);
	}

	symTab = std::make_shared<SymbolTable>(symTab,symTab->isGlobal?0:symTab->getLocalVars());
	stmts();
	int nLocals = symTab->getSymNum();
	byteCode->push_back(ADJUST);
	byteCode->push_back((char)-nLocals);

	Token* token = tokenizer->getToken();
	if (token->type == ELSE){
		match(ELSE);
		setWord(pos, int(byteCode->size() + 3 - (pos + 2)));
		byteCode->push_back((char)JMP);
		pos = (int)byteCode->size();
		pushWord(0);
		stmts();
		int nLocals = symTab->getSymNum();
		byteCode->push_back(ADJUST);
		byteCode->push_back((char)-nLocals);
		setWord(pos, byteCode->size() - (pos + 2));
	}
	else{
		setWord(pos, byteCode->size() - (pos + 2));
	}
	symTab = symTab->getNext();
}

void Parser::orExpr(labelList& orLabel,labelList& andLabel){
	andExpr(orLabel,andLabel);
	orExpr2(orLabel,andLabel);
}

void Parser::orExpr2(labelList& orLabel,labelList& andLabel){
	Token* token = tokenizer->getToken();
	if (token->type == OR){
		match(OR);
		andExpr(orLabel,andLabel);
		orExpr2(orLabel,andLabel);
	}
}

void Parser::andExpr(labelList& orLabel,labelList& andLabel){
	labelList andLabel2;
	notExpr(orLabel,andLabel2,false);
	andExpr2(orLabel,andLabel2);
	byteCode->push_back(JMP);
	orLabel.push_back(std::make_pair(byteCode->size(),0));
	pushWord(0);
	for (auto& i : andLabel2){
		i.second = byteCode->size()-i.first-WORDSIZE;
		andLabel.push_back(i);
	}
}

void Parser::andExpr2(labelList& orLabel,labelList& andLabel){
	Token* token = tokenizer->getToken();
	if (token->type == AND){
		match(AND);
		notExpr(orLabel,andLabel,false);
		andExpr2(orLabel,andLabel);
	}
}

void Parser::notExpr(labelList& orLabel,labelList& andLabel,bool notFlag){
	Token* token = tokenizer->getToken();
	if (token->type == NOT){
		match(NOT);
		notFlag = notFlag? false:true;
	}
	token = tokenizer->getToken();
	if (token->type == NOT){
		notExpr(orLabel,andLabel,notFlag);
	}
	else if (token->type == LPAREN){
		match(LPAREN);
		labelList orLabel2;
		labelList andLabel2;
		orExpr(orLabel2,andLabel2);
		
		if(!notFlag){
			for (auto& p : andLabel2){
				setWord(p.first,p.second);
			}
			byteCode->push_back(JMP);
			andLabel.push_back(std::make_pair(byteCode->size(),0));
			pushWord(0);
			for (auto& p : orLabel2){
				setWord(p.first,byteCode->size()-p.first-WORDSIZE);
			}
		}
		else{
			for (auto& p : andLabel2){
				setWord(p.first,p.second);
			}
			for (auto& p : orLabel2){
				andLabel.push_back(p);
			}
		}
		match(RPAREN);
	}
	else {
		relaExpr();
		if(notFlag){
			byteCode->push_back(OP_NOT);
		}
		byteCode->push_back(JMPIFF);
		andLabel.push_back(std::make_pair(byteCode->size(),0));
		pushWord(0);
		
	}
}

void Parser::relaExpr(){
	basic();
	Token t = *tokenizer->getToken();
	if (t.type !=EQ && t.type != NOTEQ && t.type != LT && t.type != GT && t.type != LE && t.type != GE)
		return ;
	tokenizer->advance();
	Token *token = tokenizer->getToken();;
	basic();
	switch (t.type){
	case EQ:byteCode->push_back(OP_EQ); break;
	case NOTEQ:byteCode->push_back(OP_NOTEQ); break;
	case LT:byteCode->push_back(OP_LT); break;
	case GT:byteCode->push_back(OP_GT); break;
	case LE:byteCode->push_back(OP_LE); break;
	case GE:byteCode->push_back(OP_GE); break;
	default:{
		std::ostringstream oss;
		oss<<tokenizer->getFileName() << "\t"<<tokenizer->getlinenum()<<"\tExpecting relation operator\n\t"<< tokenizer->getlinestr();
		throw SyntaxError(oss.str());
	}
	}
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



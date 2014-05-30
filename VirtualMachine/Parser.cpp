#include "tokenizer.h"
#include "Parser.h"
#include "Exceptions.h"
#include "OpCode.h"
#include "SymbolTable.h"
#include "Object.h"
#include "StringPool.h"
#include "ObjectPool.h"
#include <sstream>
#include <climits>

Parser::~Parser() {
	delete tokenizer;
}

int Parser::addSymbol(Token* token){
	if (symTab->isSymExistLocal(token->str)){
		tokenizer->error("symbol redefinition",token,SYMBOLERROR);
	}
	return symTab->putSym(token->str);
}


std::pair<bool,int> Parser::parseIdentifier(Token* token, bool push){
	auto pair = symTab->findSym(token->str);
	if (pair.second == -1){
		tokenizer->error("Unknown symbol",token,SYMBOLERROR);
	}
	byteCode->push_back(char(pair.first ? PUSHGLOBAL : PUSHLOCAL));
	pushWord(pair.second);
	return pair;
}

int Parser::funcArgs(Token* function){
	int nArgs=0;
	Token* token = tokenizer->getToken();
	if (token->type!=RPAREN){
		while(1){
			if (nArgs > 16){
				tokenizer->error("too many arguments", function);
			}
			term();
			nArgs++;
			token = tokenizer->getToken();
			if(token->type!=COMMA)
				break;
			tokenizer->advance();
			token = tokenizer->getToken();
		}
	}
	return nArgs;
}

void Parser::declarations(){
	tokenizer->advance();
	std::vector<char>::size_type pos=0;
	if(!symTab->isGlobal){
		byteCode->push_back(ADJUST);
		pos = byteCode->size();
		byteCode->push_back(0);
	}
	int n=declList();
	if (n > 127){
		tokenizer->error("too many variables");
	}
	if(!symTab->isGlobal)
		(*byteCode)[pos]=(char)n;
}

int Parser::declList(){
	int n=0;
	decl();
	n++;
	Token* token = tokenizer->getToken();
	if (token->type == COMMA){
		tokenizer->advance();
		n+=declList();
	}
	return n;
}


void Parser::decl(){
	Token* token = tokenizer->getToken();
	match(IDEN);
	int n = addSymbol(token);
	n += symTab->nLocalVars;
	token = tokenizer->getToken();
	if (token->type!=ASSIGN)
		return;
	tokenizer->advance();
	term();
	byteCode->push_back(char(symTab->isGlobal?STOREGLOBAL : STORELOCAL));
	pushWord(n);
}

void Parser::assignStmt(){
	Token* token = tokenizer->getToken();
	tokenizer->advance();
	Token* token2 = tokenizer->getToken();
	int n = 0;
	bool global = false;
	bool objLvalue = false;
	if (isClass && !isClassFunction){
		addSymbol(token);
		int index = getSharedString(token->str);
		byteCode->push_back((char)PUSHGLOBAL);
		pushWord(clsIndex);
		byteCode->push_back((char)PUSHSTRING);
		pushWord(index);
	}
	else {
		if (token2->type == COLON){
			Token* token = tokenizer->getToken();
			auto p = parseIdentifier(token, false);
			objCall(true, p);
			objLvalue = true;
		}
		else{
			std::pair<bool,int> pair= symTab->findSym(token->str);
			n = pair.second;
			global = pair.first;
			if (!pair.first){
				SymPtr sp=symTab;
				while (sp->getNext() != NULL){
					sp = sp->getNext();
				}
				n=sp->putSym(token->str);
				global=true;
			}
		}
	}
	
	match(ASSIGN);
	term();
	if (isClass || objLvalue){
		byteCode->push_back((char)SETATTR);
	}
	else {
		byteCode->push_back(char(global ? STOREGLOBAL : STORELOCAL));
		pushWord(n);
	}
}

void Parser::term(){
	factor();
	term2();
}

void Parser::term2(){
	if (tokenizer->getToken()->type == PLUS){
		tokenizer->advance();
		factor();
		byteCode->push_back((char)OP_ADD);
		term2();
	}
	else if (tokenizer->getToken()->type == MINUS){
		tokenizer->advance();
		factor();
		byteCode->push_back((char)OP_SUB);
		term2();
	}
}

void Parser::factor(){
	Token* token = tokenizer->getToken();
	if (token->type == LPAREN || token->type == NUM || token->type == IDEN || token->type == MINUS) {
		basic();
		factor2();
	}
	else {
		tokenizer->error("expecting rvalue",token);
	}
}

void Parser::factor2(){
	if (tokenizer->getToken()->type == STAR){
		tokenizer->advance();
		basic();
		byteCode->push_back((char)OP_MUL);
		factor2();
	}
	else if (tokenizer->getToken()->type == SLASH){
		tokenizer->advance();
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
		tokenizer->advance();
	}
	else if (token->type == MINUS){
		tokenizer->advance();
		token = tokenizer->getToken();
		if (token->type != NUM){
			tokenizer->error("number format error",token,TYPEERROR);
		}
		tokenizer->advance();
		float val = (float)atof(token->str.c_str());
		val=-val;
		byteCode->push_back(PUSHREAL);
		pushFloat(val);
	}
	else if (token->type == IDEN) {
		Token* nexttoken = tokenizer->getToken(1);
		if (nexttoken->type == COLON){
			Token* token = tokenizer->getToken();
			auto p = parseIdentifier(token, false);
			objCall(false, p);
			return;
		}
		if (nexttoken->type == LPAREN){
			functioncall();
			return;
		}
		parseIdentifier(token, true);
		tokenizer->advance();
	}
	else if (token->type == LPAREN){
		tokenizer->advance();
		term();
		match(RPAREN);
	}
	else if (token->type == STRING){
		int index = getSharedString(token->str);
		byteCode->push_back(PUSHSTRING);
		pushWord(index);
		tokenizer->advance();
		Token* token = tokenizer->getToken();
		if (token->type == COLON){
			objCall(false, )
		}
	}
	else {
		tokenizer->error("syntax error",token);
	}
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
			tokenizer->error("syntax error",token);
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
	else if (token->type == WHILE){
		whileStmt();
		matchSemi=false;
	}
	else if (token->type == BREAK){
		breakStmt();
	}
	else if (token->type == CONTINUE){
		continueStmt();
	}
	else {
		tokenizer->error("syntax error",token);
	}
	if(matchSemi)
		match(SEMICOLON);
}

void Parser::printStmt(){
	tokenizer->advance();
	term();
	byteCode->push_back(PRINTFUNC);
}


void Parser::returnStmt(){
	tokenizer->advance();
	Token *token = tokenizer->getToken();
	if (token->type == SEMICOLON){
		byteCode->push_back(RET0);
	}
	else {
		term(); 
		byteCode->push_back(RETCODE);
	}
}


void Parser::function(){
	tokenizer->advance();
	Token *token = tokenizer->getToken();
	match(IDEN);
	
	int index = addSymbol(token);
	StrObj* strObj = NULL;
	if (isClass){
		int sindex = getSharedString(token->str);
		strObj = stringPoolPtr->getStrObj(sindex);
	}
	match(LPAREN);
	token = tokenizer->getToken();
	int nArgs = 0;
	symTab = std::make_shared<SymbolTable>(symTab,0);

	byteCodePtr = std::make_shared<ByteCode>(byteCodePtr);
	this->byteCode = &byteCodePtr->v;

	while (token->type == IDEN){
		if (nArgs > 16){
			tokenizer->error("too many parameters");
		}
		addSymbol(token);
		nArgs++;
		tokenizer->advance();
		token = tokenizer->getToken();
		if (token->type != COMMA){
			break;
		}
		tokenizer->advance();
		token = tokenizer->getToken();
	}
	match(RPAREN);
	match(LBRACE);
	token = tokenizer->getToken();
	while (token->type != RBRACE){
		stmt();
		token = tokenizer->getToken();
	}
	tokenizer->advance();
	FunObj *obj = new FunObj;
	obj->nArgs = nArgs;
	obj->bytes = std::move(byteCodePtr->v);
	Object objectHolder;
	objectHolder.type = FUNOBJ;
	objectHolder.value.funObj = obj;
	symTab = symTab->getNext();
	if (isClass){
		assert(strObj != NULL);
		curClsType->clsAttrs.insert(std::make_pair(strObj, objectHolder));
	}
	else 
		symTab->putObj(index , objectHolder);
	
	byteCodePtr = byteCodePtr->getNext();
	this->byteCode = &byteCodePtr->v;
}

void Parser::functioncall(){
	Token* token = tokenizer->getToken();
	parseIdentifier(token, true);
	tokenizer->advance();
	match(LPAREN);
	int nArgs=funcArgs(token);
	
	match(RPAREN);
	byteCode->push_back((char)CALLFUNC);
	byteCode->push_back((char)nArgs);
}


void Parser::stmts(){
	Token* token = tokenizer->getToken();
	if (token->type == LBRACE){
		tokenizer->advance();
		token = tokenizer->getToken();
		while (token->type != RBRACE){
			stmt();
			token = tokenizer->getToken();
		}
		tokenizer->advance();
	}
	else{
		stmt();
	}
}

void Parser::ifStmt(){
	tokenizer->advance();
	match(LPAREN);
	orExpr();
	match(RPAREN);
	byteCode->push_back((char)JMPIFF);
	size_t pos = byteCode->size();
	pushWord(0);

	symTab = std::make_shared<SymbolTable>(symTab,symTab->isGlobal?0:symTab->getLocalVars());
	stmts();
	int nLocals = symTab->getSymNum();
	byteCode->push_back(ADJUST);
	byteCode->push_back((char)-nLocals);

	Token* token = tokenizer->getToken();
	if (token->type == ELSE){
		tokenizer->advance();
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

void Parser::orExpr(){
	andExpr();
	orExpr2();
}

void Parser::orExpr2(){
	Token* token = tokenizer->getToken();
	if (token->type == OR){
		tokenizer->advance();
		andExpr();
		byteCode->push_back((char)OP_OR);
		orExpr2();
	}
}

void Parser::andExpr(){
	notExpr(false);
	andExpr2();
}

void Parser::andExpr2(){
	Token* token = tokenizer->getToken();
	if (token->type == AND){
		tokenizer->advance();
		notExpr(false);
		byteCode->push_back((char)OP_AND);
		andExpr2();
	}
}

void Parser::notExpr(bool notFlag){
	Token* token = tokenizer->getToken();
	if (token->type == NOT){
		tokenizer->advance();
		notFlag = notFlag? false:true;
	}
	token = tokenizer->getToken();
	if (token->type == NOT){
		notExpr(notFlag);
	}
	else if (token->type == LPAREN){
		match(LPAREN);
		orExpr();
		if(notFlag){
			byteCode->push_back(OP_NOT);
		}
		match(RPAREN);
	}
	else {
		relaExpr();
		if(notFlag){
			byteCode->push_back(OP_NOT);
		}
	}
}

void Parser::relaExpr(){
	basic();
	Token t = *tokenizer->getToken();
	if (t.type !=EQ && t.type != NOTEQ && t.type != LT && t.type != GT && t.type != LE && t.type != GE)
		return ;
	tokenizer->advance();
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

void Parser::whileStmt(){
	tokenizer->advance();
	if (loopLabelPtr == NULL){
		loopLabelPtr = std::make_shared<LoopLabel>();
	}
	else{
		loopLabelPtr = std::make_shared<LoopLabel>(loopLabelPtr);
	}
	loopLabelPtr->start=byteCode->size();
	match(LPAREN);
	orExpr();
	match(RPAREN);
	byteCode->push_back((char)JMPIFF);
	int pos = (int)byteCode->size();
	pushWord(0);

	symTab = std::make_shared<SymbolTable>(symTab,symTab->isGlobal?0:symTab->getLocalVars());
	stmts();
	int nLocals = symTab->getSymNum();
	byteCode->push_back(ADJUST);
	byteCode->push_back((char)-nLocals);
	symTab = symTab->getNext();

	byteCode->push_back(JMP);
	pushWord(loopLabelPtr->start-(byteCode->size()+2));

	setWord(pos, byteCode->size()-pos-WORDSIZE);
	for (auto pos : loopLabelPtr->breaks){
		setWord(pos, byteCode->size()-pos-WORDSIZE);
	}
	for (auto pos : loopLabelPtr->continues){
		setWord(pos, byteCode->size()-pos-WORDSIZE);
	}
	loopLabelPtr = loopLabelPtr->next;
}

void Parser::breakStmt(){
	tokenizer->advance();
	byteCode->push_back(JMP);
	loopLabelPtr->breaks.push_back(byteCode->size());
	pushWord(0);
}

void Parser::continueStmt(){
	tokenizer->advance();
	byteCode->push_back(JMP);
	loopLabelPtr->continues.push_back(byteCode->size());
	pushWord(0);
}

int Parser::getSharedString(const std::string& str){
	auto iter = sharedStrings.find(str);
	int index = 0;
	if (iter == sharedStrings.end()){
		index = stringPoolPtr->putStringConstant(str);
		sharedStrings.insert(std::make_pair(str,index));
	}
	else
		index = iter->second;
	assert(index>=0 && index<=SHRT_MAX);
	return index;
}

void Parser::objCall(bool isLvalue, std::pair<bool,int> pair){
	tokenizer->advance(1);
	
	Token* token = tokenizer->getToken();
	match(IDEN);
	int index = getSharedString(token->str);
	byteCode->push_back((char)PUSHSTRING);
	pushWord(index);
	if (isLvalue)
		return;
	byteCode->push_back((char)GETATTR);
	Token *token2 = tokenizer->getToken();
	if (token2->type == LPAREN){
		byteCode->push_back(char(pair.first?PUSHGLOBAL:PUSHLOCAL));
		byteCode->push_back((char)pair.second);
		tokenizer->advance();
		int nArgs = funcArgs(token);
		match(RPAREN);
		byteCode->push_back(CALLFUNC);
		byteCode->push_back((char)nArgs);
	}
}


void Parser::newExpr(){
	tokenizer->advance();
	Token* token = tokenizer->getToken();
	auto p = symTab->findSym(token->str);
	if (p.second == -1){
		tokenizer->error("class not found",token,SYMBOLERROR);
	}
	byteCode->push_back((char)CREATEOBJ);
	pushWord(p.second);
}

void Parser::classDefinition(){
	tokenizer->advance();
	Token* token = tokenizer->getToken();
	match(IDEN);
	if (symTab->isSymExistLocal(token->str)){
		tokenizer->error("symbol redefinition",token,SYMBOLERROR);
	}
	int index = symTab->putSym(token->str);
	ClsType *cls = new ClsType;
	objectPoolPtr->putCls((void*)cls);
	curClsType = cls;
	isClass = true;
	Object object = { CLSTYPE, cls };
	symTab->putObj(index,object);
	clsIndex = index;

	token = tokenizer->getToken();
	if (token->type == LPAREN){
		tokenizer->advance();
		do{
			token = tokenizer->getToken();
			match(IDEN);
			if (!symTab->isSymExistLocal(token->str)){
				tokenizer->error("class not found",token,SYMBOLERROR);
			}
		} while (token->type != RPAREN);
	}
	tokenizer->advance();
	symTab = std::make_shared<SymbolTable>(symTab,0);

	match(LBRACE);
	token = tokenizer->getToken();
	while (token->type != RBRACE){
		if (token->type == IDEN){
			assignStmt();
			match(SEMICOLON);
		}
		else if (token->type == FUNCTION){
			isClassFunction = true;
			function();
			isClassFunction = false;
		}
		else {
			tokenizer->error("syntax error");
		}
		token = tokenizer->getToken();
	}
	tokenizer->advance();
	symTab = symTab->getNext();

	curClsType = NULL;
	isClass = false;
}

void Parser::match(int type){
	Token* token = tokenizer->getToken();
	if (token->type == type){
		tokenizer->advance();
		return;
	}
	tokenizer->expectedError(type,token);
}

void Parser::pushWord(int n){
	CodeWord code;
	code.word = (short)n;
	byteCode->push_back(code.c.c1);
	byteCode->push_back(code.c.c2);
}
void Parser::setWord(std::vector<char>::size_type pos,int n){
	CodeWord code;
	code.word = (short)n;
	(*byteCode)[pos]=code.c.c1;
	(*byteCode)[pos+1]=code.c.c2;
}
void Parser::pushFloat(float f){
	CodeFloat code;
	code.f = f;
	byteCode->push_back(code.c.c1);
	byteCode->push_back(code.c.c2);
	byteCode->push_back(code.c.c3);
	byteCode->push_back(code.c.c4);
}

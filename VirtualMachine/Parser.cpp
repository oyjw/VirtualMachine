#include "Tokenizer.h"
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

int Parser::addSymbol(){
	Token *token = tokenizer->getToken();
	match(IDEN);
	if (symTab->isSymExistLocal(token->str)){
		tokenizer->error("symbol redefinition",token->num,SYMBOLERROR);
	}
	return symTab->putSym(token->str);
}

std::pair<bool, int> Parser::parseIden(Token* token){
	auto pair = symTab->findSym(token->str);
	if (pair.second == -1){
		tokenizer->error("Unknown symbol",token->num,SYMBOLERROR);
	}
	return pair;
}

bool Parser::parseValue(int& type, int& index){
	Token* token = tokenizer->getToken();
	tokenizer->advance();
	if (token->type == IDEN){
		auto pair = parseIden(token);
		type = IDEN;
		index = pair.second;
		return pair.first;
	}
	else if (token->type == STRING){
		index = getSharedString(token->str);
		type = STRING;
	}
	else if (token->type == LBRACKET){
		nListArgs = 0;
		token = tokenizer->getToken();
		if (token->type != RBRACKET){
			while (1){
				orExpr();
				nListArgs++;
				token = tokenizer->getToken();
				if (token->type != COMMA)
					break;
				tokenizer->advance();
			}
		}
		match(RBRACKET);
		type = LBRACKET;
	}
	else if (token->type == LBRACE){
		nListArgs = 0;
		token = tokenizer->getToken();
		if (token->type != RBRACE){
			while (1){
				orExpr();
				match(COLON);
				orExpr();
				nListArgs+=2;
				token = tokenizer->getToken();
				if (token->type != COMMA)
					break;
				tokenizer->advance();
			}
		}
		match(RBRACE);
		type = LBRACE;
	}
	return false;
}

void Parser::pushValue(int type, int index, bool isGlobal ){
	switch (type){
		case IDEN:{
			byteCode->push_back(char(isGlobal? PUSHGLOBAL: PUSHLOCAL));  
			pushWord(index); 
			break;
		}
		case STRING:{
			byteCode->push_back((char)PUSHSTRING); 
			pushWord(index); 
			break;
		}
		case LBRACE:{
			if (nListArgs % 2 != 0){
				tokenizer->error("the number of arguments is wrong", 0, ARGUMENTERROR);
			}
		}
		case LBRACKET:{
			if (nListArgs >= 128){
				tokenizer->error("the number of arguments exceeds", 0, ARGUMENTERROR);
			}
			byteCode->push_back((char)CALLFUNC);
			byteCode->push_back((char)nListArgs);
			break;
		}
		default: assert(0);
	}
}

bool Parser::lvalue(){
	Token* token = tokenizer->getToken();
	match(IDEN);
	auto pair = parseIden(token);
	pushValue(IDEN, pair.second, pair.first);
	token = tokenizer->getToken();
	bool isPeriod = false;
	while(token->type == PERIOD || token->type == LBRACKET){
		isPeriod = token->type == PERIOD? true: false;
		parseSeleOp();
		token = tokenizer->getToken();
		if (token->type == PERIOD || token->type == LBRACKET){
			byteCode->push_back(char(isPeriod?GETATTR:GETINDEX));
		}
	}
	return isPeriod;
}

void Parser::rvalue(){
	int type, index;
	bool isGlobal = parseValue(type, index);
	pushValue(type, index, isGlobal);
	parseOp();
}

void Parser::parseOp(){
	Token* token = tokenizer->getToken();
	bool recursion = false;
	if (token->type == PERIOD){
		parseSeleOp();
		byteCode->push_back((char)GETATTR);
		token = tokenizer->getToken();
		recursion = true;
	}
	if (token->type == LBRACKET){
		parseSeleOp();
		byteCode->push_back((char)GETINDEX);
		token = tokenizer->getToken();
		recursion = true;
	}
	if (token->type == LPAREN){
		funcArgs(token);
		token = tokenizer->getToken();
		recursion = true;
	}
	if (recursion)
		parseOp();
}

void Parser::parseSeleOp(){
	Token* token = tokenizer->getToken();
	if (token->type == PERIOD){
		tokenizer->advance();
		token = tokenizer->getToken();
		match(IDEN);
		int index = getSharedString(token->str);
		byteCode->push_back((char)PUSHSTRING);
		pushWord(index);
	}
	else if (token->type == LBRACKET){
		tokenizer->advance();
		orExpr();
		match(RBRACKET);
	}
	else assert(0);
}


void Parser::funcArgs(Token* function){
	tokenizer->advance();
	int nArgs = 0;
	Token* token = tokenizer->getToken();
	if (token->type != RPAREN){
		while(true){
			if (nArgs > 16){
				tokenizer->error("too many arguments", function->num);
			}
			orExpr();
			nArgs++;
			token = tokenizer->getToken();
			if(token->type != COMMA)
				break;
			tokenizer->advance();
		}
	}
	match(RPAREN);
	byteCode->push_back((char)CALLFUNC);
	byteCode->push_back(char(nArgs));
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
	int n = addSymbol();
	n += symTab->nLocalVars;
	Token* token = tokenizer->getToken();
	if (token->type!=ASSIGN)
		return;
	tokenizer->advance();
	orExpr();
	byteCode->push_back(char(symTab->isGlobal?STOREGLOBAL : STORELOCAL));
	pushWord(n);
}

void Parser::assignStmt(){
	Token* token = tokenizer->getToken();
	Token* token2 = tokenizer->getToken(1);
	int n = 0;
	bool global = false;
	bool objLvalue = false;
	bool isPeriod = true;
	if (isClass && !isClassFunction){
		match(IDEN);
		auto iter = classAttrs.find(token->str);
		if (iter != classAttrs.end())
			tokenizer->error("symbol redefinition",token->num,SYMBOLERROR);
		else{
			classAttrs.insert(token->str);
		}
		token = tokenizer->getToken();
		int index = getSharedString(token->str);
		byteCode->push_back((char)PUSHGLOBAL);
		pushWord(clsIndex);
		byteCode->push_back((char)PUSHSTRING);
		pushWord(index);
	}
	else {
		if (token2->type == PERIOD || token2->type == LBRACKET){
			isPeriod = lvalue();
			objLvalue = true;
		}
		else{
			std::pair<bool,int> pair= symTab->findSym(token->str);
			if (!pair.first){
				tokenizer->error("unknown symbol",token->num);
			}
			n = pair.second;
			global = pair.first;
			tokenizer->advance();
		}
	}
	
	match(ASSIGN);
	orExpr();
	if (isClass){
		byteCode->push_back(char(SETATTR));
	}
	else if (objLvalue){
		byteCode->push_back(char(isPeriod?SETATTR:SETINDEX));
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
	if (token->type == LPAREN || token->type == NUM || token->type == IDEN || token->type == MINUS || token->type == STRING ||
		token->type == LBRACKET || token->type == LBRACE) {
		basic();
		factor2();
	}
	else {
		tokenizer->error("expecting rvalue",token->num);
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
			tokenizer->error("number format error",token->num,TYPEERROR);
		}
		tokenizer->advance();
		float val = (float)atof(token->str.c_str());
		val=-val;
		byteCode->push_back(PUSHREAL);
		pushFloat(val);
	}
	else if (token->type == IDEN) {
		rvalue();
	}
	else if (token->type == LPAREN){
		tokenizer->advance();
		orExpr();
		match(RPAREN);
	}
	else if (token->type == STRING){
		rvalue();
	}
	else if (token->type == LBRACKET){
		auto pair = symTab->findSym("list");
		if (pair.second == -1){
			tokenizer->error("class list not found",token->num,SYMBOLERROR);
		}
		byteCode->push_back(char(pair.first? PUSHGLOBAL: PUSHLOCAL));  
		pushWord(pair.second); 
		rvalue();
	}
	else if (token->type == LBRACE){
		auto pair = symTab->findSym("dict");
		if (pair.second == -1){
			tokenizer->error("class dict not found",token->num,SYMBOLERROR);
		}
		byteCode->push_back(char(pair.first? PUSHGLOBAL: PUSHLOCAL));  
		pushWord(pair.second); 
		rvalue();
	}
	else if (token->type == RESERVEDTRUE) {
		byteCode->push_back(PUSHTRUE);
		tokenizer->advance();
	}
	else if (token->type == RESERVEDFALSE) {
		byteCode->push_back(PUSHFALSE);
		tokenizer->advance();
	}
	else {
		tokenizer->error("syntax error",token->num);
	}
}

void Parser::program(){
	Token* token = tokenizer->getToken();
	while (token->type != ENDOF){
		elem();
		token = tokenizer->getToken();
	}
}

void Parser::elem(){
	Token* token = tokenizer->getToken();
	if (token->type == FUNCTION)
		function();
	else if (token->type == CLASS)
		classDefinition();
	else stmt();
	match(SEMICOLON);
}

void Parser::stmt(){
	if (debugLine == 0 || tokenizer->getlinenum() != debugLine){
		byteCode->push_back((char)SETLINE);
		pushWord(tokenizer->getlinenum());
		debugLine = tokenizer->getlinenum();
	}
	Token* token = tokenizer->getToken();
	if (token->type == IDEN){
		if (tokenizer->isAssignStmt)
			assignStmt();
		else {
			orExpr();
			byteCode->push_back((char)ADJUST);
			byteCode->push_back((char)-1);
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
	}
	else if (token->type == VAR){
		declarations();
	}
	else if (token->type == WHILE){
		whileStmt();
	}
	else if (token->type == BREAK){
		breakStmt();
	}
	else if (token->type == CONTINUE){
		continueStmt();
	}
	else {
		tokenizer->error("syntax error",token->num);
	}
}

void Parser::printStmt(){
	tokenizer->advance();
	orExpr();
	byteCode->push_back(PRINTFUNC);
}


void Parser::returnStmt(){
	tokenizer->advance();
	Token *token = tokenizer->getToken();
	if (token->type == SEMICOLON){
		byteCode->push_back(RET0);
	}
	else {
		if (isClassConstructor){
			tokenizer->error("constructor shouldn't return anything");
		}
		orExpr(); 
		byteCode->push_back(RETCODE);
	}
}


void Parser::function(){
	tokenizer->advance();
	Token *token = tokenizer->getToken();
	std::string functionName = token->str;
	int index = 0;
	if (isClass){
		auto iter = classAttrs.find(functionName);
		if (iter != classAttrs.end())
			tokenizer->error("symbol redefinition",token->num,SYMBOLERROR);
		else{
			classAttrs.insert(functionName);
		}
		if (token->type == INITMETHOD)
			isClassConstructor = true;
		tokenizer->advance();
	}
	else{
		index = addSymbol();
	}
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
		addSymbol();
		nArgs++;
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
	obj->functionName = functionName;
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
	if (isClassConstructor){
		isClassConstructor = false;
	}
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
	term();
	Token *token = tokenizer->getToken();
	if (token->type !=EQ && token->type != NOTEQ && token->type != LT && token->type != GT && token->type != LE && token->type != GE)
		return ;
	tokenizer->advance();
	term();
	switch (token->type){
	case EQ:byteCode->push_back(OP_EQ); break;
	case NOTEQ:byteCode->push_back(OP_NOTEQ); break;
	case LT:byteCode->push_back(OP_LT); break;
	case GT:byteCode->push_back(OP_GT); break;
	case LE:byteCode->push_back(OP_LE); break;
	case GE:byteCode->push_back(OP_GE); break;
	default:{
		tokenizer->error("Expecting relation operator");
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
	int index = stringPoolPtr->getStringConstant(str);
	if (index == -1){
		index = stringPoolPtr->putStringConstant(str);
	}
	assert(index>=0 && index<=SHRT_MAX);
	return index;
}

void Parser::classDefinition(){
	tokenizer->advance();
	std::string clsName = tokenizer->getToken()->str;
	int index = addSymbol();
	ClsType *cls = new ClsType;
	curClsType = cls;
	curClsType->clsName = clsName;
	isClass = true;
	classAttrs.clear();
	Object object;
	object.type = CLSTYPE;
	object.value.clsType = cls;
	symTab->putObj(index,object);
	clsIndex = index;
	match(LPAREN);
	Token* token = tokenizer->getToken();
	bool firstIden = true;
	while(token->type != RPAREN){
		if (firstIden){
			firstIden = false;
		}
		else{
			if (token->type != COMMA)
				break;
			tokenizer->advance();
			token = tokenizer->getToken();
		}
		match(IDEN);
		if (!symTab->isSymExistLocal(token->str)){
			tokenizer->error("class not found",0,SYMBOLERROR);
		}
		token = tokenizer->getToken();
	}
	match(RPAREN);

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

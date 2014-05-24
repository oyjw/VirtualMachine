#include "tokenizer.h"
#include "Parser.h"
#include "Exceptions.h"
#include "OpCode.h"
#include "SymbolTable.h"
#include "Object.h"
#include "StringPool.h"
#include <sstream>

void Parser::declarations(){
	match(VAR);
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
	if (symTab->isSymExistLocal(token->str)){
		tokenizer->error("symbol redefinition",token,SYMBOLERROR);
	}
	int n=symTab->putSym(token->str);
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
	byteCode->push_back(char(global ? STOREGLOBAL : STORELOCAL));
	pushWord(n);
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
			objCall();
			return;
		}
		if (nexttoken->type == LPAREN){
			functioncall();
			return;
		}
		auto pair = symTab->findSym(token->str);
		if (pair.second == -1){
			tokenizer->error("Symbol not found",token,SYMBOLERROR);
		}
		byteCode->push_back(char(pair.first ? PUSHGLOBAL : PUSHLOCAL));
		pushWord(pair.second);
		tokenizer->advance();
	}
	else if (token->type == LPAREN){
		tokenizer->advance();
		term();
		match(RPAREN);
		tokenizer->advance();
	}
	else if (token->type == STRING){
		int index = stringPoolPtr->putString(token->str);
		byteCode->push_back(PUSHSTRING);
		pushWord(index);
		tokenizer->advance();
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
	term();
	byteCode->push_back(RETCODE);
}


void Parser::function(){
	tokenizer->advance();
	Token *token = tokenizer->getToken();
	match(IDEN);
	
	if (symTab->isSymExistLocal(token->str)){
		tokenizer->error("symbol redefinition",token,SYMBOLERROR);
	}
	int index = symTab->putSym(token->str);
	match(LPAREN);
	token = tokenizer->getToken();
	int nargs = 0;
	symTab = std::make_shared<SymbolTable>(symTab,0);

	byteCodePtr = std::make_shared<ByteCode>(byteCodePtr);
	this->byteCode = &byteCodePtr->v;

	while (token->type == IDEN){
		if (nargs > 16){
			tokenizer->error("too many parameters");
		}
		if (symTab->isSymExistLocal(token->str)){
			tokenizer->error("symbol redifinition",token,SYMBOLERROR);
		}
		symTab->putSym(token->str);
		nargs++;
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
	obj->nargs = nargs;
	obj->bytes = std::move(byteCodePtr->v);
	Object objectHolder;
	objectHolder.type = FUNOBJ;
	objectHolder.value.funObj = obj;
	symTab = symTab->getNext();
	symTab->putObj(index , objectHolder);
	
	byteCodePtr = byteCodePtr->getNext();
	this->byteCode = &byteCodePtr->v;
}

void Parser::functioncall(){
	Token* token = tokenizer->getToken();
	Token* function = token;
	SymPtr sp=symTab;
	while (sp->getNext() != NULL){
		sp = sp->getNext();
	}
	
	auto p = sp->findSym(token->str);
	if (p.second == -1){
		tokenizer->error("unknown symbol", function, SYMBOLERROR);
	}
	assert(p.first);
	byteCode->push_back(PUSHGLOBAL);
	pushWord(p.second);

	tokenizer->advance();
	match(LPAREN);
	int nargs=funcArgs(function);
	
	match(RPAREN);
	byteCode->push_back((char)CALLFUNC);
	byteCode->push_back((char)nargs);
}

int Parser::funcArgs(Token* function){
	int nargs=0;
	Token* token = tokenizer->getToken();
	if (token->type!=RPAREN){
		while(1){
			if (nargs > 16){
				tokenizer->error("too many arguments", function);
			}
			term();
			nargs++;
			token = tokenizer->getToken();
			if(token->type!=COMMA)
				break;
			tokenizer->advance();
			token = tokenizer->getToken();
		}
	}
	return nargs;
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
	LabelList orLabel;
	LabelList andLabel;
	match(LPAREN);
	orExpr(orLabel,andLabel);
	match(RPAREN);
	byteCode->push_back((char)JMP);
	size_t pos = byteCode->size();
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

void Parser::orExpr(LabelList& orLabel,LabelList& andLabel){
	andExpr(orLabel,andLabel);
	orExpr2(orLabel,andLabel);
}

void Parser::orExpr2(LabelList& orLabel,LabelList& andLabel){
	Token* token = tokenizer->getToken();
	if (token->type == OR){
		match(OR);
		andExpr(orLabel,andLabel);
		orExpr2(orLabel,andLabel);
	}
}

void Parser::andExpr(LabelList& orLabel,LabelList& andLabel){
	LabelList andLabel2;
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

void Parser::andExpr2(LabelList& orLabel,LabelList& andLabel){
	Token* token = tokenizer->getToken();
	if (token->type == AND){
		match(AND);
		notExpr(orLabel,andLabel,false);
		andExpr2(orLabel,andLabel);
	}
}

void Parser::notExpr(LabelList& orLabel,LabelList& andLabel,bool notFlag){
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
		LabelList orLabel2;
		LabelList andLabel2;
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
	match(WHILE);
	if (loopLabelPtr == NULL){
		loopLabelPtr = std::make_shared<LoopLabel>();
	}
	else{
		loopLabelPtr = std::make_shared<LoopLabel>(loopLabelPtr);
	}
	loopLabelPtr->start=byteCode->size();
	LabelList orLabel;
	LabelList andLabel;
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
	match(BREAK);
	match(SEMICOLON);
	byteCode->push_back(JMP);
	loopLabelPtr->breaks.push_back(byteCode->size());
	pushWord(0);
}

void Parser::continueStmt(){
	match(CONTINUE);
	match(SEMICOLON);
	byteCode->push_back(JMP);
	loopLabelPtr->continues.push_back(byteCode->size());
	pushWord(0);
}

void Parser::objCall(){
	Token* token = tokenizer->getToken();
	tokenizer->advance(2);
	auto p = symTab->findSym(token->str);
	if (p.second == -1){
		auto iter = clsNames.find(token->str);
		if (iter == clsNames.end()){
			tokenizer->error("symbol not found",token,SYMBOLERROR);
		}
		ClsType& cls = clsData->at(iter->second);
		token = tokenizer->getToken();
		tokenizer->advance();
		Token* token2 = tokenizer->getToken();
		if (token2->type == LPAREN){
			auto iter2 = cls.methodMap.find(token->str);
			if (iter2 == cls.methodMap.end()){
				tokenizer->error("method not found",token,SYMBOLERROR);
			}
			if (!(iter2->second.second)){
				tokenizer->error("not static method",token,SYMBOLERROR);
			}
			byteCode->push_back((char)GETCLSMETHOD);
			pushWord(iter->second);
			byteCode->push_back(iter2->second.first);
			tokenizer->advance();
			int nargs = funcArgs(token);
			match(RPAREN);
			byteCode->push_back(CALLFUNC);
			byteCode->push_back((char)nargs);
		}
		else {
			auto iter2 = cls.fieldMap.find(token->str);
			if (iter2 == cls.fieldMap.end()){
				tokenizer->error("field not found",token,SYMBOLERROR);
			}
			if (!(iter2->second.second)){
				tokenizer->error("not static field",token,SYMBOLERROR);
			}
			byteCode->push_back((char)GETCLSFIELD);
			pushWord(iter->second);
			byteCode->push_back(iter2->second.first);
		}
	}

	token = tokenizer->getToken();
	match(IDEN);
	
	int index = stringPoolPtr->putStringConstant(token->str);
	byteCode->push_back(char(p.first?PUSHGLOBAL:PUSHLOCAL));
	pushWord(p.second);
	byteCode->push_back((char)GETATTR);
	byteCode->push_back((char)index);
	Token *token2 = tokenizer->getToken();
	if (token2->type == LPAREN){
		tokenizer->advance();
		int nargs = funcArgs(token);
		match(RPAREN);
		byteCode->push_back(CALLFUNC);
		byteCode->push_back((char)nargs);
	}

}


void Parser::newExpr(){
	tokenizer->advance();
	Token* token = tokenizer->getToken();
	auto iter = clsNames.find(token->str);
	if (iter == clsNames.end()){
		tokenizer->error("class not found",token,SYMBOLERROR);
	}
	byteCode->push_back((char)CREATEOBJ);
	pushWord(iter->second);
}

void Parser::classDefinition(){
	match(CLASS);
	Token* token = tokenizer->getToken();
	match(IDEN);
	auto iter = clsNames.find(token->str);
	if (iter != clsNames.end()){
		tokenizer->error("class redefinition",token,SYMBOLERROR);
	}
	int index = clsData->size();
	clsNames.insert(make_pair(token->str,index));
	ClsType cls;
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
	int nParents = 0;
	symTab = std::make_shared<SymbolTable>(symTab,0);

	match(LBRACE);
	token = tokenizer->getToken();
	while (token->type != RBRACE){
		if (token->type == IDEN){
			assignStmt();
		}
		else if (token->type == FUNCTION){
			function();
		}
		else {
			tokenizer->error("syntax error");
		}
	}
	tokenizer->advance();

	for (auto entry : symTab->map){
		cls.clsAttrs.insert(make_pair(entry.first, symTab->symVec[entry.second].obj));
	}
	symTab = symTab->getNext();
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
	code.word = (unsigned short)n;
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

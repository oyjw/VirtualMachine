#ifndef _PARSER_H_
#define _PARSER_H_
#include <vector>
#include <memory>

#define WORDSIZE 2
class SymbolTable;
typedef std::shared_ptr<SymbolTable> SymPtr;

class Tokenizer;
class StringPool;
class ByteCode;
typedef std::shared_ptr<ByteCode> ByteCodePtr;

typedef std::vector<std::pair<size_t,size_t>> LabelList;

struct LoopLabel;
typedef std::shared_ptr<LoopLabel> LoopLabelPtr;

class ByteCode{
private:
	ByteCodePtr next;
public:
	ByteCode() = default;
	ByteCode(ByteCodePtr n) :next(n) {}
	std::vector<char> v;
	void push(char code){
		v.push_back(code);
	}
	ByteCodePtr getNext(){
		return next;
	}
};


struct LoopLabel{
	size_t start;
	std::vector<size_t> breaks;
	std::vector<size_t> continues;
	LoopLabelPtr next;
	LoopLabel() = default;
	LoopLabel(LoopLabelPtr ptr) :next(ptr) {}
};

class Parser{
public:
	Parser(Tokenizer* t,SymPtr tab,std::shared_ptr<StringPool> sp,ByteCodePtr bcp):byteCodePtr(bcp),
		byteCode(&byteCodePtr->v),stringPoolPtr(sp) {
		symTab=tab;
		tokenizer=t;
	}
	~Parser() {
		delete tokenizer;
	}
	ByteCodePtr getByteCodePtr(){
		return byteCodePtr;
	}
	void declarations();
	void decl();
	int declList();
	void term();
	void term2();
	void factor();
	void factor2();
	void basic();
	
	void elem();
	void stmt();
	void stmts();
	void stmtList();
	void returnStmt();
	void printStmt();
	void assignStmt();
	void ifStmt();
	void whileStmt();
	void breakStmt();
	void continueStmt();

	void orExpr(LabelList& orLabel,LabelList& andLabel);
	void orExpr2(LabelList& orLabel,LabelList& andLabel);
	void andExpr(LabelList& orLabel,LabelList& andLabel);
	void andExpr2(LabelList& orLabel,LabelList& andLabel);
	void notExpr(LabelList& orLabel,LabelList& andLabel,bool notFlag);
	void relaExpr();
	void function();
	void program();
	void functioncall();

	void classDefinition();
private:
	void match(int type);
	void pushWord(int n);
	void setWord(std::vector<char>::size_type pos,int n);
	void pushFloat(float f);
	std::shared_ptr<StringPool> stringPoolPtr;
	ByteCodePtr byteCodePtr;
	std::vector<char> *byteCode;
	Tokenizer *tokenizer;
	SymPtr symTab;
	
	LoopLabelPtr loopLabelPtr;
};

#endif

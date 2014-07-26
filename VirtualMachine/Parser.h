#ifndef _PARSER_H_
#define _PARSER_H_
#include <vector>
#include <memory>
#include <unordered_set>
#include "Object.h"
#define WORDSIZE 2
class SymbolTable;
typedef std::shared_ptr<SymbolTable> SymPtr;
class Token;

class Tokenizer;
class ObjectPool;
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
	Parser(Tokenizer* t,SymPtr tab,std::shared_ptr<StringPool> sp,ByteCodePtr bcp,std::shared_ptr<ObjectPool> op):byteCodePtr(bcp),
		byteCode(&byteCodePtr->v),stringPoolPtr(sp),curClsType(NULL),isClass(false),isClassFunction(false),clsIndex(0),objectPoolPtr(op),
		tokenizer(t), symTab(tab), isClassConstructor(false), debugLine(0) {
	}
	~Parser() ;
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

	void orExpr();
	void orExpr2();
	void andExpr();
	void andExpr2();
	void notExpr(bool notFlag);
	void relaExpr();
	void function();
	void program();

	void funcArgs(Token* function);
	int parseSeleOp();
	void classDefinition();
	bool parseValue(int& type, int& index);
	void pushValue(int type, int index, bool isGlobal);
	void parseOp();
	void lvalue();
	void rvalue();
	std::pair<bool, int> parseIden(Token* token);
private:
	int addSymbol();
	int getBuiltInStr(const std::string& str);
	int getSharedString(const std::string& str);
	void match(int type);
	void pushWord(int n);
	void setWord(std::vector<char>::size_type pos,int n);
	void pushFloat(float f);
	std::shared_ptr<StringPool> stringPoolPtr;
	ByteCodePtr byteCodePtr;
	std::vector<char> *byteCode;
	Tokenizer *tokenizer;
	SymPtr symTab;
	//std::vector<ClsType> *clsData;
	ClsType* curClsType;
	std::unordered_set<std::string> classFunctions;
	bool isClass;
	bool isClassFunction;
	bool isClassConstructor;
	size_t clsIndex;
	int debugLine;
	int nListArgs;

	std::shared_ptr<ObjectPool> objectPoolPtr;
	std::unordered_map<std::string,int> sharedStrings;
	LoopLabelPtr loopLabelPtr;
};



#endif

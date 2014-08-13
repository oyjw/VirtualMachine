#ifndef _VIRTUAL_MACHINE_H
#define _VIRTUAL_MACHINE_H
#include <vector>
#include <memory>

#include "Parser.h"
class SymbolTable;
typedef std::shared_ptr<SymbolTable> SymPtr;
class StringPool;

struct Object;

struct CallInfo{
	int nArgs;
	std::string funcName;
	int line;
	std::shared_ptr<CallInfo> next;
	CallInfo() :line(0), nArgs(0) {}
	CallInfo(std::shared_ptr<CallInfo> n) : next(n), line(0), nArgs(0) {}
};

class VirtualMachine
{
public:
	VirtualMachine();
	~VirtualMachine() {}
	VirtualMachine(const VirtualMachine&) = delete;
	VirtualMachine& operator=(const VirtualMachine&) = delete;
	int execute(std::vector<char> &byteCodes, size_t base, size_t byteCodePos);
	void dump(std::vector<char> &byteCodes, std::ofstream& ofs);
	void run();
	void run(const std::string& fileName);
	bool pushFunObj(const std::string& symbol);
	void pushByteCode(char opcode){
		byteCodePtr->push(opcode);
	}
	void pushObject(const Object &obj){
		stack.push_back(obj);
		top++;
	}
	void getArgs(int* len, Object* objs){
		int tmp = top - framePointer;
		if (tmp > *len){
			*len = tmp;
			return;
		}
		for (int i = 0; i < tmp; ++i){
			objs[i] = stack[framePointer + i];
		}
	}
	void getArgs2(int* len, Object** objs){
		int tmp = top - framePointer;
		*len = tmp;
		*objs = new Object[*len];
		for (int i = 0; i < *len; ++i){
			(*objs)[i] = stack[framePointer + i];
		}
	}
	StrObj* addStrObj(const std::string& str);
	StrObj* getStrObj(const std::string& str);
	std::string getStackTrace();
	void checkArgs(int, int);
	void checkCallable(Object&);
	void throwError(const std::string msg, int type);
public:
	SymPtr symTab;
	std::shared_ptr<StringPool> stringPoolPtr;
	ByteCodePtr byteCodePtr;
	std::shared_ptr<ObjectPool> objectPoolPtr;
	ClsType* listCls;
	ClsType* dictCls;
	ClsType* strCls;
	Object callCFunc(ClsType* type, std::string funcName, int newBase);
	size_t getFP(){
		return framePointer;
	}
	void setFP(size_t fp){
		framePointer = fp;
	}
	int getTop(){
		return top;
	}
	void setTop(int t){
		top = t;
		stack.resize(top);
	}
private:
	void checkIndexType(Object& obj, Object& obj2);
	bool boolValue(Object& obj);
	void mark();
	void collect();
	void compute(int opcode);
	void compare(int opcode);
	int getWord(std::vector<char>& byteCode ,size_t &pos);
	float getFloat(std::vector<char>& byteCode ,size_t &pos);
	std::vector<Object> stack;
	int top;
	size_t framePointer;
	int nobjs;
	int threshold;
	std::shared_ptr<CallInfo> callInfoPtr;
};

#endif
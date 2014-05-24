#ifndef _VIRTUAL_MACHINE_H
#define _VIRTUAL_MACHINE_H
#include <vector>
#include <memory>

#include "Parser.h"
class SymbolTable;
typedef std::shared_ptr<SymbolTable> SymPtr;
class StringPool;

struct Object;

struct callInfo{
	int nargs;
	std::string funcName;
};
class VirtualMachine
{
public:
	VirtualMachine();
	VirtualMachine(std::vector<char>& b, SymPtr g,std::shared_ptr<StringPool> sp):symTab(g), top(0) ,stringPoolPtr(sp),threshold(100) {}
	~VirtualMachine() {}
	VirtualMachine(const VirtualMachine&) = delete;
	VirtualMachine& operator=(const VirtualMachine&) = delete;
	void execute(std::vector<char> &byteCodes,int base);
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
public:
	SymPtr symTab;
	std::shared_ptr<StringPool> stringPoolPtr;
	ByteCodePtr byteCodePtr;
	std::vector<clsType> clsData;
private:
	void collect();
	void compute(int opcode);
	void compare(int opcode);
	int getWord(std::vector<char>& byteCode ,size_t &pos);
	float getFloat(std::vector<char>& byteCode ,size_t &pos);
	std::vector<Object> stack;
	int base;
	int top;
	int nobjs;
	int threshold;

	
};

#endif
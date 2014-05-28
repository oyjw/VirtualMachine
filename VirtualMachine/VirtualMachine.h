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
	~VirtualMachine() {}
	VirtualMachine(const VirtualMachine&) = delete;
	VirtualMachine& operator=(const VirtualMachine&) = delete;
	int execute(std::vector<char> &byteCodes, int base, size_t byteCodePos);
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
	std::shared_ptr<ObjectPool> objectPoolPtr;
private:
	void collect();
	void compute(int opcode);
	void compare(int opcode);
	int getWord(std::vector<char>& byteCode ,size_t &pos);
	float getFloat(std::vector<char>& byteCode ,size_t &pos);
	std::vector<Object> stack;
	int top;
	size_t pos;
	int nobjs;
	int threshold;

};

#endif
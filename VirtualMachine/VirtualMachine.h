#ifndef _VIRTUAL_MACHINE_H
#define _VIRTUAL_MACHINE_H
#include <vector>
#include "Object.h"
#include "SymbolTable.h"


class VirtualMachine
{
public:
	VirtualMachine(std::vector<char>& b, SymPtr g) :byteCode(b), symTab(g), pos(0), top(0), base(0) {}
	~VirtualMachine() {
	}
	void exectue(std::vector<char> &byteCodes);
	void dump(std::vector<char> &byteCodes, const std::string& fileName);
	void run() {
		dump(byteCode, "2.txt");
	}
private:
	int getWord(){
		CodeWord code;
		code.c.c1 = byteCode[pos++];
		code.c.c2 = byteCode[pos++];
		return code.word;
	}
	float getFloat(){
		CodeFloat code;
		code.c.c1 = byteCode[pos++];
		code.c.c2 = byteCode[pos++];
		code.c.c3 = byteCode[pos++];
		code.c.c4 = byteCode[pos++];
		return code.f;
	}
	std::vector<char>& byteCode;
	int pos;
	std::vector<Object> stack;
	SymPtr symTab;
	int top;
	int base;
};

#endif
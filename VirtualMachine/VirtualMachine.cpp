#include "VirtualMachine.h"
#include "OpCode.h"
#include "Object.h"
#include <cassert>
#include <fstream>
#include <sstream>
#include <iostream>

void VirtualMachine::execute(std::vector<char>& byteCodes,int base){
	int pos = 0;
	while (pos <byteCodes.size()){
		switch (byteCodes[pos++]){
		case PUSHLOCAL:{
			int n = getWord(byteCodes,pos);
			stack.push_back(stack[base + n]);
			top++;
			break;
		}
		case PUSHGLOBAL:{			
			int n = getWord(byteCodes,pos);
			stack.push_back(symTab->getObj(n));
			top++;
			break;
		}
		case STORELOCAL:{
			int n = getWord(byteCodes,pos);
			stack[base + n] = stack[top-1];
			break;
		}
		case STOREGLOBAL:{
			int n = getWord(byteCodes,pos);
			symTab->putObj(n,stack[top-1]);
			top--;
			stack.resize(top);
			break;
		}
		case PUSHREAL:{
			int f = getFloat(byteCodes,pos);
			Object obj;
			obj.type=NUMOBJ;
			obj.value.numval=f;
			stack.push_back(obj);
			top++;
			break;
		}
		case OP_ADD:{
			Object& l = stack[top-2];
			Object& r = stack[top-1];
			if (l.type == NUMOBJ && r.type == NUMOBJ){
				float result=l.value.numval+r.value.numval;
				l.value.numval=result;
			}
			else if (l.type == STROBJ && r.type==NUMOBJ || 
					l.type == NUMOBJ && r.type == STROBJ ||
					l.type==STROBJ && r.type==STROBJ){
			
			}
			top--;
			stack.resize(top);
			break;
		}
		case OP_SUB:{
			Object& l = stack[top - 2];
			Object& r = stack[top - 1];
			if (l.type == NUMOBJ && r.type == NUMOBJ){
				float result = l.value.numval - r.value.numval;
				l.value.numval = result;
			}
			else if (l.type == STROBJ && r.type == NUMOBJ ||
				l.type == NUMOBJ && r.type == STROBJ ||
				l.type == STROBJ && r.type == STROBJ){

			}
			top--;
			stack.resize(top);
			break;
		}
		case OP_MUL:{
			Object& l = stack[top - 2];
			Object& r = stack[top - 1];
			if (l.type == NUMOBJ && r.type == NUMOBJ){
				float result = l.value.numval * r.value.numval;
				l.value.numval = result;
			}
			else if (l.type == STROBJ && r.type == NUMOBJ ||
				l.type == NUMOBJ && r.type == STROBJ ||
				l.type == STROBJ && r.type == STROBJ){

			}
			top--;
			stack.resize(top);
			break;
		}

		case OP_DIV:{
			Object& l = stack[top - 2];
			Object& r = stack[top - 1];
			if (l.type == NUMOBJ && r.type == NUMOBJ){
				float result = l.value.numval / r.value.numval;
				l.value.numval = result;
			}
			else if (l.type == STROBJ && r.type == NUMOBJ ||
				l.type == NUMOBJ && r.type == STROBJ ||
				l.type == STROBJ && r.type == STROBJ){

			}
			top--;
			stack.resize(top);
			break;
		}
		case CALLFUNC:{
			int nargs = byteCodes[pos++];
			Object &obj = stack[ top - nargs - 1 ];
			int newBase = top - nargs;
			assert(obj.type == FUNOBJ);
			execute(obj.value.funObj->bytes,newBase);
			break;
		}
		case RETCODE:{
			stack[base - 1] = stack[top-1];
			top--;
			stack.resize(top);
			break;
		}
		case PRINTFUNC:{
			int n = getWord(byteCodes,pos);
			Object& obj = symTab->getObj(n);
			if (obj.type == NUMOBJ)
				std::cout << symTab->getObj(n).value.numval <<std::endl;
			system("pause");
			break;
		}
		default:assert(0);
		}
	}
}

void VirtualMachine::dump(std::vector<char> &byteCodes,std::ofstream& ofs){
	int pos = 0;
	while (pos <byteCodes.size()){
		switch (byteCodes[pos++]){
		case PUSHLOCAL:{
			int n = getWord(byteCodes,pos);
			ofs << "\tPUSHLOCAL\t" << n <<std::endl;
			break;
		}
		case PUSHGLOBAL:{
			int n = getWord(byteCodes,pos);
			ofs << "\tPUSHGLOBAL\t" << n << std::endl;
			break;
		}
		case STORELOCAL:{
			int n = getWord(byteCodes,pos);
			ofs << "\tSTORELOCAL\t" << n << std::endl;
			break;
		}
		case STOREGLOBAL:{
			int n = getWord(byteCodes,pos);
			ofs << "\tSTOREGLOBAL\t" << n << std::endl;
			break;
		}
		case PUSHREAL:{
			int f = getFloat(byteCodes,pos);
			ofs << "\tPUSHREAL\t" << f << std::endl;
			break;
		}
		case OP_ADD:{
			ofs << "\tOP_ADD\t" << std::endl;
			break;
		}
		case OP_SUB:{
			ofs << "\tOP_SUB\t" << std::endl;
			break;
		}
		case OP_MUL:{
			ofs << "\tOP_MUL\t" << std::endl;
			break;
		}

		case OP_DIV:{
			ofs << "\tOP_DIV\t" << std::endl;
			break;
		}
		case CALLFUNC:{
			int nargs = byteCodes[pos++];
			ofs << "\tCALLFUNC\t" << nargs <<std::endl;			
			break;
		}
		case RETCODE:{
			ofs << "\tRETCODE\t" <<std::endl;
			break;
		}
		case PRINTFUNC:{
			int n = getWord(byteCodes,pos);
			ofs << "\tPRINT\t" << n << std::endl;
			break;
		}
		default:assert(0);
		}
	}
}
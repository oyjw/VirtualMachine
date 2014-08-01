#include "VirtualMachine.h"
#include "OpCode.h"
#include "Object.h"
#include "Exceptions.h"
#include "SymbolTable.h"
#include "StringPool.h"
#include "slang.h"
#include "Tokenizer.h"
#include "Parser.h"
#include "ObjectPool.h"
#include "List.h"
#include <cassert>
#include <fstream>
#include <sstream>
#include <iostream>


VirtualMachine::VirtualMachine():threshold(100), nobjs(0), top(0), framePointer(0), symTab(new SymbolTable), byteCodePtr(new ByteCode),
	stringPoolPtr(new StringPool), objectPoolPtr(new ObjectPool), callInfoPtr(new CallInfo), listCls(NULL), dictCls(NULL), strCls(NULL) {
	callInfoPtr->funcName = "main";
}

void VirtualMachine::checkCallable(Object& obj){
	std::string msg;
	if (obj.type != CLSTYPE && !(obj.type & METHOD) && obj.type != FUNOBJ && obj.type != CFUNOBJ && obj.type != USERTYPE ){
		throwError("not callable",TYPEERROR);
	}
}

void VirtualMachine::checkArgs(int actualNArgs, int nArgs){
	if (nArgs == ANYARG) 
		return;
	char buf[100];
	std::string msg;
	if (actualNArgs < nArgs){
		msg = std::to_string(nArgs - actualNArgs);
		msg = "missing " + msg + " arguments";
		throwError(msg, ARGUMENTERROR);
	}
	else if (nArgs < actualNArgs){
		msg = std::to_string(actualNArgs - nArgs);
		msg = "excess number of arguments: " + msg +" given";
		throwError(msg, ARGUMENTERROR);
	}
}

void printFunc(Object& obj){
	switch (obj.type){
		case NUMOBJ:std::cout << (int)obj.value.numval << std::endl; break;
		case NILOBJ:std::cout << "None" <<std::endl; break;
		case CLSOBJ:{
			std::cout << obj.value.clsObj->clsType->clsName << " object" <<std::endl; break;
		}
		case CLSTYPE:{
			std::cout << "class " << obj.value.clsType->clsName<<std::endl; break;
		}
		case FUNOBJ:{

		}
		case CFUNOBJ:{

		}
		default:{
			if (obj.type & METHOD && obj.type & FUNOBJ){

			}
			else if (obj.type & METHOD && obj.type & CFUNOBJ){

			}
			else{
						
			}
		}
	}
}

int VirtualMachine::execute(std::vector<char>& byteCodes,size_t base,size_t byteCodePos){
	while (byteCodePos <byteCodes.size()){
		switch (byteCodes[byteCodePos++]){
			case PUSHLOCAL:{
				int n = getWord(byteCodes,byteCodePos);
				stack.push_back(stack[base + n]);
				top++;
				break;
			}
			case PUSHGLOBAL:{			
				int n = getWord(byteCodes,byteCodePos);
				stack.push_back(symTab->getObj(n));
				top++;
				break;
			}
			case STORELOCAL:{
				int n = getWord(byteCodes,byteCodePos);
				stack[base + n] = stack[top-1];
				top--;
				stack.resize(top);
				break;
			}
			case STOREGLOBAL:{
				int n = getWord(byteCodes,byteCodePos);
				symTab->putObj(n,stack[top-1]);
				top--;
				stack.resize(top);
				break;
			}
			case PUSHREAL:{
				float f = getFloat(byteCodes,byteCodePos);
				Object obj;
				obj.type=NUMOBJ;
				obj.value.numval=f;
				stack.push_back(obj);
				top++;
				break;
			}
			case PUSHSTRING:{
				int index = getWord(byteCodes,byteCodePos);
				Object obj;
				obj.type = STROBJ;
				obj.value.strObj = stringPoolPtr->getStrObj(index);
				stack.push_back(obj);
				top++;
				int newBase = top - 1;
				nobjs++;
				collect();
				stack[top-1] = callCFunc(this->strCls, "constructor", newBase);
				break;
			}
			case OP_ADD:{
				Object& l = stack[top-2];
				Object& r = stack[top-1];
				if (l.type == NUMOBJ && r.type == NUMOBJ){
					float result=l.value.numval+r.value.numval;
					l.value.numval=result;
				}
				else if (l.type & STROBJ && r.type & STROBJ){
					nobjs++;
					collect();
					std::string& leftStr = ((StrObj*)(l.value.userData->data))->str;
					std::string& rightStr = ((StrObj*)(r.value.userData->data))->str;
					l.value.userData->data = stringPoolPtr->putString(leftStr+rightStr);
				}
				else{
					throwError("operands don't support + operator",TYPEERROR);
				}
				top--;
				stack.resize(top);
				break;
			}
			case OP_SUB:{
				compute(OP_SUB);
				break;
			}
			case OP_MUL:{
				compute(OP_MUL);
				break;
			}
			case OP_DIV:{
				compute(OP_DIV);
				break;
			}
			case OP_EQ:{
				compare(OP_EQ);
				break;
			}
			case OP_NOTEQ:{
				compare(OP_NOTEQ);
				break;
			}
			case OP_LT:{		
				compare(OP_LT);
				break;
			}
			case OP_GT:{
				compare(OP_GT);
				break;
			}
			case OP_LE:{
				compare(OP_LE);
				break;
			}
			case OP_GE:{
				compare(OP_GE);
				break;
			}
			case OP_OR:{
				Object& l = stack[top - 2];
				Object& r = stack[top - 1];
				if (!boolValue(l)){
					if (boolValue(r)){
						l.value.boolval = true;
					}
					else {
						l.value.boolval = false;
					}
					l.type = BOOLOBJ;
				}
				top--;
				stack.resize(top);
				break;
			}
			case OP_AND:{
				Object& l = stack[top - 2];
				Object& r = stack[top - 1];
				if (boolValue(l) && boolValue(r)){
					l.value.boolval = true;
				}		
				else {
					l.value.boolval = false;
				}
				l.type = BOOLOBJ;	
				top--;
				stack.resize(top);
				break;
			}
			case OP_NOT:{
				Object& obj=stack[top-1];
				if (boolValue(obj)){
					obj.value.boolval = true;
				}		
				else {
					obj.value.boolval = false;
				}
				obj.type = BOOLOBJ;	
				break;
			}
			case JMP:{
				int steps = getWord(byteCodes,byteCodePos);
				byteCodePos+=steps;		
				break;
			}
			case JMPIFF:{
				Object& obj=stack[top-1];
				int steps = getWord(byteCodes,byteCodePos);
				if (obj.type == NUMOBJ && obj.value.numval==0  ||  obj.type==NILOBJ  ||
					obj.type == BOOLOBJ && obj.value.boolval==false){
					byteCodePos+=steps;	
				}		
				top--;
				stack.resize(top);
				break;
			}
			case ADJUST:{
				int nDecls = byteCodes[byteCodePos++];
				top = top+nDecls;
				if (nDecls > 0){
					Object obj={NILOBJ,0};
					for(int i=0;i<nDecls;++i){
						stack.push_back(obj);
					}		
				}
				else if (nDecls < 0){
					stack.resize(top);
				}
				break;
			}
			case CALLFUNC:{
				int nargs = byteCodes[byteCodePos++];
				Object &obj = stack[ top - nargs - 1 ];
				checkCallable(obj);
				int newBase = top - nargs;
  				if (obj.type == USERTYPE){
					nobjs++;
					collect();
					obj = callCFunc(obj.value.userData->type, "constructor", newBase);
					top = newBase;
					stack.resize(top);
					break;
				} 
				else if (obj.type == CLSTYPE){
 					nobjs++;
					collect();
					Object newObj;
					newObj.type = CLSOBJ;
					ClsObj *clsObj = new ClsObj;
					newObj.value.clsObj = clsObj;
					objectPoolPtr->putClsObj(clsObj);
					clsObj->clsType = obj.value.clsType;
					newObj.value.clsObj = clsObj;
					obj = newObj;
					StrObj* strObj = getStrObj("__init__");
					auto& map = newObj.value.clsObj->clsType->clsAttrs;
					auto iter = map.find(strObj);
					if (iter != map.end()){
						callInfoPtr = std::make_shared<CallInfo>(callInfoPtr);
						int base = top - nargs - 1;
						if (iter->second.type == FUNOBJ){
							checkArgs(top - base, iter->second.value.funObj->nArgs);
							callInfoPtr->funcName = iter->second.value.funObj->functionName;
							execute(iter->second.value.funObj->bytes, base, 0);
						}
						else{
							checkArgs(top - base, iter->second.value.cFunObj->nArgs);
							callInfoPtr->funcName = iter->second.value.cFunObj->functionName;
							framePointer = base;
							iter->second.value.cFunObj->fun((void*)this);
						}
						callInfoPtr = callInfoPtr->next;
					}
					top = newBase;
					stack.resize(top);
					break;
				}
				callInfoPtr = std::make_shared<CallInfo>(callInfoPtr);
				int base = 0;
				Object func = obj;
				if (obj.type & METHOD){
					base = newBase - 1;
					callInfoPtr->funcName = obj.type & FUNOBJ ? obj.value.method.funObj->functionName:
						obj.value.method.cFunObj->functionName;
					obj.type = CLSOBJ;
					obj.value.clsObj = obj.value.method.self;
				}
				else{
					base = newBase;
					callInfoPtr->funcName = obj.type & FUNOBJ ? obj.value.funObj->functionName:
						obj.value.cFunObj->functionName;
				}
				if (func.type & FUNOBJ){
					FunObj* function = func.type& METHOD?func.value.method.funObj: func.value.funObj;
					checkArgs(top - base, function->nArgs);
 					int nResult = execute(function->bytes, base, 0);
					if (nResult == 0){
						stack[newBase - 1].type = NILOBJ;
					}
					else{
						stack[newBase - 1] = stack[top-1];
					}
				}
				else if (func.type & CFUNOBJ){
					CFunObj* cFunction = func.type& METHOD?func.value.method.cFunObj: func.value.cFunObj;
					checkArgs(top - base, cFunction->nArgs);
					framePointer = base;
					Object result = cFunction->fun((void*)this);
					stack[newBase - 1] = result;
				}
				else assert(0);
				callInfoPtr = callInfoPtr->next;
				
				top = newBase;
				stack.resize(top);
				
				break;
			}
			case RETCODE:{
				return 1;
			}
			case RET0:{
				return 0;
			}
			case SETLINE:{
				int line = getWord(byteCodes, byteCodePos);
				callInfoPtr->line = line;
				break;
			}
			case PRINTFUNC:{
				Object& obj = stack[top-1];
				printFunc(obj);
				top--;
				stack.resize(top);
				system("pause");
				break;
			}
			case GETATTR:{
				Object &obj = stack[top-2];
				Object &sobj = stack[top-1];
				if (obj.type != CLSTYPE && obj.type != CLSOBJ && obj.type != USERTYPE && !(obj.type & USEROBJ)){
					throwError("target doesn't support getattr",TYPEERROR);
				}
				assert(sobj.type & STROBJ);
				StrObj* strObj = ((StrObj*)sobj.value.userData->data);
				std::unordered_map<StrObj*,Object,decltype(strHasher)*,decltype(strEq)*>::iterator iter;
				bool createMethod = false;
				if (obj.type == CLSTYPE || obj.type == USERTYPE){
					auto& map = obj.value.clsType->clsAttrs;
					iter = map.find(strObj);
					if (iter == map.end()){
						throwError("class doesn't have such attribute",TYPEERROR);
					}
				}
				else{
					bool findClsAttr = false;
					std::unordered_map<StrObj*,Object,decltype(strHasher)*,decltype(strEq)*>* map;
					if (obj.type == CLSOBJ){
						map = &(obj.value.clsObj->attrs);
						iter = map->find(strObj);
						if (iter == map->end()){
							findClsAttr = true;
						}
					}
					else
						findClsAttr = true;
					if (findClsAttr){
						map = obj.type == CLSOBJ?&(obj.value.clsObj->clsType->clsAttrs):&(obj.value.userData->type->clsAttrs);
						iter = map->find(strObj);
						if (iter == map->end()){
							throwError("object doesn't have such attribute",TYPEERROR);
						}
						else{
							if (iter->second.type == FUNOBJ || iter->second.type == CFUNOBJ)
								createMethod = true;
						}	
					}
				}
				if (createMethod){
					Object method;
					method.value.method.self = obj.value.clsObj;
					if (iter->second.type == FUNOBJ){
						method.value.method.funObj = iter->second.value.funObj;
						method.type = METHOD | FUNOBJ;
					}
					else {
						method.value.method.cFunObj = iter->second.value.cFunObj;
						method.type = METHOD | CFUNOBJ;
					}
					obj = method;
				}
				else
					obj = iter->second;
				top--;
				stack.resize(top);
				break;
			}
			case SETATTR:{
				Object &obj = stack[top-3];
				Object &sobj = stack[top-2];
				Object &value = stack[top-1];
				if (obj.type != CLSTYPE && obj.type != CLSOBJ && obj.type != USERTYPE){
					throwError("target does't support setattr",TYPEERROR);
				}
				assert(sobj.type & STROBJ);
				auto &map = 
					(obj.type == CLSTYPE || obj.type == USERTYPE)? obj.value.clsType->clsAttrs:obj.value.clsObj->attrs;
				StrObj* strObj = ((StrObj*)sobj.value.userData->data);
				auto p = map.insert(std::make_pair(strObj,value));
				if (!p.second ){
					map.erase(p.first);
					map.insert(p.first,std::make_pair(strObj,value));
				}
				top = top - 3;
				stack.resize(top);
				break;
			}
			case GETINDEX:{
				Object &obj = stack[top-2];
				Object &obj2 = stack[top-1];
				checkIndexType(obj,obj2);

				int newBase = top - 2;
				obj = callCFunc(this->listCls, "[]", newBase);
				top = newBase + 1;
				stack.resize(top);
				break;
			}
			case SETINDEX:{
				Object &obj = stack[top-3];
				Object &obj2 = stack[top-2];
				Object &value = stack[top-1];
				checkIndexType(obj,obj2);
				int newBase = top - 3;
				obj = callCFunc(this->listCls, "[]=", newBase);
				top = newBase;
				stack.resize(top);
				break;
			}
			default:assert(0);
		}
	}
	return 0;
}

void VirtualMachine::checkIndexType(Object& obj, Object& obj2){
	if (!(obj.type & USEROBJ) && !(obj.type & LISTOBJ)){
		throwError("target doesn't support getindex",TYPEERROR);
	}
	assert(obj2.type & STROBJ || obj2.type == NUMOBJ);
	if (obj.type & LISTOBJ){
		if(obj2.type != NUMOBJ)
			throwError("index type error",TYPEERROR);
	}
}

Object VirtualMachine::callCFunc(ClsType* type, std::string funcName, int newBase){
	Object newObj;
	StrObj* strObj = getStrObj(funcName);
	auto& map = type->clsAttrs;
	auto iter = map.find(strObj);
	assert(iter != map.end() && iter->second.type == CFUNOBJ);
	callInfoPtr = std::make_shared<CallInfo>(callInfoPtr);
 	checkArgs(top - newBase, iter->second.value.cFunObj->nArgs);
	callInfoPtr->funcName = iter->second.value.cFunObj->functionName;
	framePointer = newBase;
	newObj = iter->second.value.cFunObj->fun((void*)this);
	callInfoPtr = callInfoPtr->next;
	return newObj;
}

bool VirtualMachine::boolValue(Object& obj){
	if (obj.type == NUMOBJ && obj.value.numval==0  ||  obj.type==NILOBJ  ||
		obj.type == BOOLOBJ && obj.value.boolval==false ||
		obj.type & STROBJ && ((StrObj*)obj.value.userData->data)->str==""){
		return false;
	}		
	else {
		return true;
	}
}

void VirtualMachine::compute(int opcode){
	Object& l = stack[top - 2];
	Object& r = stack[top - 1];
	if (l.type != NUMOBJ || r.type != NUMOBJ){
		std::ostringstream oss;
		throw TypeError(oss.str());
	}
	float result = 0.0;
	switch (opcode){
		case OP_SUB:result=l.value.numval - r.value.numval; break;
		case OP_MUL:result=l.value.numval * r.value.numval; break;
		case OP_DIV:result=l.value.numval / r.value.numval; break;
		default:assert(0);
	}
	l.value.numval=result;
	top--;
	stack.resize(top);
}

void VirtualMachine::compare(int opcode){
	Object& l = stack[top - 2];
	Object& r = stack[top - 1];
	if (l.type != NUMOBJ || r.type != NUMOBJ){
		std::ostringstream oss;
		throw TypeError(oss.str());
	}
	bool b = false;
	switch (opcode){
		case OP_EQ:b=l.value.numval == r.value.numval; break;
		case OP_NOTEQ:b=l.value.numval != r.value.numval; break;
		case OP_LT:b=l.value.numval < r.value.numval; break;
		case OP_GT:b=l.value.numval > r.value.numval; break;
		case OP_LE:b=l.value.numval <= r.value.numval; break;
		case OP_GE:b=l.value.numval >= r.value.numval; break;
		default:assert(0);
	}
	l.type = BOOLOBJ;
	l.value.boolval=b;
	top--;
	stack.resize(top);
}

void markObject(Object& obj){
	if (obj.type & USEROBJ){
		obj.value.userData->mark = true;
		if (obj.type & LISTOBJ){
			List* list = (List*)obj.value.userData->data;
			for (auto& obj : list->vec){
				markObject(obj);
			}
		}
		else if (obj.type & STROBJ){
			StrObj* strObj = (StrObj*)obj.value.userData->data;
			strObj->mark = true;
		}
	}
	else if (obj.type == CLSOBJ){
		obj.value.clsObj->mark = true;
		for (auto& pair : obj.value.clsObj->attrs){
			markObject(pair.second);
		}
	}
}

void VirtualMachine::mark(){
	std::vector<Symbol>& symbols = this->symTab->getSymbols();
	for (auto &symbol : symbols){
		markObject(symbol.obj);
	}
	for (auto& obj : stack){
		markObject(obj);
	}
}

void VirtualMachine::collect(){
	if (nobjs <= threshold)
		return;
	mark();
	stringPoolPtr->collect();
	objectPoolPtr->collect();
	nobjs = stringPoolPtr->getStrNum() + (int)objectPoolPtr->getObjNum();
	threshold = nobjs*2;
}

void VirtualMachine::dump(std::vector<char> &byteCodes,std::ofstream& ofs){
	size_t byteCodePos = 0;
	while (byteCodePos <byteCodes.size()){
		switch (byteCodes[byteCodePos++]){
			case PUSHLOCAL:{
				ofs << byteCodePos-1 ;
				int n = getWord(byteCodes,byteCodePos);
				ofs << "\tPUSHLOCAL\t" << n <<std::endl;
				break;
			}
			case PUSHGLOBAL:{
				ofs << byteCodePos-1;
				int n = getWord(byteCodes,byteCodePos);
				ofs << "\tPUSHGLOBAL\t" << n << std::endl;
				break;
			}
			case STORELOCAL:{
				ofs << byteCodePos-1;
				int n = getWord(byteCodes,byteCodePos);
				ofs << "\tSTORELOCAL\t" << n << std::endl;
				break;
			}
			case STOREGLOBAL:{
				ofs << byteCodePos-1;
				int n = getWord(byteCodes,byteCodePos);
				ofs << "\tSTOREGLOBAL\t" << n << std::endl;
				break;
			}
			case PUSHREAL:{
				ofs << byteCodePos-1;
				float f = getFloat(byteCodes,byteCodePos);
				ofs << "\tPUSHREAL\t" << f << std::endl;
				break;
			}
			case PUSHSTRING:{
				ofs << byteCodePos-1 ;
				int n = getWord(byteCodes,byteCodePos);
				ofs << "\tPUSHSTRING\t" << n <<std::endl;
				break;
			}
			case OP_ADD:{
				ofs << byteCodePos-1 << "\tOP_ADD\t" << std::endl;
				break;
			}
			case OP_SUB:{
				ofs << byteCodePos-1 << "\tOP_SUB\t" << std::endl;
				break;
			}
			case OP_MUL:{
				ofs << byteCodePos-1 << "\tOP_MUL\t" << std::endl;
				break;
			}

			case OP_DIV:{
				ofs << byteCodePos-1 << "\tOP_DIV\t" << std::endl;
				break;
			}
			case OP_EQ:{
				ofs << byteCodePos-1 << "\tOP_EQ\t"  <<std::endl;
				break;
			}
			case OP_NOTEQ:{
				ofs << byteCodePos-1 << "\tOP_NOTEQ\t"  <<std::endl;
				break;
			}
			case OP_LT:{		
				ofs << byteCodePos-1 << "\tOP_LT\t"  <<std::endl;
				break;
			}
			case OP_GT:{
				ofs << byteCodePos-1 << "\tOP_GT\t"  <<std::endl;
				break;
			}
			case OP_LE:{
				ofs << byteCodePos-1 << "\tOP_LE\t"  <<std::endl;
				break;
			}
			case OP_GE:{
				ofs << byteCodePos-1 << "\tOP_GE\t"  <<std::endl;
				break;
			}
			case OP_OR:{
				ofs << byteCodePos-1 << "\tOP_OR\t"  <<std::endl;
				break;
			}
			case OP_AND:{
				ofs << byteCodePos-1 << "\tOP_AND\t"  <<std::endl;
				break;
			}
			case OP_NOT:{
				ofs << byteCodePos-1 << "\tOP_NOT\t"  <<std::endl;	
				break;
			}
			case JMP:{
				ofs << byteCodePos-1;
				int steps = getWord(byteCodes,byteCodePos);
				ofs << "\tJMP\t" << steps <<std::endl;			
				break;
			}
			case JMPIFF:{
				ofs << byteCodePos-1;
				int steps = getWord(byteCodes,byteCodePos);
				ofs << "\tJMPIFF\t" << steps <<std::endl;			
				break;
			}
			case ADJUST:{
				ofs << byteCodePos-1;
				int nDecls = byteCodes[byteCodePos++];
				ofs << "\tADJUST\t" << nDecls <<std::endl;			
				break;
			}
			case CALLFUNC:{
				ofs << byteCodePos-1;
				int nargs = byteCodes[byteCodePos++];
				ofs << "\tCALLFUNC\t" << nargs <<std::endl;			
				break;
			}
			case RETCODE:{
				ofs << byteCodePos-1 << "\tRETCODE\t" <<std::endl;
				break;
			}
			case RET0:{
				ofs << byteCodePos-1 << "\tRET0\t" <<std::endl;
				break;
			}
			case SETLINE:{
				ofs << byteCodePos-1 ;
				int n = getWord(byteCodes,byteCodePos);
				ofs << "\tSETLINE\t" << n << std::endl;
				break;
			}
			case PRINTFUNC:{
				ofs << byteCodePos-1 << "\tPRINT\t"  << std::endl;
				break;
			}
			case GETATTR:{
				ofs << byteCodePos-1 << "\tGETATTR\t"  << std::endl;
				break;
			}
			case SETATTR:{
				ofs << byteCodePos-1 << "\tSETATTR\t"  << std::endl;
				break;
			}
			case GETINDEX:{
				ofs << byteCodePos-1 << "\tGETINDEX\t"  << std::endl;
				break;
			}
			case SETINDEX:{
				ofs << byteCodePos-1 << "\tSETINDEX\t"  << std::endl;
				break;
			}
			default:assert(0);
			}
	}
}

void VirtualMachine::run(){
	execute(byteCodePtr->v, 0, 0);
}

void VirtualMachine::run(const std::string& fileName) {
	std::ofstream ofs(fileName);
	if (!ofs.is_open()){
		std::ostringstream oss;
		oss << "file open fail: " << fileName << std::endl;
		throw std::runtime_error(oss.str());
	}
	dump(byteCodePtr->v, ofs);
	for (auto& symbol : symTab->getSymbols()){
		if (symbol.obj.type == FUNOBJ){
				
			ofs << symbol.objName << ":" <<std::endl;
			std::vector<char> & v=symbol.obj.value.funObj->bytes;
			for (auto& c : v){
				std::cout << (int)c <<std::endl;
			}
			dump(symbol.obj.value.funObj->bytes,ofs);
		}
		else if (symbol.obj.type == CLSTYPE){	
			ofs << "class " << symbol.objName << ":" <<std::endl;
			auto& map = symbol.obj.value.clsType->clsAttrs;
			for (auto iter = map.begin(); iter != map.end(); ++iter){
				if (iter->second.type == FUNOBJ){
					ofs << symbol.objName << ":" <<std::endl;
					dump(iter->second.value.funObj->bytes,ofs);
				}
			}
		}
	}
}

int VirtualMachine::getWord(std::vector<char>& byteCode ,size_t &byteCodePos){
	CodeWord code;
	code.c.c1 = byteCode[byteCodePos++];
	code.c.c2 = byteCode[byteCodePos++];
	return code.word;
}

float VirtualMachine::getFloat(std::vector<char>& byteCode ,size_t &byteCodePos){
	CodeFloat code;
	code.c.c1 = byteCode[byteCodePos++];
	code.c.c2 = byteCode[byteCodePos++];
	code.c.c3 = byteCode[byteCodePos++];
	code.c.c4 = byteCode[byteCodePos++];
	return code.f;
}

bool VirtualMachine::pushFunObj(const std::string& symbol){
	auto p = symTab->findSym(symbol);
	if (p.second == -1){
		return false;
	}
	stack.push_back(symTab->getObj(p.second));
	top++;
	return true;
}

StrObj* VirtualMachine::addStrObj(const std::string& str){
	int index = stringPoolPtr->getStringConstant(str);
	if (index == -1){
		return stringPoolPtr->putString(str);
	}
	return stringPoolPtr->getStrObj(index);
}

StrObj* VirtualMachine::getStrObj(const std::string& str){
	int index = stringPoolPtr->getStringConstant(str);
	assert(index != -1);
	return stringPoolPtr->getStrObj(index);
}

std::string VirtualMachine::getStackTrace(){
	std::shared_ptr<CallInfo> tmp = callInfoPtr;
	std::ostringstream oss;
	while (tmp){
		oss << tmp->funcName << "\t";
		if(tmp->line != 0)
			oss<<"line:" << tmp->line <<std::endl;
		else
			oss<<std::endl;
		tmp = tmp->next;
	}
	return oss.str();
}

const char* emsg[] = {
	"TypeError", "AttrError", "ArgumentError"
};

void VirtualMachine::throwError(const std::string msg, int type){
	std::shared_ptr<CallInfo> tmp = callInfoPtr;
	std::ostringstream oss;
	oss << msg << "\t" << emsg[type] << std::endl;
	oss << getStackTrace();
	throw std::runtime_error(oss.str());
}
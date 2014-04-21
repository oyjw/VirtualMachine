#include "VirtualMachine.h"
#include "OpCode.h"
#include "Object.h"

void VirtualMachine::exectue(){
	char code = byteCode[pos++];
	switch (code){
	case PUSHLOCAL:{
		int n = getWord();
		stack.push_back(stack[base + n]);
		break;
	}
	case PUSHGLOBAL:{			
		int n = getWord();
		stack.push_back(symTab->getObj(n));
		break;
	}
	case STORELOCAL:{
		int n = getWord();
		stack[base + n] = stack[--top];
		top--;
	}
	case STOREGLOBAL:{
		int n = getWord();
		symTab->putObj(stack[--top]);
		break;
	}
	case PUSHREAL:{
		int f = getFloat();
		stack[top].type=NUMOBJ;
		stack[top].value.numval=f;
	}
	case OP_ADD:{
		Object& l = stack[top-2];
		Object& r = stack[top-1];
		if (l.type == NUMOBJ && r.type == NUMOBJ){

		}
		else if (l.type == STROBJ && r.type==NUMOBJ || 
				l.type == NUMOBJ && r.type == STROBJ ||
				l.type==STROBJ && r.type==STROBJ){
			
		}
	}
	}
}
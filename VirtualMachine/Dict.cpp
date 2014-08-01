#include "Dict.h"
#include "VirtualMachine.h"
#include "slang.h"
#include "Object.h"

struct Dict{
	std::unordered_map<Object,Object> attrs;
};
namespace std{
	template<>
	struct hash<Object>{
		typedef Object argument_type;
		typedef size_t value_type;
		value_type operator()(const argument_type& obj){
			switch (obj.type){
				case NUMOBJ:return hash<float>()(obj.value.numval); break;
				case STROBJ:return strHasher(obj.value.strObj); break;
				case FUNOBJ:return hash<decltype(obj.value.funObj)>()(obj.value.funObj); break;
				case BOOLOBJ:return hash<bool>()(obj.value.boolval); break;
				case CLSOBJ:return hash<decltype(obj.value.clsObj)>()(obj.value.clsObj); break;
				//case CLSTYPE:return hash<decltype(obj.value.)>()(obj.value.funObj);
				case NILOBJ:
				default:assert(0); break;
			}
		}
	};

	template<>
	struct equal_to<Object>{
		typedef bool result_type;
		typedef Object first_argument_type;
        typedef Object second_argument_type;
		bool operator()(first_argument_type const& a, second_argument_type const& b) const{
			switch (a.type){
				case NUMOBJ:return a.value.numval == b.value.numval;
				case STROBJ:return strEq(a.value.strObj,b.value.strObj); 
				case FUNOBJ:return a.value.funObj == b.value.funObj; 
				case BOOLOBJ:return a.value.boolval == b.value.boolval; 
				case CLSOBJ:return a.value.clsObj == b.value.clsObj;
				//case CLSTYPE:return hash<decltype(obj.value.)>()(obj.value.funObj);
				case NILOBJ:
				default:assert(0); break;
			}
		}
	};
}
Object dictNew(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	int len;
	Object* objs;
	getArgs(state, &len, &objs);
	Dict* l = new Dict;
	for (int i = 0; i < len; ++i)
		l->vec.push_back(objs[i]);
	Object obj;
	obj.type = USEROBJ | DICTOBJ;
	obj.value.userData.type = vm->dictCls;
	obj.value.userData.data = (void*)l;
	return obj;
}

Object dictGet(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	int len;
	Object* objs;
	getArgs(state, &len, &objs);
	assert(len == 2);
	Dict* l =(Dict*)objs[0].value.userData.data;
	float index = objs[1].value.numval;
	if (index < 0 || (size_t)index >= l->vec.size()){
		vm->throwError("index out of range",ARGUMENTERROR);
	}
	return l->vec[(size_t)index];
}

Object dictSet(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	int len;
	Object* objs;
	getArgs(state, &len, &objs);
	assert(len == 3);
	dict* l =(dict*)objs[0].value.userData.data;
	float index = objs[1].value.numval;
	if (index < 0 || (size_t)index >= l->vec.size()){
		vm->throwError("index out of range",ARGUMENTERROR);
	}
	l->vec[(size_t)index] = objs[2];
	return {NILOBJ,{}};
}

void dictInit(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	ClsType* cls = defineClass(state, "dict");
	vm->dictCls = cls;
	assert(cls != NULL);
	defineClassMethod(state, cls, "constructor", dictNew, ANYARG);
	defineClassMethod(state, cls, "[]", dictGet, 2);
	defineClassMethod(state, cls, "[]=", dictSet, 3);
}
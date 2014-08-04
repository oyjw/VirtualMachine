#include "Dict.h"
#include "VirtualMachine.h"
#include "slang.h"
#include "Object.h"
namespace std{
	template<>
	struct hash<Object>{
		typedef Object argument_type;
		typedef size_t value_type;
		value_type operator()(const argument_type& obj){
			if(obj.type == NUMOBJ){
				return hash<float>()(obj.value.numval);
			}
			else if (obj.type & USEROBJ && obj.type & STROBJ){
				return strHasher((StrObj*)obj.value.userData->data);
			}
			else
				 std::runtime_error("dict key type isn't num or str");
		}
	};

	template<>
	struct equal_to<Object>{
		typedef bool result_type;
		typedef Object first_argument_type;
        typedef Object second_argument_type;
		bool operator()(first_argument_type const& a, second_argument_type const& b) const{
			if (a.type != b.type){
				throw std::runtime_error("dict key types don't match");
			}
			if(a.type == NUMOBJ){
				return a.value.numval == b.value.numval;
			}
			else if (a.type & USEROBJ && a.type & STROBJ){
				return ((StrObj*)a.value.userData->data)->str == ((StrObj*)b.value.userData->data)->str;
			}
			throw std::runtime_error("dict key type isn't num or str");
		}
	};
}

struct Dict{
	std::unordered_map<Object,Object> attrs;
};

Object dictNew(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	int len;
	Object* objs;
	getArgs2(state, &len, &objs);
	Dict* dict = new Dict;
	assert(len%2 == 0);
	for (int i = 0; i < len; i = i + 2){
		auto pair = dict->attrs.insert(std::make_pair(objs[i],objs[i+1]));
		if (!pair.second){
			vm->throwError("dict already has the same key",ATTRERROR);
		}
	}
	delete []objs;
	Object obj;
	obj.type = USEROBJ | DICTOBJ;
	obj.value.userData = new UserData(vm->dictCls,dict);
	setGC(state, obj.value.userData);
	return obj;
}

//Object dictGet(void* state){
//	VirtualMachine *vm = (VirtualMachine*)state;
//	int len;
//	Object* objs = NULL;
//	getArgs2(state, &len, &objs);
//	assert(len == 2);
//	Dict* l =(Dict*)objs[0].value.userData.data;
//	float index = objs[1].value.numval;
//	if (index < 0 || (size_t)index >= l->vec.size()){
//		vm->throwError("index out of range",ARGUMENTERROR);
//	}
//	return l->vec[(size_t)index];
//}
//
//Object dictSet(void* state){
//	VirtualMachine *vm = (VirtualMachine*)state;
//	int len;
//	Object* objs = NULL;
//	getArgs2(state, &len, &objs);
//	assert(len == 3);
//	dict* l =(dict*)objs[0].value.userData.data;
//	float index = objs[1].value.numval;
//	if (index < 0 || (size_t)index >= l->vec.size()){
//		vm->throwError("index out of range",ARGUMENTERROR);
//	}
//	l->vec[(size_t)index] = objs[2];
//	return {NILOBJ,{}};
//}

void dictInit(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	ClsType* cls = defineClass(state, "dict");
	vm->dictCls = cls;
	assert(cls != NULL);
	defineClassMethod(state, cls, "constructor", dictNew, EVENARG);
	//defineClassMethod(state, cls, "[]", dictGet, 2);
	//defineClassMethod(state, cls, "[]=", dictSet, 3);
}
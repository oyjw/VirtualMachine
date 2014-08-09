#ifndef _DICT_H_
#define _DICT_H_
#include "Object.h"
namespace std{
	template<>
	struct hash<Object>{
		typedef Object argument_type;
		typedef size_t value_type;
		value_type operator()(const argument_type& obj){
			value_type ret = 0;
			if(obj.type == NUMOBJ){
				ret = hash<float>()(obj.value.numval);
			}
			else if (obj.type & USEROBJ && obj.type & STROBJ){
				ret = strHasher((StrObj*)obj.value.userData->data);
			}
			else assert(0);
			return ret;
		}
	};

	template<>
	struct equal_to<Object>{
		typedef bool result_type;
		typedef Object first_argument_type;
        typedef Object second_argument_type;
		bool operator()(first_argument_type const& a, second_argument_type const& b) const{
			bool ret = false;
			if (a.type != b.type){
				ret = false;
			}
			if(a.type == NUMOBJ){
				ret = a.value.numval == b.value.numval;
			}
			else if (a.type & USEROBJ && a.type & STROBJ){
				ret = ((StrObj*)a.value.userData->data)->str == ((StrObj*)b.value.userData->data)->str;
			}
			else assert(0);
			return ret;
		}
	};
}

struct Dict{
	std::unordered_map<Object,Object> attrs;
};
void dictInit(void* state);
#endif

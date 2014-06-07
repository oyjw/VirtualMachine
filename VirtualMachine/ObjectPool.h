#ifndef _OBJECT_POOL_H_
#define _OBJECT_POOL_H_
#include <string>
#include <vector>
#include <unordered_map>

#include "Object.h"

bool mark(Object* obj);

class ObjectPool{
private:
	std::vector<ClsObj*> objs;
public:
	ObjectPool() {}
	~ObjectPool(){
		for (auto &pobj : objs){
			delete pobj;
		}
	}
	void collect(){
		int j = 0;
		for (size_t i = 0; i < objs.size(); i++){
			if (1){

			}
			else{
				delete objs[i];
			}
		}
		objs.resize(j);
	}
	int putObj(ClsObj* pobj){
		objs.push_back(pobj);
		return (int)objs.size()-1;
	}
};

#endif
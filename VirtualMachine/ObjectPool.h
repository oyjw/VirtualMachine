#ifndef _OBJECT_POOL_H_
#define _OBJECT_POOL_H_
#include <string>
#include <vector>
#include <unordered_map>

#include "Object.h"

class ObjectPool{
private:
	std::vector<void*> objs;
	size_t nclass;
public:
	ObjectPool() :nclass(0) {}
	~ObjectPool(){
		for (auto &pobj : objs){
			delete pobj;
		}
	}
	void collect(){
		/*for (int i = nclass; i++; i < objs.size()){
			delete objs[i];
		}*/
	}
	int putCls(void* pcls){
		objs.push_back(pcls);
		nclass++;
		return (int)objs.size()-1;
	}
	int putObj(void* pobj){
		objs.push_back(pobj);
		return (int)objs.size()-1;
	}
};

#endif
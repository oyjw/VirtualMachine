#ifndef _OBJECT_POOL_H_
#define _OBJECT_POOL_H_
#include <string>
#include <vector>
#include <unordered_map>

#include "Object.h"

bool mark(Object* obj);

class ObjectPool{
private:
	std::vector<ClsObj*> clsObjVec;
	std::vector<UserData*> userDataVec;
public:
	ObjectPool() {}
	~ObjectPool(){
		for (auto &pobj : clsObjVec){
			delete pobj;
		}
		for (auto &userData : userDataVec){
			delete userData->data;
			delete userData;
		}
	}
	void collect(){
		int j = 0;
		for (size_t i = 0; i < clsObjVec.size(); i++){
			if (clsObjVec[i]->mark){
				clsObjVec[i]->mark = false;
				clsObjVec[j++] = clsObjVec[i];
			}
			else{
				delete clsObjVec[i];
			}
		}
		clsObjVec.resize(j);
		j = 0;
		for (size_t i = 0; i < userDataVec.size(); i++){
			if (userDataVec[i]->mark){
				userDataVec[i]->mark = false;
				userDataVec[j++] = userDataVec[i];
			}
			else{
				delete userDataVec[i]->data;
				delete userDataVec[i];
			}
		}
		userDataVec.resize(j);
	}
	void putClsObj(ClsObj* clsObj){
		clsObjVec.push_back(clsObj);
	}
	void putUserData(UserData* userData){
		userDataVec.push_back(userData);
	}
	size_t getObjNum(){
		return clsObjVec.size() + userDataVec.size();
	}
};

#endif
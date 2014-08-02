#ifndef _OBJECT_POOL_H_
#define _OBJECT_POOL_H_
#include <string>
#include <vector>
#include <unordered_map>

#include "Object.h"
#include "List.h"
bool mark(Object* obj);

class ObjectPool{
private:
	std::vector<ClsObj*> clsObjVec;
	std::vector<std::pair<UserData*,bool>> userDataVec;
public:
	ObjectPool() {}
	~ObjectPool(){
		for (auto &pobj : clsObjVec){
			delete pobj;
		}
		for (auto &userData : userDataVec){
			if (!userData.second){
				if (userData.first->type->clsName == "list")
					delete (List*)userData.first->data;
				else delete userData.first->data;	
			}
			delete userData.first;
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
			if (userDataVec[i].first->mark){
				userDataVec[i].first->mark = false;
				userDataVec[j++] = userDataVec[i];
			}
			else{
				if (!userDataVec[i].second){
					if (userDataVec[i].first->type->clsName == "list")
						delete (List*)userDataVec[i].first->data;
					else delete userDataVec[i].first->data;	
				}
				delete userDataVec[i].first;
			}
		}
		userDataVec.resize(j);
	}
	void putClsObj(ClsObj* clsObj){
		clsObjVec.push_back(clsObj);
	}
	void putUserData(UserData* userData,bool isStr){
		userDataVec.push_back(std::make_pair(userData, isStr));
	}
	size_t getObjNum(){
		return clsObjVec.size() + userDataVec.size();
	}
};

#endif
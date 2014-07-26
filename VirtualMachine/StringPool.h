#ifndef _STRING_POOL_H_
#define _STRING_POOL_H_
#include <string>
#include <vector>
#include <unordered_map>

#include "Object.h"

class StringPool{
private:
	std::unordered_map<std::string,int> builtIns;
	std::unordered_map<std::string,int> sharedStrings;
	std::vector<StrObj*> strings;
	size_t constants;
public:
	StringPool() :constants(0) {
		putBuiltInStr("constructor");
	}
	~StringPool(){
		for (auto &sobj : strings){
			delete sobj;
		}
	}
	void collect(){
		size_t j = constants;
		for (size_t i = constants; i < strings.size(); ++i){
			if (strings[i]->mark){
				strings[j++] = strings[i];
			}
			else{
				delete strings[i];
			}
		}
		strings.resize(j);
	}
	int putStringConstant(const std::string& str){
		StrObj *sobj = new StrObj(str);
		strings.push_back(sobj);
		constants++;
		sharedStrings.insert(std::make_pair(str,constants));
		return (int)constants;
	}
	int putBuiltInStr(const std::string& str){
		
		int sindex = putStringConstant(str);
		builtIns.insert(std::make_pair(str,sindex));
		return sindex;
	}
	StrObj* putString(const std::string &str){
		StrObj *sobj = new StrObj(str);
		strings.push_back(sobj);
		return sobj;
	}
	StrObj* getStringConstant(const std::string& str){
		auto iter = sharedStrings.find(str);
		assert(iter != builtIns.end());
		return strings[iter->second];
	}
	StrObj* getStrObj(size_t n){
		return strings[n];
	}
	int getStrNum(){
		return int(strings.size()-constants);
	}

};

#endif
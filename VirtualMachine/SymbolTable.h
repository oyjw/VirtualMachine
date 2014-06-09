#ifndef _SYMBOL_TABLE_H_
#define _SYMBOL_TABLE_H_

#include <map>
#include <string>
#include <cassert>
#include "Object.h"
#include <utility>

#include <memory>

class SymbolTable;
typedef std::shared_ptr<SymbolTable> SymPtr;

typedef std::pair<bool,int> SymPair;

class SymbolTable{
private:
	SymPtr next;
	typedef std::map<std::string, int>::iterator map_iter;

	/*const std::string& findSymLocal(int index){
		assert(index<(int)symVec.size()) ;
		return symVec[index].objName;
	}
*/
	
public:
	std::map<std::string, int> map;
	std::vector<Symbol> symVec;
	bool isGlobal;
	int nLocalVars;
	std::vector<Symbol>&& getSymbols(){
		return std::move(symVec);
	}
	SymbolTable(SymPtr nextSymTab,int n) :next(nextSymTab), isGlobal(false), nLocalVars(n) {}
	SymbolTable():isGlobal(true),nLocalVars(0) {
	}
	~SymbolTable() {
		if (isGlobal){
			for (auto& symbol : symVec){
				Object& obj = symbol.obj;
				if (obj.type == FUNOBJ)
					delete obj.value.funObj;
				else if (obj.type == CLSTYPE){
					auto &map = obj.value.clsType->clsAttrs;
					for (auto iter = map.begin(); iter != map.end(); ++iter){
						if (iter->second.type == FUNOBJ){
							delete iter->second.value.funObj;
						}
					}
					delete obj.value.clsType;
				}
			}
		}
	}
	SymbolTable(const SymbolTable& symTab) =delete;
	bool isSymExistLocal(const std::string& symbol){
		map_iter iter = map.find(symbol);
		if (iter != map.end())
			return true;
		return false;
	}

	std::pair<bool,int> findSym(const std::string& symbol){
		map_iter iter = map.find(symbol);
		if (iter != map.end()) 
			return std::make_pair(isGlobal,iter->second+nLocalVars);
		SymPtr sp=next;
		while (sp!=NULL){
			std::map<std::string,int> nextMap=sp->map;
			iter = nextMap.find(symbol);
			if (iter != nextMap.end()) {
				int ret;
				if(sp->isGlobal)
					ret=iter->second;
				else
					ret=sp->nLocalVars+iter->second;
				return std::make_pair(sp->isGlobal,ret);
			}
			sp = sp->next;
		}
		return std::make_pair(false,-1);
	}
	
	int putSym(const std::string& symbol){
		Object obj = {NILOBJ,0};
		symVec.push_back({ obj, symbol });
		int n = symVec.size() - 1;
		map.insert(make_pair(symbol, n));
		return n;
	}

	Object& getObj(int idx){
		return symVec[idx].obj;
	}

	void putObj(int n, Object obj){
		assert(n<(int)symVec.size() && n>=0);
		symVec[n].obj = obj;
	}

	/*bool putObj(const std::string& symbol, Object obj){
		map_iter iter = map.find(symbol);
		if (iter == map.end()) return false;
		symVec[iter->second].obj = obj;
		return true;
	}*/
	int getSymNum() {
		return (int)symVec.size();
	}
	int getLocalVars() {
		return (int)symVec.size() + nLocalVars;
	}
	SymPtr getNext(){
		return next;
	}
};

#endif
#ifndef _SYMBOL_TABLE_H_
#define _SYMBOL_TABLE_H_

#include <map>
#include <string>
#include <cassert>
#include "Object.h"


#include <memory>

class SymbolTable;
typedef std::shared_ptr<SymbolTable> SymPtr;

class SymbolTable{
private:
	std::map<std::string, int> map;
	std::vector<Symbol> symVec;
	SymPtr next;
	typedef std::map<std::string, int>::iterator map_iter;

	/*const std::string& findSymLocal(int index){
		assert(index<(int)symVec.size()) ;
		return symVec[index].objName;
	}
*/
	
public:
	std::vector<Symbol>&& getSymbols(){
		return std::move(symVec);
	}
	SymbolTable(SymPtr nextSymTab) :next(nextSymTab) {}
	SymbolTable() {}
	~SymbolTable() {
		for (auto& symbol : symVec){
			Object& obj=symbol.obj;
			if (obj.type==FUNOBJ)
				delete obj.value.funObj;
		}

	}
	SymbolTable(const SymbolTable& symTab) =delete;
	int findSymLocal(const std::string& symbol){
		map_iter iter = map.find(symbol);
		if (iter != map.end())
			return iter->second;
		return -1;
	}

	int findSym(const std::string& symbol){
		map_iter iter = map.find(symbol);
		if (iter != map.end()) 
			return iter->second;
		SymPtr sp=next;
		int symNum = (int)symVec.size();
		while (sp!=NULL){
			std::map<std::string,int> nextMap=sp->map;
			iter = nextMap.find(symbol);
			if (iter!=nextMap.end()) 
				return symNum+iter->second;
			sp = sp->next;
			symNum += int(sp->symVec.size());
		}
		return -1;
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
	SymPtr getNext(){
		/*SymPtr p=next;
		while (next != NULL){
			p=p->next;
		}*/
		return next;
	}
};





#endif
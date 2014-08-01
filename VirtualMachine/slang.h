#ifndef _SLANG_H_
#define _SLANG_H_


#include "Object.h"
#ifdef __cplusplus
extern "C" {
#endif
#define SYMBOLEXIST 1

#define TYPEERROR 0
#define ATTRERROR 1
#define ARGUMENTERROR 2
#define SYNTAXERROR 3
#define SYMBOLERROR 4
#define TOKENERROR 5

#define ANYARG -1
int defineMethod(void* state,cFunc func);
void* newState();
void freeState(void* state);
void parseFile(void* state, const char* fileName);
void getArgs(void* state, int *len, Object** objects);
ClsType* defineClass(void* state, char* className);
int defineClassMethod(void* state, void* cls, char* funcName, cFunc func, int nArgs);
void setGC(void* state,UserData *obj);
void setGC2(void* state,UserData *obj);
#ifdef __cplusplus
}
#endif

#endif
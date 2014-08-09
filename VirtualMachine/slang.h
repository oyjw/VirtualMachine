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
#define MEMORYERROR 3
#define SYNTAXERROR 4
#define SYMBOLERROR 5
#define TOKENERROR 6

#define ANYARG -1
#define EVENARG -2
int defineMethod(void* state,cFunc func);
void* newState();
void freeState(void* state);
void parseFile(void* state, const char* fileName);
void getArgs(void* state, int *len, Object* objects);
void getArgs2(void* state, int *len, Object** objects);
ClsType* defineClass(void* state, char* className);
int defineClassMethod(void* state, void* cls, char* funcName, cFunc func, int nArgs);
void setGC(void* state,UserData *obj);
void setGC2(void* state,UserData *obj);
#ifdef __cplusplus
}
#endif

#endif
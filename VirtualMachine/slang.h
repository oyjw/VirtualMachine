#ifndef _SLANG_H_
#define _SLANG_H_


#include "Object.h"
#ifdef __cplusplus
extern "C" {
#endif
#define SYMBOLEXIST 1

#define ANY -1
int defineMethod(void* state,cFunc func);
void* newState();
void freeState(void* state);
void parseFile(void* state, const char* fileName);
void getArgs(void* state, int *len, Object** objects);
ClsType* defineClass(void* state, char* className);
int defineClassMethod(void* state, void* cls, char* funcName, cFunc func, int nArgs);
#ifdef __cplusplus
}
#endif

#endif
#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_
#include <exception>
#include <string>
class TypeError :public std::exception
{
public:
	TypeError(const std::string& msg) :exception(msg) {}
}


class TokenError :public std::exception
{
public:
	TokenError(const std::string& msg):exception(msg) {}
}

class SyntaxError :public std::exception
{
public:
	SyntaxError(const std::string& msg) :exception(msg) {}
}

class SymbolError :public std::exception
{
public:
	SymbolError(const std::string& msg) :exception(msg) {}
}
#endif

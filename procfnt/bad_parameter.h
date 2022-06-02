#pragma once
#include "exception.h"
#include <string>
#include <iostream>

class bad_parameter : public exception
{
protected:
	std::string message;
	std::string paraName;
public:
	bad_parameter(std::string param, std::string msg);

	std::string ToString();

	//const char* what();
};


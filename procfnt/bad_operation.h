#pragma once

#include "exception.h"
#include <string>

class bad_operation : exception
{
protected:
	std::string message;
public:
	bad_operation(std::string p_message);

	std::string ToString();

	//const char* what();
};


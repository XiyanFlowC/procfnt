#pragma once

#include <exception>
#include <string>

class bad_operation : std::exception
{
protected:
	std::string message;
public:
	bad_operation(std::string p_message);

	std::string to_string();
};


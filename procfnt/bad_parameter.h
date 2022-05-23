#pragma once
#include <exception>
#include <string>
#include <iostream>

class bad_parameter : public std::exception
{
protected:
	std::string message;
	std::string paraName;
public:
	bad_parameter(std::string param, std::string msg);

	std::string to_string();
};


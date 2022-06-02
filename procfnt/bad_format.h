#pragma once
#include "exception.h"
#include <string>

class bad_format : exception
{
protected:
	std::string file;
	size_t offset;
	std::string message;
public:
	std::string FileName();

	size_t Offset();

	bad_format(std::string fileName, size_t offset, std::string message);

	std::string ToString();

	// const char* what();
};


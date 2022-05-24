#pragma once
#include <exception>
#include <string>

class bad_format : std::exception
{
protected:
	std::string file;
	size_t offset;
	std::string message;
public:
	std::string FileName();

	size_t Offset();

	bad_format(std::string fileName, size_t offset, std::string message);

	std::string to_string();

	const char* what();
};


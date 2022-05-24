#include "bad_format.h"

std::string bad_format::FileName()
{
	return file;
}

size_t bad_format::Offset()
{
	return offset;
}

bad_format::bad_format(std::string fileName, size_t offset, std::string message)
{
	file = fileName;
	this->offset = offset;
	this->message = message;
}

std::string bad_format::to_string()
{
	return "Invalid format in file: " + file + "(near " + std::to_string(offset) + "): " + message + "\n";
}

const char* bad_format::what()
{
	return to_string().c_str();
}

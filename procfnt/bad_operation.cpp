#include "bad_operation.h"

bad_operation::bad_operation(std::string p_message)
{
	message = p_message;
}

std::string bad_operation::to_string()
{
	return message;
}

const char* bad_operation::what()
{
	return to_string().c_str();
}

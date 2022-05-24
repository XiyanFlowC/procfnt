#include "bad_parameter.h"

bad_parameter::bad_parameter(std::string param, std::string msg)
{
	paraName = param;
	message = msg;
}

std::string bad_parameter::to_string()
{
	return std::string("Parameter ") + paraName + " failure: " + message + "\n";
}

const char* bad_parameter::what()
{
	return to_string().c_str();
}

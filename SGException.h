#pragma once
#include <exception>

#define SG_THROW(v) {std::stringstream _____s; _____s << __FUNCTION__ << "(" << __LINE__ << "): " << v; throw std::exception(_____s.str().c_str());}


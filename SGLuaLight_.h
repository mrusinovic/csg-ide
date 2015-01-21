#pragma once
#include "SGLuaVector.h"

#include "carve/poly.hpp"

namespace CSGProcessor
{

struct SLuaLight
{

	std::shared_ptr<carve::poly::Polyhedron> poly;
	irr::core::vector3df color;

	SLuaLight(const std::shared_ptr<carve::poly::Polyhedron>& p, const SGLuaVector& c);

	//~SLuaLight(void);

	//static void LuaBind(lua_State* L);

};


}
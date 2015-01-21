#include "stdafx.h"
#include "SGLuaLight.h"

namespace CSGProcessor
{

SLuaLight::SLuaLight(const std::shared_ptr<carve::poly::Polyhedron>& p, const SGLuaVector& c)
{

}

SLuaLight::~SLuaLight(void)
{
}

// void SLuaLight::LuaBind( lua_State* L )
// {
// 	using namespace luabind;
//
//	module(L)
// 		[
// 			class_<SLuaLight>("light")
// 			.def(constructor<const SGLuaVector&,const SGLuaVector,float>())
// 			.def(constructor<const SGLuaVector&,float>())
// 			.def(constructor<const SGLuaVector&,const SGLuaVector>())
// 			.def(constructor<const SGLuaVector&>())
// 			// 			.def(constructor<const cl_I&>())
// 			// 			.def(constructor<float>())
// 			// 			.def(constructor<const char*>())
// 			// 			.def(tostring(const_self))
// 			//.def("render",&CSGMaterial::render)
//		];
// }

}
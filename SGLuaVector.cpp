#include "stdafx.h"
#include "SGLuaVector.h"

namespace CSGProcessor
{

#define OP_REG(op) LuaPVal operator op(const LuaPVal& p1, const LuaPVal& p2){return LuaPVal(p1.v op p2.v);}
OP_REG(+) OP_REG(-) OP_REG(/) OP_REG(*)
#undef OP_REG
#define OP_REG(op) LuaPVal operator op(const LuaPVal& p1, float p2){return LuaPVal(p1.v op p2);}
OP_REG(+) OP_REG(-) OP_REG(/) OP_REG(*)
#undef OP_REG


#define OP_REG(op) SGLuaVector operator op(const SGLuaVector& p1, const SGLuaVector& p2){return SGLuaVector(p1.uv op p2.uv, p1.p);}
OP_REG(+) OP_REG(-) OP_REG(/) OP_REG(*)
#undef OP_REG
#define OP_REG(op) SGLuaVector operator op(const SGLuaVector& p1, float p2){return SGLuaVector(p1.uv op p2, p1.p);}
OP_REG(+) OP_REG(-) OP_REG(/) OP_REG(*)
#undef OP_REG


SGLuaVector::SGLuaVector( luabind::object const& x, luabind::object const& y, luabind::object const& z )
{
	LuaPVal f(x); uv.X=f; p.X = f;
	f = LuaPVal(y); uv.Y=f; p.Y = f;
	f = LuaPVal(z); uv.Z=f; p.Z = f;
}



void SGLuaVector::LuaBind( lua_State* L )
{
	using namespace luabind;
	module(L)
		[
			class_<SGLuaVector>("V")
			.def(constructor<luabind::object,luabind::object,luabind::object>())
			.def(const_self + const_self)
			.def(const_self - const_self)
			.def(const_self * const_self)
			.def(const_self / const_self)
			.def(const_self + other<float>())
			.def(const_self - other<float>())
			.def(const_self * other<float>())
			.def(const_self / other<float>())
			.def(const_self + other<float>())
			.def(-self)
			,class_<LuaPVal>("P")
			.def(constructor<float>())
			.def(const_self + const_self)
			.def(const_self - const_self)
			.def(const_self * const_self)
			.def(const_self / const_self)
			.def(const_self + other<float>())
			.def(const_self - other<float>())
			.def(const_self * other<float>())
			.def(const_self / other<float>())
			.def(const_self + other<float>())
		];

}

}
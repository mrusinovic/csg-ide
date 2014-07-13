#include "stdafx.h"
#include "SGLuaVector.h"


#define OP_REG(op) LuaPVal operator op(const LuaPVal& p1, const LuaPVal& p2){return LuaPVal(p1.v op p2.v);}
OP_REG(+) OP_REG(-) OP_REG(/) OP_REG(*)
#undef OP_REG
#define OP_REG(op) LuaPVal operator op(const LuaPVal& p1, float p2){return LuaPVal(p1.v op p2);}
OP_REG(+) OP_REG(-) OP_REG(/) OP_REG(*)
#undef OP_REG



SGLuaVector::SGLuaVector( luabind::object const& x, luabind::object const& y, luabind::object const& z )
{
	LuaPVal f(x); X=f; px = f;
	f = LuaPVal(y); Y=f; py = f;
	f = LuaPVal(z); Z=f; pz = f;
}



void SGLuaVector::LuaBind( lua_State* L )
{
	using namespace luabind;
	module(L)
		[
			class_<SGLuaVector>("vec")
			.def(constructor<luabind::object,luabind::object,luabind::object>())
			,class_<LuaPVal>("p")
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

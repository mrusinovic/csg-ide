#pragma once

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "luabind/luabind.hpp"

struct LuaFVal {
	float v;
	bool p;
	LuaFVal(){}
	LuaFVal(float _v):v(_v),p(false){}
	operator float(){ return v; }
	operator bool(){ return p; }
};

struct LuaPVal : LuaFVal { 
	LuaPVal(){}
	explicit LuaPVal(float _v):LuaFVal(_v){ p=true;} 

	explicit LuaPVal(luabind::object const& x) : LuaFVal(0.f)
	{
		switch (luabind::type(x)){
		case LUA_TUSERDATA:
			v = luabind::object_cast<LuaPVal>(x).v/100;
			p = true;
			break;
		case LUA_TNUMBER:
			v = luabind::object_cast<float>(x);
			break;
		case LUA_TSTRING:
			{
				std::string str(luabind::object_cast<const char*>(x));
				auto pos = str.find('%');

				if (pos!=std::string::npos){
					v = atof(str.c_str())/100;
					p=true;
				}else{
					v = atof(str.c_str());
					p=false;
				}
			}
			break;
		default:
			throw std::exception("Parameter must be string(\"55%\") or number(0.45)!");
		}
	}

};


typedef irr::core::vector3df UnderVector;

struct SGLuaVector
{
	float X,Y,Z;
	bool px,py,pz;

	SGLuaVector(const UnderVector& v):X(v.X),Y(v.Y),Z(v.Z),px(false),py(false),pz(false){}

	SGLuaVector(float x, float y, float z):X(x),Y(y),Z(z),px(false),py(false),pz(false){}

	SGLuaVector(luabind::object const& x, luabind::object const& y, luabind::object const& z);

	static void LuaBind( lua_State* L );

	SGLuaVector operator -()const{ SGLuaVector v = *this; v.X*=-1; v.Y*=-1; v.Z*=-1; return v;}

	UnderVector vec()const{return UnderVector(X,Y,Z);}
};

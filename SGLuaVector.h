#pragma once
#include "SGException.h"


namespace CSGProcessor
{

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
					v = (float)atof(str.c_str())/100;
					p=true;
				}else{
					v = (float)atof(str.c_str());
					p=false;
				}
			}
			break;
		default:
			SG_THROW("Parameter must be string(\"55%\") or number(0.45)!");
		}
	}

};


typedef irr::core::vector3df UnderVector;

struct SGLuaVector
{
	UnderVector uv;
	irr::core::vector3di p;

	SGLuaVector(){}

	SGLuaVector(const UnderVector& v):uv(v), p(irr::core::vector3di()){}

	SGLuaVector(const UnderVector& v, irr::core::vector3di np):uv(v), p(np){}

	SGLuaVector(float x, float y, float z):uv(x,y,z),p(irr::core::vector3di()){}

	SGLuaVector(luabind::object const& x, luabind::object const& y, luabind::object const& z);

	static void LuaBind( lua_State* L );

	SGLuaVector operator -()const{ SGLuaVector v = *this; uv * -1; return v;}




};

}
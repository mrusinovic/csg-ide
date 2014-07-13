#pragma once


extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "luabind/luabind.hpp"


// enum SGPolyType{
// 	SGP_CUBE, SGP_SPHERE
// }


#define LOG_OUTPUT(msg,...)			CSGProcessor::Log(/*__FILE__*/"",__FUNCTION__,msg,__VA_ARGS__)
#define LOG_RESET					CSGProcessor::LogReset();


namespace carve{
	namespace poly{
		struct Polyhedron;
	}
}

namespace CSGProcessor
{
	void LogReset();
	void Log(const char* file, const char* function, const char* msg,...);



class CSGPoligonState
{
	std::shared_ptr<carve::poly::Polyhedron> m_poly;
public:
	CSGPoligonState(void);
	CSGPoligonState(std::shared_ptr<carve::poly::Polyhedron> p);
	~CSGPoligonState(void);

	CSGPoligonState clone()const;

	carve::poly::Polyhedron* GetPoly()const {return m_poly.get();}

	void transform(const irr::core::matrix4& m)const;

	//inverts normals and winding order of vertices
	CSGPoligonState& invert();
	CSGPoligonState& scale(float x, float y, float z);
	CSGPoligonState& move(float x, float y, float z);
	CSGPoligonState& rotate(float angle, float x, float y, float z);

	void clear(){m_poly=nullptr;}
	bool contains( CSGProcessor::CSGPoligonState const& ps );

	static void LuaBind(lua_State* L);

	static CSGPoligonState CUBE();
	static CSGPoligonState SPHERE();
	static CSGPoligonState TORUS();
	static CSGPoligonState CYLINDER();
	static CSGPoligonState CONE();
	static CSGPoligonState CONE(int slices);
};

//performs union
CSGPoligonState operator +(const CSGPoligonState& p1,const CSGPoligonState& p2);

//all from p1 that is not in p2
CSGPoligonState operator -(const CSGPoligonState& p1,const CSGPoligonState& p2);

//intersection
CSGPoligonState operator *(const CSGPoligonState& p1,const CSGPoligonState& p2);
}
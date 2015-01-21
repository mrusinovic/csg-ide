#pragma once



// enum SGPolyType{
// 	SGP_CUBE, SGP_SPHERE
// }


// namespace carve{
// 	namespace poly{
// 		struct Polyhedron;
// 	}
// }

#include "carve/poly.hpp"
//#include "SGMaterial.h"
#include "SGMaterial.h"

namespace CSGProcessor
{
	void LogReset();
	void Log(const char* file, const char* function, const char* msg,...);



class CSGPoligonState
{
	std::shared_ptr<carve::mesh::MeshSet<3>> m_poly;
public:
	CSGPoligonState(void);
	CSGPoligonState(std::shared_ptr<carve::mesh::MeshSet<3>> p);
	~CSGPoligonState(void);

	CSGPoligonState clone()const;

	std::shared_ptr<carve::mesh::MeshSet<3>> GetPoly()const {return m_poly;}

	void transform(const irr::core::matrix4& m_)const;

	//inverts normals and winding order of vertices
	CSGPoligonState& invert();
	CSGPoligonState& scale(float x, float y, float z);
	CSGPoligonState& move(float x, float y, float z);
	CSGPoligonState& rotate(float angle, float x, float y, float z);

	CSGPoligonState& uvscale(float u, float v);
	CSGPoligonState& uvmove(float u, float v);
	CSGPoligonState& uvswap();

	CSGPoligonState& simplify();

	void clear(){m_poly=nullptr;}
	bool contains( CSGProcessor::CSGPoligonState const& ps );

	static void LuaBind(lua_State* L);

#define DECLARE_CONSTRUCTION(name)\
	static CSGPoligonState name();\
	static CSGPoligonState name(const SGMaterial& m);\
	static CSGPoligonState name(const SGMaterial& m, float tx, float ty);\
	static CSGPoligonState name(float tx, float ty)

#define DECLARE_CONSTRUCTION_PAR1(name, type, par)\
	static CSGPoligonState name(type par);\
	static CSGPoligonState name(type par, const SGMaterial& m);\
	static CSGPoligonState name(type par, const SGMaterial& m, float tx, float ty);\
	static CSGPoligonState name(type par, float tx, float ty)

	DECLARE_CONSTRUCTION(CUBE);
	DECLARE_CONSTRUCTION(SPHERE);
//	DECLARE_CONSTRUCTION(TORUS);
	DECLARE_CONSTRUCTION(CYLINDER);
	DECLARE_CONSTRUCTION(CONE);
	DECLARE_CONSTRUCTION_PAR1(CONE,int,slices);
#undef DECLARE_CONSTRUCTION
#undef DECLARE_CONSTRUCTION_PAR1

};

//performs union
CSGPoligonState operator +(const CSGPoligonState& p1,const CSGPoligonState& p2);

//all from p1 that is not in p2
CSGPoligonState operator -(const CSGPoligonState& p1,const CSGPoligonState& p2);

//intersection
CSGPoligonState operator *(const CSGPoligonState& p1,const CSGPoligonState& p2);
}
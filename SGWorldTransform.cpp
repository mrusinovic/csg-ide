#include "stdafx.h"

#include "carve/csg.hpp"
#include "SGWorldTransform.h"
#include "SGException.h"
#include "SGScript.h"

namespace CSGProcessor
{


CSGWorldTransform::CSGWorldTransform( void )
	:m_parent(NULL)
{

}

CSGWorldTransform::~CSGWorldTransform( void )
{

}


CSGWorldTransform::TransMatrix CSGWorldTransform::get()const
{
	CSGWorldTransform::TransMatrix ret(m_initial.get());

	for(auto& m : m_stack)
		ret = ret * m.get();

	return ret*matrix.get();
}


const int XX = 1;
const int YY = 2;
const int ZZ = 3;

void CSGWorldTransform::LuaBind( lua_State* L )
{
	using namespace luabind;
	module(L)
		[
			class_<CSGWorldTransform>("w_s")
			.def("init",&CSGWorldTransform::init)
			.def("start",&CSGWorldTransform::make_child)
			.def("finish",&CSGWorldTransform::get_parent)
			.def("render",(void(CSGWorldTransform::*)(const CSGPoligonState&))&CSGWorldTransform::render)
			.def("render",(void(CSGWorldTransform::*)(const CSGPoligonState&, bool))&CSGWorldTransform::render)
			.def("invert",&CSGWorldTransform::invert)
			.def("rotate",&CSGWorldTransform::rotate)
			.def("sides",&CSGWorldTransform::select_sides)
			.def("side",&CSGWorldTransform::select_side)
			.def("divide",&CSGWorldTransform::divide)
			.def("split",&CSGWorldTransform::split)
			.def("size",&CSGWorldTransform::size)
			.def("scale",&CSGWorldTransform::scale)
			.def("scaleby",&CSGWorldTransform::scaleby)
			.def("move",&CSGWorldTransform::move)
			.def("push",&CSGWorldTransform::push)
			.def("pop",&CSGWorldTransform::pop)
			.def("ocluded",&CSGWorldTransform::ocluded)
			.def("transform",&CSGWorldTransform::transform)
			//.def("light",&CSGWorldTransform::light)
			.property("width",&CSGWorldTransform::getWidth)
			.property("height",&CSGWorldTransform::getHeight)
			.property("depth",&CSGWorldTransform::getDepth)
			.property("ocl",&CSGWorldTransform::getOcl, &CSGWorldTransform::setOcl)
			//.def_readwrite("ocl_pass",&CSGWorldTransform::m_oclPass)
		];

	luabind::globals(L)["PI"] = irr::core::PI64;

	luabind::globals(L)["X"] = XX;
	luabind::globals(L)["Y"] = YY;
	luabind::globals(L)["Z"] = ZZ;


	SGLuaVector::LuaBind(L);
//	SLuaLight::LuaBind(L);

}


UnderVector CSGWorldTransform::ConvertVector( SGLuaVector const& v )
{
	auto m = matrix;
	matrix.makeIdentity();
	auto tr = get();
	matrix = m;
	auto tmp = tr.getScale();
	return UnderVector(v.p.X ? matrix.s.X * v.uv.X : v.uv.X/tmp.X, v.p.Y ? matrix.s.Y * v.uv.Y : v.uv.Y/tmp.Y, v.p.Z ? matrix.s.Z * v.uv.Z : v.uv.Z/tmp.Z);
}


void CSGWorldTransform::init( float x, float y, float z )
{
//	m_oclPass=false;
	m_ocl = CSGProcessor::CSGPoligonState();

	m_stack.clear();

	m_dimStart.set(x,y,z);

	m_initial.makeIdentity();
	matrix.makeIdentity();

	m_initial.s = m_dimStart;
}

CSGWorldTransform* CSGWorldTransform::get_parent()
{
	auto i = CSGScript::GetInstance();
	i->SetCurWT(m_parent);
	return m_parent;
}

CSGWorldTransform* CSGWorldTransform::make_child(  )
{
	auto c = std::make_shared<CSGWorldTransform>();
	c->m_parent = this;

	auto m = get();

	c->m_initial.makeIdentity();
	c->matrix.makeIdentity();

	c->m_initial.t = m.getTranslation();
	c->m_initial.s	 = m.getScale();
	c->m_initial.r = m.getRotationDegrees()*irr::core::DEGTORAD64;
	c->m_dimStart = c->m_initial.s;

	m_children.push_back(c);

	auto i = CSGScript::GetInstance();

	i->SetCurWT(c.get());
	return c.get();
}

void CSGWorldTransform::render( const CSGPoligonState& a )
{
	render(a,true);
}

void CSGWorldTransform::render( const CSGPoligonState& a, bool tr )
{
	//MR: horrible! fix me
	//it is done in this way because carve is killing texture coordinates of some overlapping polys.
	//cube():scale(1,1,0.5) - cube():scale(0.7,0.7,0.5):move(0,0.1,0) + cube(wall1):scale(1.2,0.1,0.5):move(0,-0.5,0)
	//window_wall + floor destroys wall texture coords.

//	auto msh = transform(a.clone()) - m_staticGeometry;

// 	if (m_ocl.GetPoly())
// 		m_staticGeometry = m_staticGeometry + (transform(a) - m_ocl - m_staticGeometry);
// 	else
// 		m_staticGeometry = m_staticGeometry + (transform(a) - m_staticGeometry);

	auto p = tr ? transform(a.clone()) : a;

	//p.simplify();

	if (m_ocl.GetPoly())
		m_staticGeometry = m_staticGeometry + (p - m_ocl);
	else
		m_staticGeometry = m_staticGeometry + p;

	m_staticGeometry.simplify();

}


void CSGWorldTransform::rotate( const SGLuaVector& v )
{
	matrix.r.set(v.uv.X,v.uv.Y,v.uv.Z);
}

void CSGWorldTransform::scaleby( const SGLuaVector& v )
{
	auto uv = ConvertVector(v).abs() * UnderVector(irr::core::sign(v.uv.X), irr::core::sign(v.uv.Y), irr::core::sign(v.uv.Z));
	matrix.s += uv;
}

void CSGWorldTransform::scale( const SGLuaVector& v )
{
	matrix.s = ConvertVector(v);
}

void CSGWorldTransform::move( const SGLuaVector& v )
{
	matrix.t += ConvertVector(v);
}

void CSGWorldTransform::select_side(int side, luabind::object const& fun )
{
	UnderVector uv;

	switch (side)
	{
	case XX: uv.X = 1; break;
	case YY: uv.Y = 1; break;
	case ZZ: uv.Z = 1; break;
	case -XX: uv.X = -1; break;
	case -YY: uv.Y = -1; break;
	case -ZZ: uv.Z = -1; break;
	default: SG_THROW("side - First parameter must be one of (x,y,z,-x,-y,-z)!");
	}

	push();
	to_side(uv.X,uv.Y,uv.Z);
	push();
	matrix.s.Z=0;
	luabind::call_function<void>(fun,uv.X,uv.Y,uv.Z);
	pop();
	pop();

}

void CSGWorldTransform::select_sides( luabind::object const& fun )
{
#define SSIDE(x,y,z)\
	push();\
	to_side(x,y,z);\
	push();\
	matrix.s.Z=0; \
	luabind::call_function<void>(fun,x,y,z);\
	pop(); pop()

	SSIDE(1,0,0);
	SSIDE(-1,0,0);
	SSIDE(0,0,1);
	SSIDE(0,0,-1);

#undef SSIDE

}

void CSGWorldTransform::divide( int _x, int _y, int _z, luabind::object const& fun )
{
	push();

	for(int x=0;x<_x;x++)
		for(int y=0;y<_y;y++)
			for(int z=0;z<_z;z++){

				matrix.makeIdentity();

				matrix.s.set(1.f/_x,1.f/_y,1.f/_z);

				matrix.t.set(
					matrix.s.X*x - matrix.s.X/2*(_x-1),
					matrix.s.Y*y - matrix.s.Y/2*(_y-1),
					matrix.s.Z*z - matrix.s.Z/2*(_z-1)
					);

				matrix.t.roundTo(5);

// 				auto tr = get().getTranslation();
// 				auto sc = get().getScale();
// 
// 				std::stringstream str;
// 				str << tr.X << "," << tr.Y << "," << tr.Z << " - "
// 					<< tr.X + sc.X << "," << tr.X + sc.Y << "," << tr.X + sc.Z << std::endl;
// 
// 
// 				OutputDebugStringA(str.str().c_str());


				push();
				luabind::call_function<void>(fun,x,y,z);
				pop();
			}


	pop();
}

void CSGWorldTransform::size( const SGLuaVector& v )
{
 	auto uv = ConvertVector(v).abs() * v.uv.getSigns();// * UnderVector(irr::core::sign(v.uv.X), irr::core::sign(v.uv.Y), irr::core::sign(v.uv.Z));

	auto m = matrix;
	m.s.set(1,1,1);

	matrix.s += uv;
	matrix.t = uv/2;
	matrix.r.set(0,0,0);

	auto a = m.get()*matrix.get();

	matrix.s = a.getScale();
	matrix.r = m.r;
	matrix.t = a.getTranslation();

}


//faceOut - when calling size will we extend outwards or in
void CSGWorldTransform::to_side( int x, int y, int z, bool faceOut )
{
	matrix.t = UnderVector(x,y,z)/2;
	if (x!=0){
		matrix.r = UnderVector(0, irr::core::PI/2 * (faceOut ? 1 : -1), 0) * irr::core::sign(x);
	}
	if (z>0){
		matrix.r.set(0, irr::core::PI * (faceOut ? -1 : 1), 0);
	}
	if (y!=0){
		matrix.r.set(irr::core::sign(y)*irr::core::PI/2 * (faceOut ? -1 : 1), 0, 0);
	}

	matrix.r.roundTo(5);

}

void CSGWorldTransform::split( int by, luabind::object const& table )
{
	struct PART{
		PART(const LuaFVal& s, const luabind::object& o):size(s),fun(o){}
		LuaFVal size;
		luabind::object fun;
	};

	std::vector<PART> parts;

	if (luabind::type(table) != LUA_TTABLE) 
		SG_THROW("split - Second parameter must be array!");

	for (luabind::iterator i(table), end; i != end; ++i)
	{
		try
		{
			auto f = LuaPVal(*i); ++i;
			if (i==end)
				SG_THROW("Odd number of values!");

			if (luabind::type(*i)!=LUA_TFUNCTION)
				SG_THROW("Value is not function pointer!");

			parts.emplace_back(f,*i);
		}
		catch (std::exception& e)
		{
			SG_THROW("split - Second parameter {val, fun,...} : " << e.what());
		}
	}

	float totalAbs = 0;
	float totalPerc = 0;
	bool bvalue = false;
	for(auto& p : parts) {
		if (!p.size.p) 
			totalAbs+=p.size.v;
		else {
			totalPerc += p.size.v;
			bvalue = true;
		}
	}

	if (bvalue && !irr::core::equals(totalPerc,1.f))
		SG_THROW("split - Sum of all percentages must be equal 100%!");

	bvalue = false;

	UnderVector scaleBy,scaleByInv(1);

	switch (by)
	{
	case XX: scaleByInv.X = 0; scaleBy.X = 1; if (totalAbs>getWidth()) bvalue = true; break;
	case YY: scaleByInv.Y = 0; scaleBy.Y = 1; if (totalAbs>getHeight()) bvalue = true; break;
	case ZZ: scaleByInv.Z = 0; scaleBy.Z = 1; if (totalAbs>getDepth()) bvalue = true; break;
	default: SG_THROW("split - First parameter must be one of (x,y,z)!");
	}
	if (bvalue) 
		SG_THROW("split - Sum of all absolute values can not be greater than available space!");

	/*
	
	|                                  3                            |
	*****************************************************************
	|           80%                    |     1        |     20%     |

	3-1 = 2
	livo 80% od 2
	desno 20% od 2

	|                                  4                            |
	*****************************************************************
	|         70%   |      1    |  10% |     1        |     20%     |

	4 - (1+1) = 2

	1.4             |      1    |  0.2 |    1         |     0.4      |


	2
	50% 0.5 50%
	0.75   0.5  0.75
	1
	0.375  0.25  0.375
	*/

	auto tr = UnderVector(-.5f)*scaleBy;
	
	push();

	auto scale = UnderVector(1) - ConvertVector(SGLuaVector(scaleBy*totalAbs));
	for(auto& p : parts) {

		UnderVector uv;
		if (p.size.p){
			uv = (scaleBy * (scale * p.size.v));
 		}else{
			uv = ConvertVector(SGLuaVector(scaleBy * p.size.v));
		}


		matrix.s = uv + scaleByInv;
		matrix.t.set(tr + uv/2);
		tr += uv;

		matrix.s.roundTo(5);
		matrix.t.roundTo(5);

		push();
		
		luabind::call_function<void>(p.fun);
		pop();
	}

	pop();

}

CSGProcessor::CSGPoligonState const& CSGWorldTransform::transform( CSGProcessor::CSGPoligonState const& ps )
{
	ps.transform(get());
	return ps;
}

bool CSGWorldTransform::ocluded()
{
	if (!m_ocl.GetPoly()) return false;

	//TODO: this is so bad my eyes hurt - check carve lib to find better way!
	return m_ocl.contains(transform(CSGProcessor::CSGPoligonState::CUBE()));
}

// void CSGWorldTransform::light( CSGProcessor::CSGPoligonState const& ps, const SGLuaVector& color )
// {
// 	m_lights.emplace_back(ps.irrMesh(),color.vec());
// }

}
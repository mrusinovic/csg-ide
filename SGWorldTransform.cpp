#include "stdafx.h"

#include "carve/csg.hpp"
#include "SGWorldTransform.h"

CSGWorldTransform::CSGWorldTransform( void )
	:m_oclPass(false)
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
			.def("rotate",&CSGWorldTransform::rotate)
			.def("sides",&CSGWorldTransform::select_sides)
			.def("side",&CSGWorldTransform::select_side)
			.def("divide",&CSGWorldTransform::divide)
			.def("split",&CSGWorldTransform::split)
			.def("size",&CSGWorldTransform::size)
			.def("scale",&CSGWorldTransform::scale)
			.def("move",&CSGWorldTransform::move)
			.def("push",&CSGWorldTransform::push)
			.def("pop",&CSGWorldTransform::pop)
			.def("ocluded",&CSGWorldTransform::ocluded)
			.def("transform",&CSGWorldTransform::transform)
			.property("width",&CSGWorldTransform::getWidth)
			.property("height",&CSGWorldTransform::getHeight)
			.property("depth",&CSGWorldTransform::getDepth)
			.property("ocl",&CSGWorldTransform::getOcl, &CSGWorldTransform::setOcl)
			.def_readwrite("ocl_pass",&CSGWorldTransform::m_oclPass)
		];

	luabind::globals(L)["pi"] = irr::core::PI64;

	luabind::globals(L)["x"] = XX;
	luabind::globals(L)["y"] = YY;
	luabind::globals(L)["z"] = ZZ;


	SGLuaVector::LuaBind(L);

}


UnderVector CSGWorldTransform::ConvertVector( SGLuaVector const& v )
{
	auto m = matrix;
	matrix.makeIdentity();
	auto tr = get();
	matrix = m;
	auto tmp = tr.getScale();
	return UnderVector(v.px ? matrix.s.X * v.X : v.X/tmp.X, v.py ? matrix.s.Y * v.Y : v.Y/tmp.Y,v.pz ? matrix.s.Z * v.Z : v.Z/tmp.Z);
}


void CSGWorldTransform::init( float x, float y, float z )
{
	m_oclPass=false;
	m_ocl = CSGProcessor::CSGPoligonState();

	m_stack.clear();

	m_dimStart.set(x,y,z);

	m_initial.makeIdentity();
	matrix.makeIdentity();

	m_initial.s = m_dimStart;
}

void CSGWorldTransform::rotate( const SGLuaVector& v )
{
	matrix.r.set(v.X,v.Y,v.Z);
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
	default: throw std::exception("side - First parameter must be one of (x,y,z,-x,-y,-z)!");
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

				push();
				luabind::call_function<void>(fun,x,y,z);
				pop();
			}


	pop();
}

template<typename T> int sign(const T& val){return (val > T(0)) - (val < T(0)); }

void CSGWorldTransform::size( const SGLuaVector& v )
{
 	auto uv = ConvertVector(v).abs() * UnderVector(sign(v.X), sign(v.Y),sign(v.Z));

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
		matrix.r =sign(x)*UnderVector(0, irr::core::PI/2 * (faceOut ? 1 : -1), 0);
	}
	if (z>0){
		matrix.r.set(0, irr::core::PI * (faceOut ? -1 : 1), 0);
	}
	if (y!=0){
		matrix.r.set(sign(y)*irr::core::PI/2 * (faceOut ? -1 : 1), 0, 0);
	}

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
		throw std::exception("split - Second parameter must be array!");

	for (luabind::iterator i(table), end; i != end; ++i)
	{
		try
		{
			auto f = LuaPVal(*i); ++i;
			if (i==end)
				throw std::exception("Odd number of values!");


			if (luabind::type(*i)!=LUA_TFUNCTION)
				throw std::exception("Value is not function pointer!");

			parts.emplace_back(f,*i);
		}
		catch (std::exception& e)
		{
			std::string str = std::string("split - Second parameter {val, fun,...} : ") + e.what();
			throw std::exception(str.c_str());
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
		throw std::exception("split - Sum of all percentages must be equal 100%!");

	bvalue = false;

	UnderVector scaleBy,scaleByInv(1);

	switch (by)
	{
	case XX: scaleByInv.X = 0; scaleBy.X = 1; if (totalAbs>getWidth()) bvalue = true; break;
	case YY: scaleByInv.Y = 0; scaleBy.Y = 1; if (totalAbs>getHeight()) bvalue = true; break;
	case ZZ: scaleByInv.Z = 0; scaleBy.Z = 1; if (totalAbs>getDepth()) bvalue = true; break;
	default: throw std::exception("split - First parameter must be one of (x,y,z)!");
	}
	if (bvalue) 
		throw std::exception("split - Sum of all absolute values can not be greater than available space!");

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


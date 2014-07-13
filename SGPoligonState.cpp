#include "stdafx.h"
#include "MainFrm.h"

#include "carve/csg.hpp"

#include "SGCreator.h"
#include "SGPoligonState.h"

namespace CSGProcessor
{


void LogReset(){((CMainFrame*)AfxGetMainWnd())->GetOutputWnd().Clear();}
void Log(const char* file, const char* function, const char* msg,...){
	static char buf[2048]; 
	int len = sprintf_s(buf,"%s:%s ",file,function);
	va_list ap;	va_start (ap, msg);  vsprintf(buf+len,msg,ap); va_end (ap);
	((CMainFrame*)AfxGetMainWnd())->GetOutputWnd().AddString(buf);

}

CSGPoligonState CSGPoligonState::CUBE()		{return CSGPoligonState(std::shared_ptr<carve::poly::Polyhedron>(SGCreator::makeCube()));}
CSGPoligonState CSGPoligonState::SPHERE()	{return CSGPoligonState(std::shared_ptr<carve::poly::Polyhedron>(SGCreator::makeSphere()));}
CSGPoligonState CSGPoligonState::TORUS()	{return CSGPoligonState(std::shared_ptr<carve::poly::Polyhedron>(SGCreator::makeTorus()));}
CSGPoligonState CSGPoligonState::CYLINDER()	{return CSGPoligonState(std::shared_ptr<carve::poly::Polyhedron>(SGCreator::makeCylinder()));}
CSGPoligonState CSGPoligonState::CONE()		{return CSGPoligonState(std::shared_ptr<carve::poly::Polyhedron>(SGCreator::makeCone()));}
CSGPoligonState CSGPoligonState::CONE(int slices){return CSGPoligonState(std::shared_ptr<carve::poly::Polyhedron>(SGCreator::makeCone(slices)));}

CSGPoligonState::CSGPoligonState(void)
	:m_poly(NULL)
{

}

CSGPoligonState::CSGPoligonState( std::shared_ptr<carve::poly::Polyhedron> p )
	:m_poly(p)
{

}


CSGPoligonState::~CSGPoligonState(void)
{
	//LOG_OUTPUT("%X - %d",m_poly.get(), m_poly.use_count());
}

void CSGPoligonState::LuaBind( lua_State* L )
{
	using namespace luabind;

	module(L)
		[
			// real numbers
			class_<CSGPoligonState>("poly")
// 			.def(constructor<>())
// 			.def(constructor<const cl_I&>())
// 			.def(constructor<float>())
// 			.def(constructor<const char*>())
// 			.def(tostring(const_self))
// 			
// 			.def(const_self / const_self)
// 			.def(const_self <= const_self)
// 			.def(const_self < const_self)
// 			.def(const_self == const_self)
// 			.def(other<int>() + const_self)
// 			.def(other<int>() - const_self)
// 			.def(other<int>() * const_self)
// 			.def(other<int>() / const_self)
// 			.def(other<int>() <= const_self)
// 			.def(other<int>() < const_self)
// 			.def(const_self + other<int>())
// 			.def(const_self - other<int>())
// 			.def(const_self * other<int>())
// 			.def(const_self / other<int>())
// 			.def(const_self <= other<int>())
// 			.def(const_self < other<int>())
			//.def(-self)
			.def(const_self + const_self)
			.def(const_self - const_self)
			.def(const_self * const_self)
			.def("invert",&CSGPoligonState::invert)
			.def("scale",&CSGPoligonState::scale)
			.def("move",&CSGPoligonState::move)
			.def("rotate",&CSGPoligonState::rotate)
			.def("contains",&CSGPoligonState::contains)
			.def("clone",&CSGPoligonState::clone)
			.def("clear",&CSGPoligonState::clear)


			,
			def("cube",&CSGPoligonState::CUBE),
			def("sphere",&CSGPoligonState::SPHERE),
			def("torus",&CSGPoligonState::TORUS),
			def("cylinder",&CSGPoligonState::CYLINDER),
			def("cone",(CSGPoligonState(*)())&CSGPoligonState::CONE),
			def("cone",(CSGPoligonState(*)(int))&CSGPoligonState::CONE)
		];
}


CSGPoligonState& CSGPoligonState::invert()
{
	m_poly->invertAll();
	return *this;
}

CSGPoligonState& CSGPoligonState::scale( float x, float y, float z )
{
	m_poly->transform(carve::math::Matrix::SCALE(x,y,z));
	return *this;
}

CSGPoligonState& CSGPoligonState::move( float x, float y, float z )
{
	m_poly->transform(carve::math::Matrix::TRANS(x,y,z));
	return *this;
}

CSGPoligonState& CSGPoligonState::rotate(float angle, float x, float y, float z )
{
	m_poly->transform(carve::math::Matrix::ROT(angle, x,y,z));
	return *this;
}

void CSGPoligonState::transform( const irr::core::matrix4& m )const
{
	carve::math::Matrix mx;
	for(int x=0;x<4;x++)
		for(int y=0;y<4;y++)
			mx.m[y][x] = m[y*4 + x];

	m_poly->transform(mx);
}

bool CSGPoligonState::contains( CSGProcessor::CSGPoligonState const& ps )
{
	if (!m_poly || !ps.m_poly) return false;

	try
	{
		std::shared_ptr<carve::poly::Polyhedron> poly(carve::csg::CSG().compute(m_poly.get(),ps.m_poly.get(),carve::csg::CSG::OP::INTERSECTION));
		return !poly->vertices.empty();
	}catch(const std::exception& e){
		LOG_OUTPUT("Carve error: %s", e.what());
	}catch(...){
		LOG_OUTPUT("Unknown Carve error");
	}
	return false;
}

CSGProcessor::CSGPoligonState CSGPoligonState::clone()const
{
	_ASSERT(m_poly);
	return CSGProcessor::CSGPoligonState(std::make_shared<carve::poly::Polyhedron>(*m_poly.get()));
}

CSGPoligonState operator+( const CSGPoligonState& a,const CSGPoligonState& b )
{
	if (!a.GetPoly()) return b.clone();
	if (!b.GetPoly()) return a.clone();

	try{
		return CSGPoligonState(std::shared_ptr<carve::poly::Polyhedron>(carve::csg::CSG().compute(a.GetPoly(),b.GetPoly(), carve::csg::CSG::OP::UNION)));
	}catch(const std::exception& e){
		LOG_OUTPUT("Carve error: %s", e.what());
	}catch(...){
		LOG_OUTPUT("Unknown Carve error");
	}
	return CSGPoligonState();
}

CSGPoligonState operator-( const CSGPoligonState& a,const CSGPoligonState& b )
{
	try{
		return CSGPoligonState(std::shared_ptr<carve::poly::Polyhedron>(carve::csg::CSG().compute(a.GetPoly(),b.GetPoly(), carve::csg::CSG::OP::A_MINUS_B)));
	}catch(const std::exception& e){
		LOG_OUTPUT("Carve error: %s", e.what());
	}catch(...){
		LOG_OUTPUT("Unknown Carve error");
	}
	return CSGPoligonState();
}

CSGPoligonState operator*( const CSGPoligonState& a,const CSGPoligonState& b )
{
	try{
		return CSGPoligonState(std::shared_ptr<carve::poly::Polyhedron>(carve::csg::CSG().compute(a.GetPoly(),b.GetPoly(), carve::csg::CSG::OP::INTERSECTION)));
	}catch(const std::exception& e){
		LOG_OUTPUT("Carve error: %s", e.what());
	}catch(...){
		LOG_OUTPUT("Unknown Carve error");
	}
	return CSGPoligonState();
}
}
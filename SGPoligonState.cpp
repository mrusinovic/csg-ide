#include "stdafx.h"
//#include "MainFrm.h"

#include "carve/csg.hpp"
//#include "carve/triangle_intersection.hpp"
#include "carve/mesh_simplify.hpp"

#include "SGCreator.h"
#include "SGPoligonState.h"
#include "SGScript.h"
#include "SGTeselator.h"
#include "SGWorldTransform.h"

namespace CSGProcessor
{

#define IMPLEMENT_CONSTRUCTION(name, op)\
	CSGPoligonState CSGPoligonState::name()	{ \
		auto i = CSGScript::GetInstance(); \
		return CSGPoligonState(std::shared_ptr<carve::mesh::MeshSet<3>>(SGCreator::op(i->m_fvTex,i->m_fTexNum,i->m_lastMaterial, i->m_lastMaterialTexX,i->m_lastMaterialTexY))); \
	}\
	CSGPoligonState CSGPoligonState::name(const SGMaterial& m){\
	auto i = CSGScript::GetInstance(); \
	i->m_lastMaterial = m.getLocalInst(); \
	return CSGPoligonState(std::shared_ptr<carve::mesh::MeshSet<3>>(SGCreator::op(i->m_fvTex,i->m_fTexNum,i->m_lastMaterial, i->m_lastMaterialTexX,i->m_lastMaterialTexY))); \
	}\
	CSGPoligonState CSGPoligonState::name(const SGMaterial& m, float tx, float ty){\
		auto i = CSGScript::GetInstance(); \
		i->m_lastMaterial = m.getLocalInst(); i->m_lastMaterialTexX=tx; i->m_lastMaterialTexY=ty; \
		return CSGPoligonState(std::shared_ptr<carve::mesh::MeshSet<3>>(SGCreator::op(i->m_fvTex,i->m_fTexNum,i->m_lastMaterial, i->m_lastMaterialTexX,i->m_lastMaterialTexY))); \
	}\
	CSGPoligonState CSGPoligonState::name(float tx, float ty){\
		auto i = CSGScript::GetInstance(); \
		i->m_lastMaterialTexX=tx; i->m_lastMaterialTexY=ty; \
		return CSGPoligonState(std::shared_ptr<carve::mesh::MeshSet<3>>(SGCreator::op(i->m_fvTex,i->m_fTexNum,i->m_lastMaterial, i->m_lastMaterialTexX,i->m_lastMaterialTexY))); \
	}
	
#define IMPLEMENT_CONSTRUCTION_PAR1(name, op, type, par)\
		CSGPoligonState CSGPoligonState::name(type par)	{ \
		auto i = CSGScript::GetInstance(); \
		return CSGPoligonState(std::shared_ptr<carve::mesh::MeshSet<3>>(SGCreator::op(i->m_fvTex,i->m_fTexNum,i->m_lastMaterial, i->m_lastMaterialTexX,i->m_lastMaterialTexY, par))); \
	}\
		CSGPoligonState CSGPoligonState::name(type par, const SGMaterial& m){\
		auto i = CSGScript::GetInstance(); \
		i->m_lastMaterial = m.getLocalInst(); \
		return CSGPoligonState(std::shared_ptr<carve::mesh::MeshSet<3>>(SGCreator::op(i->m_fvTex,i->m_fTexNum,i->m_lastMaterial, i->m_lastMaterialTexX,i->m_lastMaterialTexY, par))); \
	}\
		CSGPoligonState CSGPoligonState::name(type par, const SGMaterial& m, float tx, float ty){\
		auto i = CSGScript::GetInstance(); \
		i->m_lastMaterial = m.getLocalInst(); i->m_lastMaterialTexX=tx; i->m_lastMaterialTexY=ty; \
		return CSGPoligonState(std::shared_ptr<carve::mesh::MeshSet<3>>(SGCreator::op(i->m_fvTex,i->m_fTexNum,i->m_lastMaterial, i->m_lastMaterialTexX,i->m_lastMaterialTexY, par))); \
	}\
		CSGPoligonState CSGPoligonState::name(type par, float tx, float ty){\
		auto i = CSGScript::GetInstance(); \
		i->m_lastMaterialTexX=tx; i->m_lastMaterialTexY=ty; \
		return CSGPoligonState(std::shared_ptr<carve::mesh::MeshSet<3>>(SGCreator::op(i->m_fvTex,i->m_fTexNum,i->m_lastMaterial, i->m_lastMaterialTexX,i->m_lastMaterialTexY, par))); \
	}

IMPLEMENT_CONSTRUCTION(CUBE,makeCube);
IMPLEMENT_CONSTRUCTION(SPHERE,makeSphere);
//IMPLEMENT_CONSTRUCTION(TORUS,makeTorus);
IMPLEMENT_CONSTRUCTION(CYLINDER,makeCylinder);
IMPLEMENT_CONSTRUCTION(CONE,makeCone);
IMPLEMENT_CONSTRUCTION_PAR1(CONE, makeCone, int, sides);

#undef IMPLEMENT_CONSTRUCTION
#undef IMPLEMENT_CONSTRUCTION_PAR1

CSGPoligonState::CSGPoligonState(void)
{

}

CSGPoligonState::CSGPoligonState( std::shared_ptr<carve::mesh::MeshSet<3>> p )
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
			.def("uvscale",&CSGPoligonState::uvscale)
			.def("uvmove",&CSGPoligonState::uvmove)
			.def("uvswap",&CSGPoligonState::uvswap)

			,
			def("cube",(CSGPoligonState(*)())&CSGPoligonState::CUBE),
			def("sphere",(CSGPoligonState(*)())&CSGPoligonState::SPHERE),
			def("cylinder",(CSGPoligonState(*)())&CSGPoligonState::CYLINDER),
			def("cone",(CSGPoligonState(*)())&CSGPoligonState::CONE),
			def("cone",(CSGPoligonState(*)(int))&CSGPoligonState::CONE),

			def("cube",(CSGPoligonState(*)(const SGMaterial&))&CSGPoligonState::CUBE),
			def("sphere",(CSGPoligonState(*)(const SGMaterial&))&CSGPoligonState::SPHERE),
			def("cylinder",(CSGPoligonState(*)(const SGMaterial&))&CSGPoligonState::CYLINDER),
			def("cone",(CSGPoligonState(*)(const SGMaterial&))&CSGPoligonState::CONE),
			def("cone",(CSGPoligonState(*)(int, const SGMaterial&))&CSGPoligonState::CONE),

			def("cube",(CSGPoligonState(*)(const SGMaterial&,float,float))&CSGPoligonState::CUBE),
			def("sphere",(CSGPoligonState(*)(const SGMaterial&,float,float))&CSGPoligonState::SPHERE),
			def("cylinder",(CSGPoligonState(*)(const SGMaterial&,float,float))&CSGPoligonState::CYLINDER),
			def("cone",(CSGPoligonState(*)(const SGMaterial&,float,float))&CSGPoligonState::CONE),
			def("cone",(CSGPoligonState(*)(int, const SGMaterial&,float,float))&CSGPoligonState::CONE),

			def("cube",(CSGPoligonState(*)(float,float))&CSGPoligonState::CUBE),
			def("sphere",(CSGPoligonState(*)(float,float))&CSGPoligonState::SPHERE),
			def("cylinder",(CSGPoligonState(*)(float,float))&CSGPoligonState::CYLINDER),
			def("cone",(CSGPoligonState(*)(float,float))&CSGPoligonState::CONE),
			def("cone",(CSGPoligonState(*)(int,float,float))&CSGPoligonState::CONE)
		];
}


CSGPoligonState& CSGPoligonState::invert()
{
	m_poly->meshes.front()->invert();
	return *this;
}

CSGPoligonState& CSGPoligonState::scale( float x, float y, float z )
{
	auto s = carve::math::Matrix::SCALE(x,y,z);
	m_poly->transform([s](const carve::geom::vector<3>& v){ return s * v; });

	return *this;
}

CSGPoligonState& CSGPoligonState::move( float x, float y, float z )
{
	auto s = carve::math::Matrix::TRANS(x,y,z);
	m_poly->transform([s](const carve::geom::vector<3>& v){ return s * v; });
	return *this;
}

CSGPoligonState& CSGPoligonState::rotate(float angle, float x, float y, float z )
{
	auto s = carve::math::Matrix::ROT(angle, x,y,z);
	m_poly->transform([s](const carve::geom::vector<3>& v){ return s * v; });
	return *this;
}

void CSGPoligonState::transform( const irr::core::matrix4& m )const
{
	auto s = m.getScale();

	if (s.getVolume()==0)
		SG_THROW("Scale volume == 0 (" << s.X << "," << s.Y << "," << s.Z << ")");

	carve::math::Matrix mx;
	for(int x=0;x<4;x++)
		for(int y=0;y<4;y++)
			mx.m[y][x] = m[y*4 + x];

	m_poly->transform([mx](const carve::geom::vector<3>& v){ return mx * v; });
}

bool CSGPoligonState::contains( CSGProcessor::CSGPoligonState const& ps )
{
	if (!m_poly || !ps.m_poly) return false;

	try
	{
		std::shared_ptr<carve::mesh::MeshSet<3>> poly(carve::csg::CSG().compute(m_poly.get(),ps.m_poly.get(),carve::csg::CSG::OP::INTERSECTION));
		return !poly->meshes.front()->faces.empty();// vertices.empty();
	}catch(const carve::exception& e){
		SG_THROW("Carve error: " << e.str());
	}catch(...){
		SG_THROW("Carve error: Unknown");
	}
	return false;
}

CSGProcessor::CSGPoligonState CSGPoligonState::clone()const
{
	_ASSERT(m_poly);

	auto script = CSGScript::GetInstance();
	
	auto& fvTex = script->m_fvTex;
	auto& fTexNum = script->m_fTexNum;

	auto cl = std::shared_ptr<carve::mesh::MeshSet<3>>(m_poly->clone());

	for(size_t x=0, end = cl->meshes.size();x<end;x++){

		auto& om = m_poly->meshes[x];
		auto& nm = cl->meshes[x];

		for(int y=0, fe = om->faces.size(); y<fe;y++){
			auto& of = om->faces[y];
			auto& nf = nm->faces[y];

			for (auto oe = of->begin(), ne = nf->begin(); oe != of->end(); ++oe, ++ne) {
				fvTex.setAttribute(nf,ne.idx(),fvTex.getAttribute(of, oe.idx()));
			}

			fTexNum.setAttribute(nf, fTexNum.getAttribute(of, nullptr));
		}

	}

	return CSGProcessor::CSGPoligonState(cl);
}

CSGPoligonState operator+( const CSGPoligonState& a,const CSGPoligonState& b )
{
	if (!a.GetPoly() || a.GetPoly()->vertex_storage.empty()) return b;//b.clone();
	if (!b.GetPoly() || b.GetPoly()->vertex_storage.empty()) return a;//a.clone();

	carve::csg::CSG csg;
	auto i = CSGScript::GetInstance();
	i->m_fTexNum.installHooks(csg);
	i->m_fvTex.installHooks(csg);

	//off = 0.0000003;
	a.GetPoly()->transform([](const carve::geom::vector<3>& v){ 
		return carve::geom::VECTOR(irr::core::round_to(v.x,2),irr::core::round_to(v.y,2),irr::core::round_to(v.z,2)); });

	b.GetPoly()->transform([](const carve::geom::vector<3>& v){ 
		return carve::geom::VECTOR(irr::core::round_to(v.x,2),irr::core::round_to(v.y,2),irr::core::round_to(v.z,2)); });

	for(int x=10;x>=0;x--){
		try{

			if (x % 2)
				return CSGPoligonState(std::shared_ptr<carve::mesh::MeshSet<3>>(csg.compute(a.GetPoly().get(),b.GetPoly().get(), carve::csg::CSG::OP::UNION)));
			else
				return CSGPoligonState(std::shared_ptr<carve::mesh::MeshSet<3>>(csg.compute(b.GetPoly().get(),a.GetPoly().get(), carve::csg::CSG::OP::UNION)));

		}catch(const carve::exception& e){
			if (x==0){
				SG_THROW("Carve error: " << e.str());
			}
		}catch(...){
			if (x==0){
				SG_THROW("Carve error: Unknown");
			}
		}
	}
	return CSGPoligonState();
}

CSGPoligonState operator-( const CSGPoligonState& a,const CSGPoligonState& b )
{
	if (!a.GetPoly() || a.GetPoly()->vertex_storage.empty()) return b;//b.clone();
	if (!b.GetPoly() || b.GetPoly()->vertex_storage.empty()) return a;//a.clone();

	carve::csg::CSG csg;
	auto i = CSGScript::GetInstance();
	i->m_fTexNum.installHooks(csg);
	i->m_fvTex.installHooks(csg);

	a.GetPoly()->transform([](const carve::geom::vector<3>& v){ 
		return carve::geom::VECTOR(irr::core::round_to(v.x,2),irr::core::round_to(v.y,2),irr::core::round_to(v.z,2)); });

	b.GetPoly()->transform([](const carve::geom::vector<3>& v){ 
		return carve::geom::VECTOR(irr::core::round_to(v.x,2),irr::core::round_to(v.y,2),irr::core::round_to(v.z,2)); });

	for(int x=10;x>=0;x--){
		try{
			return CSGPoligonState(std::shared_ptr<carve::mesh::MeshSet<3>>(csg.compute(a.GetPoly().get(),b.GetPoly().get(), carve::csg::CSG::OP::A_MINUS_B)));
		}catch(const carve::exception& e){
			if (x==0){
				SG_THROW("Carve error: " << e.str());
			}
		}catch(...){
			if (x==0){
				SG_THROW("Carve error: Unknown");
			}
		}
	}
	return CSGPoligonState();
}

CSGPoligonState operator*( const CSGPoligonState& a,const CSGPoligonState& b )
{
	if (!a.GetPoly() || a.GetPoly()->vertex_storage.empty()) return b;//b.clone();
	if (!b.GetPoly() || b.GetPoly()->vertex_storage.empty()) return a;//a.clone();

	carve::csg::CSG csg;
	auto i = CSGScript::GetInstance();
	i->m_fTexNum.installHooks(csg);
	i->m_fvTex.installHooks(csg);

	a.GetPoly()->transform([](const carve::geom::vector<3>& v){ 
		return carve::geom::VECTOR(irr::core::round_to(v.x,2),irr::core::round_to(v.y,2),irr::core::round_to(v.z,2)); });

	b.GetPoly()->transform([](const carve::geom::vector<3>& v){ 
		return carve::geom::VECTOR(irr::core::round_to(v.x,2),irr::core::round_to(v.y,2),irr::core::round_to(v.z,2)); });

	try{
		return CSGPoligonState(std::shared_ptr<carve::mesh::MeshSet<3>>(csg.compute(a.GetPoly().get(),b.GetPoly().get(), carve::csg::CSG::OP::INTERSECTION)));
	}catch(const carve::exception& e){
		SG_THROW("Carve error: " << e.str());
	}catch(...){
		SG_THROW("Carve error: Unknown");
	}
	return CSGPoligonState();
}

CSGPoligonState& CSGPoligonState::uvscale(float u, float v)
{
	auto script = CSGScript::GetInstance();

	auto& fvTex = script->m_fvTex;
	auto& fTexNum = script->m_fTexNum;

	for(auto& om : m_poly->meshes){
		for(auto& of : om->faces){
			for (auto oe = of->begin(); oe != of->end(); ++oe) {
				auto attr = fvTex.getAttribute(of, oe.idx());
				attr.u*=u; attr.v*=v;
				fvTex.setAttribute(of,oe.idx(),attr);
			}
		}
	}
	return *this;
}

CSGPoligonState& CSGPoligonState::uvmove(float u, float v)
{
	auto script = CSGScript::GetInstance();

	auto& fvTex = script->m_fvTex;
	auto& fTexNum = script->m_fTexNum;

	for(auto& om : m_poly->meshes){
		for(auto& of : om->faces){
			for (auto oe = of->begin(); oe != of->end(); ++oe) {
				auto attr = fvTex.getAttribute(of, oe.idx());
				attr.u+=u; attr.v+=v;
				fvTex.setAttribute(of,oe.idx(),attr);
			}
		}
	}
	return *this;
}

CSGPoligonState& CSGPoligonState::uvswap()
{
	auto script = CSGScript::GetInstance();

	auto& fvTex = script->m_fvTex;
	auto& fTexNum = script->m_fTexNum;

	for(auto& om : m_poly->meshes){
		for(auto& of : om->faces){
			for (auto oe = of->begin(); oe != of->end(); ++oe) {
				auto attr = fvTex.getAttribute(of, oe.idx());
				std::swap(attr.u,attr.v);
				fvTex.setAttribute(of,oe.idx(),attr);
			}
		}
	}
	return *this;
}

CSGPoligonState& CSGPoligonState::simplify()
{

// 	carve::mesh::MeshSimplifier simplifier;
// 
// 	double min_colin = 0.01;
// 	double min_delta_v = 1.0;
// 	double min_norm_angl = M_PI/90.0;//180.0;
// 	double min_len = 0.01;
// 	double min_abs_vol = 1.0;
// 
// 	try{
// 
// 		simplifier.removeFins(m_poly.get());
// 		simplifier.removeLowVolumeManifolds(m_poly.get(), min_abs_vol);
// 
// 		simplifier.mergeCoplanarFaces(m_poly.get(),min_norm_angl);
// 
// 		simplifier.simplify(m_poly.get(), min_colin, min_delta_v, min_norm_angl, min_len);
// 
// 		simplifier.removeFins(m_poly.get());
// 		simplifier.removeLowVolumeManifolds(m_poly.get(), min_abs_vol);
// 
// 		simplifier.improveMesh(m_poly.get(), min_colin, min_delta_v, min_norm_angl);
// 
// 	}catch(const carve::exception& e){		SG_THROW("Carve error: " << e.str());
// 	}catch(...){
// 		SG_THROW("Carve error: Unknown");
// 	}
 	return *this;
}

}
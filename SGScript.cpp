#include "stdafx.h"
#include "SGScript.h"
#include "MainFrm.h"

#include <regex>
#include <strstream>
#include <mutex>

//#include "SGMaterial.h"
#include "SGWorldTransform.h"
#include "..\LibFSRad\RadGen.h"
#include "FsRadProgressDlg.h"
#include "..\LibFSRad\LMapGen.h"
#include "SGTeselator.h"
#include "luabind\exception_handler.hpp"

//#define RADGEN_SCALE	(10)
#define RADGEN_SCALE	(10)

namespace CSGProcessor
{



void LogReset(){((CMainFrame*)AfxGetMainWnd())->GetOutputWnd().Clear();}
void Log(const char* file, const char* function, const char* msg,...){
	static char buf[2048]; 
	int len = 0;//sprintf_s(buf,"%s:%s ",file,function);
	va_list ap;	va_start (ap, msg);  vsprintf(buf+len,msg,ap); va_end (ap);
	((CMainFrame*)AfxGetMainWnd())->GetOutputWnd().AddString(buf);

}

#define LOG_OUTPUT(msg,...)			CSGProcessor::Log(/*__FILE__*/"",""/*__FUNCTION__*/,msg,__VA_ARGS__)
#define LOG_RESET					CSGProcessor::LogReset()

//const int HZ = 0;
//const int VT = 1;
		
std::map<DWORD, CSGScript*> CSGScript::s_instances;
std::mutex s_mux;

CSGScript::CSGScript()
	:m_calcLightMap(true),m_reuseLightmap(false),m_lastMaterial(NULL), m_lastMaterialTexX(1.f), m_lastMaterialTexY(1.f)
{
	std::unique_lock<std::mutex>(s_mux);
	s_instances[GetCurrentThreadId()] = this;

	static struct REGStatic{

		static void translate_exception(lua_State* L, std::exception const& e)
		{
			// log the error message
			luabind::object msg( luabind::from_stack( L, -1 ) );
			std::ostringstream str;
			
			str << e.what();
			try{ str << " " << msg; }catch(...){}

			str << std::endl;

			std::string traceback = luabind::call_function<std::string>( luabind::globals(L)["debug"]["traceback"] );
			str << traceback;
			lua_pushstring(L, str.str().c_str());
		}

		REGStatic(){
			//luabind::set_pcall_callback(add_file_and_line);
			luabind::register_exception_handler<std::exception>(&REGStatic::translate_exception);
		}
	} REGStaticInstance;

}

CSGScript::~CSGScript()
{
	std::unique_lock<std::mutex>(s_mux);
	auto i = s_instances.find(GetCurrentThreadId());
	_ASSERT(i!=s_instances.end());
	s_instances.erase(i);
}

void log_debug(luabind::object x)
{
	std::strstream s;  s << x << '\0';
	CSGProcessor::Log("","",s.str());
}

void debug_break()
{
	_ASSERT(false);
}

void CSGScript::LuaBind(lua_State* L)
{

	luabind::globals(L)["world"] = m_wt.get();

	using namespace luabind;
	module(L)
		[
			def("log",&log_debug),
			def("debug_break",&debug_break)
		];

}


bool CSGScript::Run( const char* script, IFSRadProgress* progress)
{

	carve::setEpsilon(0.001f);

	m_progress = progress;

	m_wt = std::make_shared<CSGWorldTransform>();

	LOG_RESET;

	((CMainFrame*)AfxGetMainWnd())->GetRendererWnd().ClearNodes();

	auto L = lua_open();

	m_lua = L;

	luaL_openlibs(L);

	luabind::open(L);

	CSGWorldTransform::LuaBind(L);

	LuaBind(L);

	CSGPoligonState::LuaBind(L);
	SGMaterial::LuaBind(L);

	bool ret = false;

	try{
		((CMainFrame*)AfxGetMainWnd())->GetRendererWnd().Pause();
		if ( luaL_dostring(L, script) == 0 ){
			PostProcess();
			ret=true;
		}else{
			LOG_OUTPUT("ERROR: %s",lua_tostring(L, -1));
			lua_pop(L, 1); // remove error message
		}
	}catch(std::exception& e){
		LOG_OUTPUT("ERROR: %s",e.what());
	}catch(...){
		LOG_OUTPUT("ERROR: Unknown error!");
	}

	lua_close(L);

	((CMainFrame*)AfxGetMainWnd())->GetRendererWnd().Resume();
//	((CMainFrame*)AfxGetMainWnd())->GetRendererWnd().CombineNodes();

	return ret;
}

CSGScript* CSGScript::GetInstance()
{
	std::unique_lock<std::mutex>(s_mux);
	auto i = s_instances.find(GetCurrentThreadId());
	_ASSERT(i!=s_instances.end());
	return i->second;
}

void CSGScript::PostProcess()
{

	typedef std::map<SGMaterial*, irr::scene::SMeshBufferLightMap*> mesh_map;
	typedef std::vector<mesh_map> mesh_map_vec;

	mesh_map_vec meshMaps;{
		struct rec_convert{
			rec_convert(mesh_map_vec& maps, std::shared_ptr<CSGWorldTransform> wt, CSGScript* sc){
				for(auto& w : wt->GetChildren()) rec_convert(maps,w,sc);
				auto map = SGCreator::ConvertPolyhedronToIrrMesh(wt->GetStaticGeometry().GetPoly(),sc->m_fvTex,sc->m_fTexNum);
				if (!map.empty()) maps.push_back(std::move(map));
			}
		}tmp(meshMaps, m_wt, this);
	}

	auto device = ((CMainFrame*)AfxGetMainWnd())->GetRendererWnd().m_device;


	RadGen radGen;

	radGen.readDefaultParms();
	radGen.clamping() = RadGen::ClampRetain;

	radGen.lightmapWidth()=1024;
	radGen.lightmapHeight()=1024;
 	radGen.uTexelsPerUnit() = 1 / 4.f;
 	radGen.vTexelsPerUnit() = 1 / 4.f;

	auto& geomDb = radGen.geometry();

	size_t noPoly = 0;

	for(auto& mmap : meshMaps)
		for(auto& pair : mmap)
			noPoly += pair.second->getIndexCount()/3;


	geomDb.polys().reserve(noPoly);

	//TODO: ovde bi mogli bit problemi -- TRIBA UVIK BIT SAMO 1
	geomDb.lightmaps().reserve(1);//mb->Indices.size()/3);



	//0 to size_t => irr::scene::SMeshBufferLightMap* (for index 0)
	std::vector< std::pair<size_t, irr::scene::SMeshBufferLightMap*> > mbIndex;

	noPoly = 0;
	for(auto& mmap : meshMaps){
		for(auto& pair : mmap){
			auto mb = pair.second;
			noPoly = AddMeshToRadGen(&radGen,mb, noPoly, pair.first->emisivity, pair.first->reflectivity); 
			mbIndex.emplace_back(noPoly, mb);
			mb->Vertices.clear();
			mb->Indices.clear();
		}
	}

	//run
	if (m_calcLightMap) 
		radGen.go(m_progress);
	else{
		LMapGen	lmg;
		lmg.lightmapWidth() = radGen.lightmapWidth();
		lmg.lightmapHeight() = radGen.lightmapHeight();
		lmg.uTexelsPerUnit() = radGen.uTexelsPerUnit();
		lmg.vTexelsPerUnit() = radGen.vTexelsPerUnit();

		if (!lmg.generate( *m_progress,  geomDb.polys(), geomDb.lightmaps()))
			SG_THROW("Unable to generate lightmap coordinates & lightmaps");
	}

	//convert lightmap to irr image

	auto& lmap = geomDb.lightmaps()[0];

	auto* data = new unsigned char[lmap.width()*lmap.height()*3];

	if (!m_calcLightMap){

		memset(data,255,lmap.width()*lmap.height()*3);

	}else{

		for (size_t x = 0, index = 0, end = lmap.width()*lmap.height();x<end;x++){

			unsigned int	r = static_cast<unsigned int>(lmap.data()[x].r());
			unsigned int	g = static_cast<unsigned int>(lmap.data()[x].g());
			unsigned int	b = static_cast<unsigned int>(lmap.data()[x].b());

			data[index++] = r;
			data[index++] = g;
			data[index++] = b;
		}
	}

	irr::video::ITexture* texture=nullptr;

	//char textureName[32];
	//sprintf_s(textureName,"media/%p.png",m_wt.get());

	const char* textureName = "media/lightmap.png";

	//if (m_reuseLightmap)
	//	texture = device->getVideoDriver()->getTexture(LIGHTMAP_TEXTURE);

	if (!texture){

		auto image = device->getVideoDriver()->createImageFromData(
			irr::video::ECOLOR_FORMAT::ECF_R8G8B8,
			irr::core::dimension2du(lmap.width(), lmap.height()),
			data,true);

		texture = device->getVideoDriver()->addTexture(textureName,image);

		device->getVideoDriver()->writeImageToFile(image,textureName);

		image->drop();
	}

	//copy polys back to the mesh buffers

	size_t pIndex = 0;

	auto currBuff = mbIndex.begin();

	for (RadPrimList::node *i = geomDb.polys().head(); i ; i = i->next(), ++pIndex)
	{
		if (pIndex == 0 || currBuff->first == pIndex){

			if (pIndex) currBuff++;
			auto& mat = currBuff->second->Material;
			mat.setTexture(1,texture);
			mat.MaterialType = irr::video::E_MATERIAL_TYPE::EMT_LIGHTMAP;
			mat.setFlag(irr::video::E_MATERIAL_FLAG::EMF_LIGHTING,false);
			mat.DiffuseColor.set(255,255,255,255);
			mat.EmissiveColor.color=0;
		}

		RadPrim& p = i->data();

		auto num = p.xyz().size();
		auto id = p.polyID();
		auto lid = p.textureID();

		auto cnt = currBuff->second->Vertices.size();

		irr::core::vector3df normal(p.plane().normal().x(),p.plane().normal().y(),p.plane().normal().z());
		normal.normalize();

		// Populate the verts
		for (unsigned int j = 0; j < p.xyz().size(); ++j)
		{
			irr::core::vector2df tv(p.texuv()[j].u(), p.texuv()[j].v());
			irr::core::vector2df lv(p.uv()[j].u() / radGen.lightmapWidth(), p.uv()[j].v() / radGen.lightmapHeight());
			irr::core::vector3df pos(p.xyz()[j].x()/RADGEN_SCALE, p.xyz()[j].y()/RADGEN_SCALE, p.xyz()[j].z()/RADGEN_SCALE);

			currBuff->second->Vertices.push_back(irr::video::S3DVertex2TCoords(pos,normal,irr::video::SColor(255,255,255,255),tv,lv));
			currBuff->second->Indices.push_back(cnt + j);

			//std::stringstream str;
			//str << pos.X << "," << pos.Y <<"," <<  pos.Z << " :: " << lv.X	<< "," << lv.Y << std::endl;
			//OutputDebugStringA(str.str().c_str());

		}
	}

	//TODO: weld what that can be welded

	for(auto& mmap : meshMaps){

		irr::scene::SMesh* mesh = new irr::scene::SMesh;
		for(auto& pair : mmap){
			pair.second->Material.setTexture(0,device->getVideoDriver()->getTexture(pair.first->texture.c_str()));
			pair.second->Material.Lighting=false;		
			mesh->addMeshBuffer(pair.second);
			pair.second->drop();
		}
		mesh->recalculateBoundingBox();

		auto node = ((CMainFrame*)AfxGetMainWnd())->GetRendererWnd().AddNode(mesh,false);

		mesh->drop();
	}

}


size_t CSGScript::AddMeshToRadGen(RadGen* radGen, irr::scene::SMeshBufferLightMap* mb, 
								  size_t polyCnt, 
								  const irr::core::vector3df& lightColor,
								  const irr::core::vector3df& reflectivity)
{
	geom::Color3 lc(lightColor.X,lightColor.Y,lightColor.Z);
	geom::Color3 refl(reflectivity.X,reflectivity.Y,reflectivity.Z);

	if (reflectivity.getLengthSQ()<0.000001)
		refl = radGen->defaultReflectivity();

	if (lightColor.getLengthSQ()<0.000001)
		lc = geom::Color3(0,0,0);

	auto& geomDb = radGen->geometry();

#define IRR_VC3_TO_P3(v) geom::Point3((v).X*RADGEN_SCALE, (v).Y*RADGEN_SCALE, (v).Z*RADGEN_SCALE)
#define IRR_VC2_TO_P2(v) geom::Point2((v).X, (v).Y)

	for (size_t x = 0, end = mb->Indices.size();x<end;x+=3, polyCnt++)
	{

		//entPoly &	p = obj.polys[j];
		RadPrim		poly;

		auto& v1 = mb->Vertices[mb->Indices[x]];
		auto& v2 = mb->Vertices[mb->Indices[x+1]];
		auto& v3 = mb->Vertices[mb->Indices[x+2]];

		//IRR_VC3_TO_P3(v2.Pos);
		//IRR_VC3_TO_P3(v3.Pos);

		poly.xyz() += IRR_VC3_TO_P3(v1.Pos);
		poly.xyz() += IRR_VC3_TO_P3(v2.Pos);
		poly.xyz() += IRR_VC3_TO_P3(v3.Pos);

 		if (v1.Pos == v2.Pos || v1.Pos == v3.Pos || v2.Pos == v3.Pos){
 			polyCnt--;
			continue;
 		}

		poly.uv() += geom::Point2(0,0);
		poly.uv() += geom::Point2(0,0);
		poly.uv() += geom::Point2(0,0);

		poly.texuv() += IRR_VC2_TO_P2(v1.TCoords);
		poly.texuv() += IRR_VC2_TO_P2(v2.TCoords);
		poly.texuv() += IRR_VC2_TO_P2(v3.TCoords);

		poly.textureID() = polyCnt;
		poly.calcPlane(false);

		poly.illuminationColor() = lc;

		poly.reflectanceColor() = radGen->defaultReflectivity();
		
		auto vec = poly.plane().vector();
		
		if (irr::core::round_to(vec.length())>0)
			geomDb.polys() += poly;
		else
			polyCnt--;

	}

#undef IRR_VC3_TO_P3
#undef IRR_VC2_TO_P2


	return polyCnt;

}

void CSGScript::CeanUpGeometry()
{
// 	std::shared_ptr<carve::poly::Polyhedron> totalUnion;
// 	for(auto&  mat : m_materials)
// 		totalUnion = mat.second->CreateTotalUnion(totalUnion);
// 
//  	for(auto&  mat : m_materials)
// 		mat.second->ExtractFromTotalUnion(totalUnion);
// 
// 	
//	render(CSGPoligonState(totalUnion));

}

irr::IrrlichtDevice* CSGScript::GetIrrDevice()
{
	return ((CMainFrame*)AfxGetMainWnd())->GetRendererWnd().m_device;
}


void CSGScript::SetCurWT( CSGWorldTransform* wt )
{
	luabind::globals(m_lua)["world"] = wt;	
}



}
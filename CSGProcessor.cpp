#include "stdafx.h"
#include "CSGProcessor.h"
#include "MainFrm.h"

#include <regex>
#include <strstream>

#include "SGTeselator.h"
#include "SGWorldTransform.h"
//#include "SGPoligonState.h"

namespace CSGProcessor
{

const int HZ = 0;
const int VT = 1;

CSGWorldTransform wt;
		
int render(const CSGPoligonState& a)
{

	if (wt.getOcl().GetPoly()){
		auto mesh = SGCreator::ConvertPolyhedronToIrrMesh((wt.transform(a.clone()) - wt.getOcl()).GetPoly(), CSGWorldTransform::TransMatrix::EM4CONST_IDENTITY);
		if (mesh){
			auto node = ((CMainFrame*)AfxGetMainWnd())->GetRendererWnd().AddNode(mesh);
			mesh->drop();
		}
	}
	else
	{
		auto mesh = SGCreator::ConvertPolyhedronToIrrMesh(a.GetPoly(), wt.get());
		if (mesh){
			auto node = ((CMainFrame*)AfxGetMainWnd())->GetRendererWnd().AddNode(mesh);
			mesh->drop();
		}
	}

	return 0;
}

void log_debug(luabind::object x)
{
	std::strstream s;
	s << x;
	s << '\0';
	CSGProcessor::Log("","",s.str());
}

void LuaBind(lua_State* L)
{
	wt = CSGWorldTransform();

	luabind::globals(L)["hz"] = HZ;
	luabind::globals(L)["vt"] = VT;
	luabind::globals(L)["world"] = &wt;

	using namespace luabind;
	module(L)
		[
			def("render",&render),
			def("log",&log_debug)
		];

}

void Run( const char* script )
{

	LOG_RESET;
 

	((CMainFrame*)AfxGetMainWnd())->GetRendererWnd().ClearNodes();

	auto L = lua_open();
	luaL_openlibs(L);

	luabind::open(L);

	CSGWorldTransform::LuaBind(L);

	LuaBind(L);
	CSGPoligonState::LuaBind(L);

	if ( luaL_dostring(L, script) != 0 ) {
		LOG_OUTPUT("ERROR: %s",lua_tostring(L, -1));
		lua_pop(L, 1); // remove error message
	}

	lua_close(L);

//	((CMainFrame*)AfxGetMainWnd())->GetRendererWnd().CombineNodes();

}


}
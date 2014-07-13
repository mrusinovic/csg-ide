
#include "stdafx.h"

#include "RendererWnd.h"
#include "Resource.h"
#include "CSGProcessor.h"
#include "CMeshCombiner.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar

CRendererWnd::CRendererWnd()
	:m_wireframe(false)
{

}

CRendererWnd::~CRendererWnd()
{
}

BEGIN_MESSAGE_MAP(CRendererWnd, CDockablePane)
	ON_WM_TIMER()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar message handlers


bool CRendererWnd::Initialize3D( )
{
	SIrrlichtCreationParameters params;
	params.AntiAlias=true;
	params.Bits=32;
	params.DriverType= EDT_OPENGL;
	params.EventReceiver=this;
	params.Vsync=true;
	params.DeviceType=EIDT_WIN32;
	params.Doublebuffer=true;
	params.Stencilbuffer=true;

	params.WindowId = (void*)m_wnd.m_hWnd;

	m_device = createDeviceEx(params);

	_ASSERT(m_device);

	m_driver = m_device->getVideoDriver();
	m_smgr = m_device->getSceneManager();

	auto cam = m_smgr->addCameraSceneNodeMaya(NULL,-50.f,1.f,5.f,1,5);
	
	cam->setNearValue(0.1f);
	cam->setFarValue(100);

	cam->setTarget(vector3df(0,0,0));
	cam->setRotation(vector3df(180,0,0));

	class MyShaderCallBack : public irr::video::IShaderConstantSetCallBack
	{
	public:

		virtual void OnSetConstants(irr::video::IMaterialRendererServices* services, irr::s32 userData)
		{
			static const irr::s32 t[] = {0,1,2,3};
			irr::video::IVideoDriver* driver = services->getVideoDriver();
			services->setPixelShaderConstant("baseMap", (irr::f32*)&t[0], 1);
			services->setPixelShaderConstant("secondMap", (irr::f32*)&t[1], 1);
		}
	};

	auto scb = new MyShaderCallBack();

	m_nodeMaterial = m_device->getVideoDriver()->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
		"..\\media\\shaders\\terrain.vert", "main", irr::video::EVST_VS_1_1,
		"..\\media\\shaders\\terrain.frag", "main", irr::video::EPST_PS_1_1,
		scb, irr::video::EMT_SOLID);

	scb->drop();

	return true;
}


int CRendererWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rc(0,0,lpCreateStruct->cx,lpCreateStruct->cy);

	m_wnd.Create(AfxRegisterWndClass(0),NULL,WS_CHILD,rc,this,1);
	m_wnd.ShowWindow(SW_SHOW);

	Initialize3D();
	SetTimer(1,250, NULL);

	return 0;
}

void CRendererWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	m_wnd.SetWindowPos(NULL,0,0,cx,cy,SWP_NOMOVE|SWP_NOREDRAW|SWP_NOZORDER);
}

void CRendererWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
}

bool CRendererWnd::OnEvent( const SEvent& event )
{
	return false;
}

void CRendererWnd::OnTimer( UINT_PTR nIDEvent )
{
	Render();
}

void CRendererWnd::Render()
{
	m_device->getTimer()->tick();

	m_driver->beginScene(true, true, video::SColor(0,0,0,0));

	m_smgr->drawAll();


	auto ray = m_smgr->getSceneCollisionManager()->getRayFromScreenCoordinates(vector2di(150, m_driver->getViewPort().getHeight() - 150), m_smgr->getActiveCamera());

	const vector3df& vv = m_smgr->getActiveCamera()->getPosition();	
	auto pt = ray.getClosestPoint(vv + (m_smgr->getActiveCamera()->getTarget() - vv).normalize()*10);

	matrix4 tr;
	tr.setTranslation(pt);
	m_driver->setTransform (ETS_WORLD, tr );

	SMaterial m;
	m.Lighting=false;
	m_driver->setMaterial(m);

	float sz = 1.f;

	m_driver->draw3DLine(vector3df (-sz, 0, 0), vector3df (0, 0, 0), SColor (255, 155, 0, 0));
	m_driver->draw3DLine(vector3df (0, 0, 0), vector3df (sz, 0, 0), SColor (255, 255, 0, 0));

	m_driver->draw3DLine(vector3df (0, -sz, 0), vector3df (0, 0, 0), SColor (255, 0, 155, 0));
	m_driver->draw3DLine(vector3df (0, 0, 0), vector3df (0, sz, 0), SColor (255, 0, 255, 0));

	m_driver->draw3DLine(vector3df (0, 0, -sz), vector3df (0, 0, 0), SColor (255, 0, 0, 155));
	m_driver->draw3DLine(vector3df (0, 0, 0), vector3df (0, 0, sz), SColor (255, 0, 0, 255));

	//DrawCursorPickingResponse();

	//m_gui->drawAll();
	m_driver->endScene();

}


void CRendererWnd::OnDestroy()
{
	KillTimer(1);
	__super::OnDestroy();
	m_device->closeDevice();
	m_device->drop();
}


void CRendererWnd::ClearNodes()
{
	auto n = m_device->getSceneManager()->getRootSceneNode()->getChildren();

	for(auto i = n.begin();i!=n.end();i++){
		if ((*i)->getID()<0) 
			(*i)->remove();
	}
	m_nodes.clear();
}

void CRendererWnd::SetMaterial(IMeshSceneNode* node)
{
	node->setMaterialFlag(irr::video::E_MATERIAL_FLAG::EMF_LIGHTING,false);
	node->setMaterialFlag(irr::video::E_MATERIAL_FLAG::EMF_WIREFRAME,m_wireframe);
	node->setMaterialTexture(0, m_device->getVideoDriver()->getTexture("..\\media\\White.jpg"));
	node->setMaterialTexture(1, m_device->getVideoDriver()->getTexture("..\\media\\White.jpg"));
	node->setMaterialType((irr::video::E_MATERIAL_TYPE)m_nodeMaterial);
	//node->setMaterialFlag(irr::video::E_MATERIAL_FLAG::EMF_BACK_FACE_CULLING,false);

}


irr::scene::IMeshSceneNode* CRendererWnd::AddNode( irr::scene::IMesh* mesh )
{
	auto node = m_device->getSceneManager()->addMeshSceneNode(mesh);
	SetMaterial(node);
	m_nodes.push_back(node);
	return node;
}


void CRendererWnd::OnViewWireframe()
{
	m_wireframe = !m_wireframe;
	auto lst = m_device->getSceneManager()->getRootSceneNode()->getChildren();

	for(auto n = lst.begin();n!=lst.end();n++){
		(*n)->setMaterialFlag(irr::video::E_MATERIAL_FLAG::EMF_WIREFRAME,m_wireframe);
	}
}

void CRendererWnd::CombineNodes()
{
	if (m_nodes.empty()) return;

	auto node = m_smgr->addMeshSceneNode(CMeshCombiner().combineMeshes(m_driver,m_nodes));
	SetMaterial(node);
	m_nodes.clear();
	m_nodes.push_back(node);

}


#include "stdafx.h"

#include "RendererWnd.h"
#include "Resource.h"
#include "SGScript.h"
#include "CMeshCombiner.h"
#include "MainFrm.h"
#include <thread>

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
	:m_wireframe(false),m_runDemo(false),m_anim(NULL)
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
	ON_MESSAGE_VOID(WM_USER+1,Initialize3D)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar message handlers


void CRendererWnd::InitializeDesignDevice()
{

	auto cam = m_device->getSceneManager()->addCameraSceneNodeMaya(NULL,-50.f,1.f,5.f,1,5);

	cam->setNearValue(0.1f);
	cam->setFarValue(10000);

	cam->setTarget(vector3df(0,0,0));
	cam->setRotation(vector3df(180,0,0));
}

void CRendererWnd::InitializeResources()
{
	class MyShaderCallBack : public irr::video::IShaderConstantSetCallBack
	{
	public:

		virtual void OnSetConstants(irr::video::IMaterialRendererServices* services, irr::s32 userData)
		{
			static const irr::f32 t[] = {0,1,2,3};
			irr::video::IVideoDriver* driver = services->getVideoDriver();
			services->setPixelShaderConstant("baseMap", (irr::f32*)&t[0], 1);
			services->setPixelShaderConstant("secondMap", (irr::f32*)&t[1], 1);
		}
	};

	auto scb = new MyShaderCallBack();

	m_nodeMaterial = m_device->getVideoDriver()->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
		"media\\shaders\\terrain.vert", "main", irr::video::EVST_VS_1_1,
		"media\\shaders\\terrain.frag", "main", irr::video::EPST_PS_1_1,
		scb, irr::video::EMT_SOLID);

	scb->drop();

	//////////////////////////////////////////////////////////////////////////

	class MyShaderCallBack1 : public irr::video::IShaderConstantSetCallBack
	{
	public:

		virtual void OnSetConstants(irr::video::IMaterialRendererServices* services, irr::s32 userData)
		{
			static const irr::f32 t[] = {0,1,2,3};
			irr::video::IVideoDriver* driver = services->getVideoDriver();
			services->setPixelShaderConstant("difuseMap", (irr::f32*)&t[0], 1);
			services->setPixelShaderConstant("lightMap", (irr::f32*)&t[1], 1);
			services->setPixelShaderConstant("normalMap", (irr::f32*)&t[2], 1);
		}
	};

	auto scb1 = new MyShaderCallBack1();

	m_nodeMaterialBump = m_device->getVideoDriver()->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
		"media\\shaders\\bumpmap.vert", "main", irr::video::EVST_VS_1_1,
		"media\\shaders\\bumpmap.frag", "main", irr::video::EPST_PS_1_1,
		scb1, irr::video::EMT_SOLID);

	scb1->drop();

}

void CRendererWnd::Initialize3D( )
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

	InitializeDesignDevice();
	InitializeResources();

}


int CRendererWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rc(0,0,lpCreateStruct->cx,lpCreateStruct->cy);

	m_wnd.Create(AfxRegisterWndClass(0),NULL,WS_CHILD,rc,this,1);
	m_wnd.ShowWindow(SW_SHOW);

	Initialize3D();
	Resume();

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
	m_wnd.SetFocus();
}

bool CRendererWnd::OnEvent( const SEvent& event )
{

	if (event.EventType==EEVENT_TYPE::EET_KEY_INPUT_EVENT){
		if (event.KeyInput.Key==EKEY_CODE::KEY_ESCAPE)
			m_runDemo=false;
		if (m_runDemo && event.KeyInput.Key == EKEY_CODE::KEY_KEY_G && !event.KeyInput.PressedDown){
			ToggleCameraAnimation();
		}
	}

	if (event.EventType==EEVENT_TYPE::EET_MOUSE_INPUT_EVENT){
		if (m_runDemo)
			m_wnd.SetFocus();
		else{
			if (event.MouseInput.Event==EMOUSE_INPUT_EVENT::EMIE_LMOUSE_DOUBLE_CLICK){
				m_runDemo=true;
				RunDemo();
			}
		}
	}

	return false;
}

void CRendererWnd::OnTimer( UINT_PTR nIDEvent )
{
	Render();
}

void CRendererWnd::Render()
{
	m_device->getTimer()->tick();

	auto driver = m_device->getVideoDriver();
	auto smgr  = m_device->getSceneManager();

	driver->beginScene(true, true, video::SColor(0,0,0,0));

	smgr->drawAll();


	auto ray = smgr->getSceneCollisionManager()->getRayFromScreenCoordinates(vector2di(150, driver->getViewPort().getHeight() - 150), smgr->getActiveCamera());

	const vector3df& vv = smgr->getActiveCamera()->getPosition();	
	auto pt = ray.getClosestPoint(vv + (smgr->getActiveCamera()->getTarget() - vv).normalize()*10);

	matrix4 tr;
	tr.setTranslation(pt);
	driver->setTransform (ETS_WORLD, tr );

	SMaterial m;
	m.Lighting=false;
	driver->setMaterial(m);

	float sz = 1.f;

	driver->draw3DLine(vector3df (-sz, 0, 0), vector3df (0, 0, 0), SColor (255, 155, 0, 0));
	driver->draw3DLine(vector3df (0, 0, 0), vector3df (sz, 0, 0), SColor (255, 255, 0, 0));

	driver->draw3DLine(vector3df (0, -sz, 0), vector3df (0, 0, 0), SColor (255, 0, 155, 0));
	driver->draw3DLine(vector3df (0, 0, 0), vector3df (0, sz, 0), SColor (255, 0, 255, 0));

	driver->draw3DLine(vector3df (0, 0, -sz), vector3df (0, 0, 0), SColor (255, 0, 0, 155));
	driver->draw3DLine(vector3df (0, 0, 0), vector3df (0, 0, sz), SColor (255, 0, 0, 255));

	//DrawCursorPickingResponse();

	//gui->drawAll();
	driver->endScene();

}


void CRendererWnd::OnDestroy()
{
	Pause();
	__super::OnDestroy();

	m_device->closeDevice();
	m_device->drop();
}


void CRendererWnd::ClearNodes()
{
	auto n = m_device->getSceneManager()->getRootSceneNode()->getChildren();
	
	auto cam = m_device->getSceneManager()->getActiveCamera();

	for(auto i = n.begin();i!=n.end();i++){
		if ((*i)!=cam)  (*i)->remove();
	}


	auto driver = m_device->getVideoDriver();
	auto smgr = m_device->getSceneManager();

	driver->removeAllTextures();

	driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);

	scene::ISceneNode* skybox=smgr->addSkyBoxSceneNode(
		driver->getTexture("media/sb_night_up.jpg"),
		driver->getTexture("media/sb_night_dn.jpg"),
		driver->getTexture("media/sb_night_lf.jpg"),
		driver->getTexture("media/sb_night_rt.jpg"),
		driver->getTexture("media/sb_night_ft.jpg"),
		driver->getTexture("media/sb_night_bk.jpg"));
/*
	scene::ISceneNode* skybox=smgr->addSkyBoxSceneNode(
		driver->getTexture("media/sb_day_up.jpg"),
		driver->getTexture("media/sb_day_dn.jpg"),
		driver->getTexture("media/sb_day_lf.jpg"),
		driver->getTexture("media/sb_day_rt.jpg"),
		driver->getTexture("media/sb_day_ft.jpg"),
		driver->getTexture("media/sb_day_bk.jpg"));
*/
	//scene::ISceneNode* skydome = smgr->addSkyDomeSceneNode(driver->getTexture("media/skydome_stardust_sky.jpg"),16,8,0.99f,2,1500.f);
	driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);

}

void CRendererWnd::SetMaterial(irr::scene::IMeshSceneNode* node)
{
	node->setMaterialFlag(irr::video::E_MATERIAL_FLAG::EMF_LIGHTING,false);
	node->setMaterialFlag(irr::video::E_MATERIAL_FLAG::EMF_WIREFRAME,m_wireframe);
	node->setMaterialTexture(0, m_device->getVideoDriver()->getTexture("media\\White.jpg"));
	node->setMaterialTexture(1, m_device->getVideoDriver()->getTexture("media\\White.png"));
	node->setMaterialType((irr::video::E_MATERIAL_TYPE)m_nodeMaterial);

}


irr::scene::IMeshSceneNode* CRendererWnd::AddNode( irr::scene::IMesh* mesh, bool setMaterial )
{

	auto node = m_device->getSceneManager()->addMeshSceneNode(mesh,NULL,1);
	if (setMaterial) 
		SetMaterial(node);
	else {
	//	node->setMaterialType((irr::video::E_MATERIAL_TYPE)m_nodeMaterialBump);
	//	auto tex = m_device->getVideoDriver()->getTexture("media\\nwall_bump.bmp");
	//	m_device->getVideoDriver()->makeNormalMapTexture(tex);
	//	node->setMaterialTexture(2, tex);
	}

//	node->setMaterialFlag(irr::video::E_MATERIAL_FLAG::EMF_BACK_FACE_CULLING,false);
	if (m_wireframe)
		node->setMaterialFlag(irr::video::E_MATERIAL_FLAG::EMF_WIREFRAME,m_wireframe);

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
	_ASSERT(FALSE);//clean m_meshes
//	if (m_nodes.empty()) return;

//	auto node = m_smgr->addMeshSceneNode(CMeshCombiner().combineMeshes(m_driver,m_nodes));
//	SetMaterial(node);
//	m_nodes.clear();
//	m_nodes.push_back(node);
}

void CRendererWnd::Resume()
{
	SetTimer(1,250, NULL);
}

void CRendererWnd::Pause()
{
	KillTimer(1);
}

void CRendererWnd::RunDemo()
{
	Pause();

	auto driver = m_device->getVideoDriver();
	auto smgr = m_device->getSceneManager();

	UndockPane();
	FloatPane(CRect(0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN)));

	auto cam = smgr->getActiveCamera();
	auto pos = cam->getAbsolutePosition();
	cam->remove();

	auto camera = smgr->addCameraSceneNodeFPS(NULL,50,0.004,-1,0,0,false,0.25);
	camera->setNearValue(0.1f);

	camera->setPosition(pos);

	m_device->getCursorControl()->setVisible(false);

	while (m_device->run() && m_runDemo) {

		driver->beginScene(true, true, video::SColor(255,0,0,0));
		smgr->drawAll();
		driver->endScene();

		for (int i = 0; i < 10; i++){
			MSG	msg;
			if (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	m_device->getCursorControl()->setVisible(true);
	m_anim=NULL;

 	smgr->getActiveCamera()->remove();
 	InitializeDesignDevice();
 
 	//DockToFrameWindow(CBRS_ALIGN_RIGHT);
	DockToRecentPos();
	Resume();
}


BOOL CRendererWnd::PreTranslateMessage(MSG* pMsg)
{
	return FALSE;
	/*
	BOOL ret = __super::PreTranslateMessage(pMsg);
	if (!ret){
		if (pMsg->message==WM_KEYDOWN) 
			ret=TRUE;
	}
	return ret;
	*/
}

void CRendererWnd::ToggleCameraAnimation()
{
	auto driver = m_device->getVideoDriver();
	auto smgr = m_device->getSceneManager();
	auto children = smgr->getRootSceneNode()->getChildren();
	if (children.size()<2) return;

	auto camera = smgr->getActiveCamera();
	if (m_anim){
		camera->removeAnimator(m_anim);
		m_anim=NULL;
		return;
	}

	auto& nodes = smgr->getRootSceneNode()->getChildren();
	auto metaSelector = smgr->createMetaTriangleSelector();

	for(auto i = nodes.begin();i!=nodes.end();i++){

		if ((*i)->getID()>0){
		
			auto node = (IMeshSceneNode*)(*i);

			auto selector = smgr->createOctreeTriangleSelector(node->getMesh(), node, 128);
			metaSelector->addTriangleSelector(selector);
			selector->drop();
		}
	}

	m_anim = smgr->createCollisionResponseAnimator(
		metaSelector, camera, core::vector3df(0.5,1,0.5),
		core::vector3df(0,-1,0), core::vector3df(0,0.7,0),0.005f);
	metaSelector->drop();
	camera->addAnimator(m_anim);

	m_anim->drop();
}

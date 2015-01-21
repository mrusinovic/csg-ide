
#pragma once

class CRendererWnd : public CDockablePane, irr::IEventReceiver
{


// Construction
public:
	CRendererWnd();

	irr::IrrlichtDevice* m_device;
	irr::scene::ISceneNodeAnimator* m_anim;
	//	irr::video::IVideoDriver* m_driver;
//	irr::scene::ISceneManager* m_smgr;

// Attributes
protected:
	int m_nodeMaterial,m_nodeMaterialBump;
	bool m_wireframe;
	CWnd m_wnd;
	bool m_runDemo;

	virtual BOOL OnShowControlBarMenu(CPoint point){return FALSE;}
	virtual void OnPaneContextMenu(CWnd* pParentFrame, CPoint point){}

	void Initialize3D();

	void InitializeResources();
	void InitializeDesignDevice();

	void RunDemo();
	void ToggleCameraAnimation();

	virtual bool OnEvent( const irr::SEvent& event );
	void SetMaterial(irr::scene::IMeshSceneNode* node);
// Implementation
public:
	virtual ~CRendererWnd();

	void Render();
	irr::scene::IMeshSceneNode* AddNode(irr::scene::IMesh* mesh, bool setMaterial = true);
	void ClearNodes();
	void CombineNodes();
	void OnViewWireframe();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnDestroy();
	void Resume();
	void Pause();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};


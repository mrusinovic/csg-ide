
#pragma once

class CRendererWnd : public CDockablePane, irr::IEventReceiver
{
// Construction
public:
	CRendererWnd();

// Attributes
protected:
	irr::IrrlichtDevice* m_device;
	irr::video::IVideoDriver* m_driver;
	irr::scene::ISceneManager* m_smgr;
	int m_nodeMaterial;
	bool m_wireframe;
	CWnd m_wnd;
	irr::core::array<irr::scene::IMeshSceneNode*> m_nodes;

	bool Initialize3D();

	virtual bool OnEvent( const irr::SEvent& event );
	void SetMaterial(irr::scene::IMeshSceneNode* node);
// Implementation
public:
	virtual ~CRendererWnd();

	void Render();
	irr::scene::IMeshSceneNode* AddNode(irr::scene::IMesh* mesh);
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
};


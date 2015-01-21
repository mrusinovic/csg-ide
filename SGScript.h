#pragma once

class RadGen;

//#include "SGMaterial.h"
#include "..\LibFSRad\IFSRadProgress.h"
#include "SGMaterial.h"

namespace CSGProcessor
{
	class CSGWorldTransform;

	class CSGScript
	{
		static std::map<DWORD, CSGScript*> s_instances;

		void LuaBind(lua_State* L);

		std::shared_ptr<CSGWorldTransform> m_wt;

		lua_State* m_lua;

		std::map<INT_PTR, std::shared_ptr<SGMaterial> > m_materials;

		void PostProcess();

		size_t AddMeshToRadGen(RadGen* radGen, irr::scene::SMeshBufferLightMap* mb, 
			size_t polyCnt, 
			const irr::core::vector3df& lightColor,
			const irr::core::vector3df& reflectivity);

		IFSRadProgress* m_progress;

		void CeanUpGeometry();


	public:



		CarveVertexTexCoords m_fvTex;
		CarveFaceMaterials m_fTexNum;
		SGMaterial* m_lastMaterial;
		float m_lastMaterialTexX, m_lastMaterialTexY;

		bool m_calcLightMap, m_reuseLightmap;

		CSGScript();
		~CSGScript();


		static CSGScript* GetInstance();

		irr::IrrlichtDevice* GetIrrDevice();


		inline lua_State* GetLuaState() {return m_lua;}
		inline std::shared_ptr<CSGWorldTransform> GetWorldTransform(){return m_wt;}

		inline std::shared_ptr<SGMaterial> GetMaterial(INT_PTR id){
			auto i = m_materials.find(id);
			if (i!=m_materials.end()) return i->second;
			return nullptr;
		}

		inline void SetMaterial(INT_PTR id, const std::shared_ptr<SGMaterial>& m){
			_ASSERT(m_materials.find(id)==m_materials.end());
			m_materials[id] = m;
		}

		void SetCurWT(CSGWorldTransform* wt);

		bool Run(const char* script, IFSRadProgress* progress);
	};

};


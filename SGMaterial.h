#pragma once

#include "carve/interpolator.hpp"
#include "SGLuaVector.h"

namespace CSGProcessor
{

struct SGMaterial{

	static std::shared_ptr<SGMaterial> invisibleMaterial;

	std::string texture;
	irr::core::vector3df reflectivity, emisivity;

	SGMaterial(void){}
	SGMaterial(const char* tex):texture(tex){}
	SGMaterial(const char* tex, const SGLuaVector& refl):texture(tex), reflectivity(refl.uv){}
	SGMaterial(const char* tex, const SGLuaVector& refl, const SGLuaVector& emis):texture(tex), reflectivity(refl.uv), emisivity(emis.uv){}
	SGMaterial(const SGMaterial& m, const SGLuaVector& emis):texture(m.texture), reflectivity(m.reflectivity), emisivity(emis.uv){}

	SGMaterial* getLocalInst() const;



	static void LuaBind(lua_State* L);
};


struct SGTextureAttrCarve {
	float u;
	float v;

	SGTextureAttrCarve() : u(0.0f), v(0.0f) { }
	SGTextureAttrCarve(float _u, float _v) : u(_u), v(_v) { }
};

SGTextureAttrCarve operator*(double s, const SGTextureAttrCarve &t);
SGTextureAttrCarve &operator+=(SGTextureAttrCarve &t1, const SGTextureAttrCarve &t2);

typedef carve::interpolate::FaceVertexAttr<SGTextureAttrCarve> CarveVertexTexCoords;
typedef carve::interpolate::FaceAttr<SGMaterial*> CarveFaceMaterials;

}
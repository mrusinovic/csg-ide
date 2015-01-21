#include "stdafx.h"
#include "SGMaterial.h"
#include "SGScript.h"

namespace CSGProcessor
{

SGTextureAttrCarve operator*(double s, const SGTextureAttrCarve &t) {
	return SGTextureAttrCarve(t.u * s, t.v * s);
}

SGTextureAttrCarve &operator+=(SGTextureAttrCarve &t1, const SGTextureAttrCarve &t2) {
	t1.u += t2.u;
	t1.v += t2.v;
	return t1;
}

std::shared_ptr<SGMaterial> SGMaterial::invisibleMaterial = std::make_shared<SGMaterial>();

void SGMaterial::LuaBind( lua_State* L )
{
	using namespace luabind;

	module(L)
		[
			class_<SGMaterial>("material")
			.def(constructor<const char*>())
			.def(constructor<const char*, const SGLuaVector&>())
			.def(constructor<const char*, const SGLuaVector&, const SGLuaVector&>())
			.def(constructor<const SGMaterial&, const SGLuaVector&>())

			// 			.def(constructor<const cl_I&>())
			// 			.def(constructor<float>())
			// 			.def(constructor<const char*>())
			// 			.def(tostring(const_self))
			//			.def("render",&CSGMaterial::SGLuaMaterial::render)
			//			.def("light",&CSGMaterial::SGLuaMaterial::light)
		];

	luabind::globals(L)["INVISIBLE"] = invisibleMaterial.get();

	CSGScript::GetInstance()->SetMaterial((INT_PTR)invisibleMaterial.get(),invisibleMaterial);

}

SGMaterial* SGMaterial::getLocalInst() const
{
	auto i = CSGScript::GetInstance();
	auto m = i->GetMaterial((INT_PTR)this);

	if (!m){
		m = std::make_shared<SGMaterial>(*this);
		i->SetMaterial((INT_PTR)this,m);
	}

	return m.get();
}



}
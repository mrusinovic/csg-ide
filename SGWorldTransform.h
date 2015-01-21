#pragma once

#include "SGLuaVector.h"
//#include "SGLuaLight.h"
#include "SGPoligonState.h"
namespace CSGProcessor
{

// struct SLight
// {
// 	irr::scene::IMesh* mesh;
// 	UnderVector color;
// 	SLight(irr::scene::IMesh* m, const UnderVector& c):mesh(m), color(c){}
// };

class CSGWorldTransform
{
public:
	typedef irr::core::matrix4 TransMatrix;
private:
	struct v3{
		UnderVector r,s,t;

		void makeIdentity(){r.set(0,0,0); s.set(1,1,1); t.set(0,0,0);}

		TransMatrix get()const{
			TransMatrix mat;
			mat.setRotationRadians(r);
			mat.setTranslation(t);

			if (s != UnderVector(1.f,1.f,1.f)){
				TransMatrix smat;
				smat.setScale(s);
				mat *= smat;
			}
			return mat;
		}
	};

	v3 matrix, m_initial;
	std::vector<v3> m_stack;

// 	std::vector<SLight> m_lights;

	UnderVector m_dimStart;
	CSGProcessor::CSGPoligonState m_ocl;
	CSGPoligonState m_staticGeometry;
	CSGWorldTransform* m_parent;
	
	std::vector<std::shared_ptr<CSGWorldTransform>> m_children;

	inline float getWidth()const{return abs(get().getScale().X);}
	inline float getHeight()const{return abs(get().getScale().Y);}
	inline float getDepth()const{return abs(get().getScale().Z);}

	inline CSGProcessor::CSGPoligonState const& setOcl(CSGProcessor::CSGPoligonState const& o){m_ocl = o; return m_ocl;}

	UnderVector ConvertVector(SGLuaVector const& v);

public:
	CSGWorldTransform(void);
	~CSGWorldTransform(void);

	static void LuaBind(lua_State* L);

	inline std::vector<std::shared_ptr<CSGWorldTransform>> const& GetChildren()const{return m_children;}

	inline CSGPoligonState const& GetStaticGeometry()const{return m_staticGeometry;}

// 	inline std::vector<SLight> const& GetLights(){return m_lights;}

	inline CSGPoligonState const& getOcl()const{return m_ocl;}

	TransMatrix get()const;

	void init(float x, float y, float z);

	void render(const CSGPoligonState& a);
	void render(const CSGPoligonState& a, bool transform);
	void invert(){m_staticGeometry.invert();}

	void push(){m_stack.push_back(matrix); matrix.makeIdentity(); }
	void pop(){matrix = m_stack.back(); m_stack.pop_back(); }
	void rotate(const SGLuaVector& v);

	void select_sides(luabind::object const& fun);
	void select_side(int side, luabind::object const& fun);
	void divide(int x, int y, int z, luabind::object const& fun);
	
	void split(int by, luabind::object const& table);

	void size(const SGLuaVector& v);
	void scale(const SGLuaVector& v);
	void scaleby(const SGLuaVector& v);
	void move(const SGLuaVector& v);

	CSGWorldTransform* make_child();
	CSGWorldTransform* get_parent();
//	void light(CSGProcessor::CSGPoligonState const& ps, const SGLuaVector& color);
	

	bool ocluded();

	void to_side(int x, int y, int z, bool faceOut=false);

 	CSGProcessor::CSGPoligonState const& transform(CSGProcessor::CSGPoligonState const& ps);


};
/*
class CSGWorldTransformRoot{
	CSGWorldTransform m_wt;
	CSGWorldTransform* m_cur;
	CSGWorldTransformRoot():m_cur(&m_wt){}

	CSGWorldTransform& get(){return m_wt;}
	CSGWorldTransform& cur(){return *m_cur;}
	CSGWorldTransform& child(){_ASSERT(m_cur); m_cur = m_cur->create(); return *m_cur;}
	CSGWorldTransform& prent(){_ASSERT(m_cur); m_cur = m_cur->parent(); return *m_cur;}

};
*/
}
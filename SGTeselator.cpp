#include "stdafx.h"
#include "SGTeselator.h"

#include "carve/carve.hpp"
#include "carve/mesh.hpp"
#include "carve/matrix.hpp"

#include <gl/GLU.h>
//#include "SGScript.h"

namespace CSGProcessor
{

namespace SGCreator
{

inline static irr::core::vector3df ToIrrVect(const double *v){return irr::core::vector3df((float)v[0],(float)v[1],(float)v[2]);}

static void WINAPI _tessBegin(GLenum type, void *data) {}
static void WINAPI _tessEnd(void *data) {}
static void WINAPI _tessEdgeFlag( GLboolean ) {}

static void WINAPI _tessError(GLenum type, void *data) 
{
	ASSERT(FALSE);
}


static void WINAPI _tessCombine(GLdouble coords[3], void* vertex_data[4], GLfloat weight[4], void** outData, irr::scene::SMeshBufferLightMap* buf )
{
	irr::core::vector3df n;
	irr::core::vector2df tx;

	for(int x=0;x<4 && vertex_data[x];x++){
		short index = (short)(vertex_data[x])-1;
		n += buf->Vertices[index].Normal;

		tx += buf->Vertices[index].TCoords * weight[x];
	}

// 	std::stringstream str;
// 	str << tx.X << "," << tx.Y << std::endl;
// 	OutputDebugStringA(str.str().c_str());

	buf->Vertices.push_back(irr::video::S3DVertex2TCoords(ToIrrVect(coords),n.normalize(),irr::video::SColor(255,255,255,255),tx));
	*outData = (void*)buf->Vertices.size();
}

static void WINAPI _tessVertex(void *vertex_data, irr::scene::SMeshBufferLightMap* buf) 
{
	buf->Indices.push_back(((short)vertex_data)-1);
}

void Teselate(const std::vector<irr::video::S3DVertex2TCoords>& vertices,const irr::core::vector3df& normal, irr::scene::SMeshBufferLightMap* buf)
{
	GLUtesselator *tess = gluNewTess();

	gluTessCallback( tess, GLU_TESS_BEGIN_DATA,     (void (CALLBACK *)())   _tessBegin      );
	gluTessCallback( tess, GLU_TESS_EDGE_FLAG,      (void (CALLBACK *)())   _tessEdgeFlag   );
	gluTessCallback( tess, GLU_TESS_END_DATA,       (void (CALLBACK *)())   _tessEnd        );
	gluTessCallback( tess, GLU_TESS_COMBINE_DATA,   (void (CALLBACK *)())   _tessCombine    );
	gluTessCallback( tess, GLU_TESS_ERROR_DATA,		(void (CALLBACK *)())   _tessError      );
	gluTessCallback( tess, GLU_TESS_VERTEX_DATA,    (void (CALLBACK *)())   _tessVertex     );


	gluTessBeginPolygon(tess, (void *)buf);
	gluTessBeginContour(tess);

	int size = buf->Vertices.size()+1;

	for (size_t i = 0, l = vertices.size(); i != l; ++i) {		
		auto& v = vertices[i];
		buf->Vertices.push_back(v);
		GLdouble tmp[3] = {(GLdouble)v.Pos.X,(GLdouble)v.Pos.Y, (GLdouble)v.Pos.Z};
		gluTessVertex(tess, tmp, (GLvoid *)(size+i));
	}

	gluTessEndContour(tess);
	gluTessEndPolygon(tess);

	gluDeleteTess(tess);
}


std::map<SGMaterial*, irr::scene::SMeshBufferLightMap*> ConvertPolyhedronToIrrMesh(
	std::shared_ptr<carve::mesh::MeshSet<3>> const& poly, CarveVertexTexCoords& fvTex,CarveFaceMaterials& fTexNum)
{
	std::map<SGMaterial*, irr::scene::SMeshBufferLightMap*> meshMap;

	if (!poly) return meshMap;

	std::vector<irr::video::S3DVertex2TCoords> vertices;

	for (carve::mesh::MeshSet<3>::face_iter i = poly->faceBegin(); i != poly->faceEnd(); ++i) {

		carve::mesh::MeshSet<3>::face_t *f = *i;
		auto normal = ToIrrVect(f->plane.N.v);

		vertices.clear();

		bool textured = true;
		for (carve::mesh::MeshSet<3>::face_t::edge_iter_t e = f->begin(); e != f->end(); ++e) {

			irr::core::vector2df uv;

			if (fvTex.hasAttribute(f, e.idx())) {
				auto attr = fvTex.getAttribute(f, e.idx());
				uv.set(attr.u, attr.v);
			} else {
				textured = false;
			}

			vertices.emplace_back(ToIrrVect(e->vert->v.v),normal,irr::video::SColor(255,255,255,255),uv);
		}

		_ASSERT(textured);
		
		auto material = fTexNum.getAttribute(f, nullptr);
		_ASSERT(material);
		
		//just skip over invisible one
		if (material == SGMaterial::invisibleMaterial.get() ) 
			continue;

		auto buf = meshMap[material];
		if (!buf){
			buf = new irr::scene::SMeshBufferLightMap;

			meshMap[material] = buf;
			buf->recalculateBoundingBox();
		}
			
		int size = buf->Vertices.size();

		switch(vertices.size()){
		case 3:
			buf->Vertices.push_back(vertices[0]);
			buf->Vertices.push_back(vertices[1]);
			buf->Vertices.push_back(vertices[2]);

			buf->Indices.push_back(size);
			buf->Indices.push_back(size+1);
			buf->Indices.push_back(size+2);			break;

		case 4:
			buf->Vertices.push_back(vertices[0]);
			buf->Vertices.push_back(vertices[1]);
			buf->Vertices.push_back(vertices[2]);

			buf->Indices.push_back(size);
			buf->Indices.push_back(size+1);
			buf->Indices.push_back(size+2);
			buf->Vertices.push_back(vertices[0]);
			buf->Vertices.push_back(vertices[2]);
			buf->Vertices.push_back(vertices[3]);

			buf->Indices.push_back(size+3);
			buf->Indices.push_back(size+4);
			buf->Indices.push_back(size+5);			break;

		default:
			Teselate(vertices, normal, buf);
		}

		for(auto end = buf->Vertices.size();size<end;size++)
			buf->BoundingBox.addInternalPoint(buf->Vertices[size].Pos);

	}

	return meshMap;
}

#if 0
irr::scene::SMesh* ConvertPolyhedronToIrrMesh( std::shared_ptr<carve::mesh::MeshSet<3>> poly, irr::core::matrix4 matrix, int group )
{
	if (!poly) return NULL;

	//TODO: change this so that we do not save duplicated vertices (if possible)
	//it will make life easier for the mesh simplification that will be used later

	irr::scene::SMesh* mesh=new irr::scene::SMesh;
	//irr::scene::SMeshBuffer* buf = new irr::scene::SMeshBuffer;
	irr::scene::SMeshBufferLightMap* buf = new irr::scene::SMeshBufferLightMap;

	irr::core::aabbox3df bbox;
	bbox.reset(irr::core::vector3df());

	std::vector<carve::mesh::Vertex<3> *> vertices;

	for (size_t i = 0, l = poly->meshes.front()->faces.size(); i != l; ++i) {

		auto f = poly->meshes.front()->faces[i];
		f->getVertices(vertices);

		//if (group == -1 || f.manifold_id == group) 
		{

			int size = buf->Vertices.size();

			auto normal = ToIrrVect(f->plane.N.v);

			switch(vertices.size()){
			case 3:
				buf->Vertices.push_back(irr::video::S3DVertex2TCoords(ToIrrVect(vertices[0]->v.v),normal,irr::video::SColor(255,255,255,255),irr::core::vector2df()));
				buf->Vertices.push_back(irr::video::S3DVertex2TCoords(ToIrrVect(vertices[1]->v.v),normal,irr::video::SColor(255,255,255,255),irr::core::vector2df()));
				buf->Vertices.push_back(irr::video::S3DVertex2TCoords(ToIrrVect(vertices[2]->v.v),normal,irr::video::SColor(255,255,255,255),irr::core::vector2df()));

				buf->Indices.push_back(size);
				buf->Indices.push_back(size+1);
				buf->Indices.push_back(size+2);				break;

			case 4:
				buf->Vertices.push_back(irr::video::S3DVertex2TCoords(ToIrrVect(vertices[0]->v.v),normal,irr::video::SColor(255,255,255,255),irr::core::vector2df()));
				buf->Vertices.push_back(irr::video::S3DVertex2TCoords(ToIrrVect(vertices[1]->v.v),normal,irr::video::SColor(255,255,255,255),irr::core::vector2df()));
				buf->Vertices.push_back(irr::video::S3DVertex2TCoords(ToIrrVect(vertices[2]->v.v),normal,irr::video::SColor(255,255,255,255),irr::core::vector2df()));

				buf->Indices.push_back(size);
				buf->Indices.push_back(size+1);
				buf->Indices.push_back(size+2);
				buf->Vertices.push_back(irr::video::S3DVertex2TCoords(ToIrrVect(vertices[0]->v.v),normal,irr::video::SColor(255,255,255,255),irr::core::vector2df()));
				buf->Vertices.push_back(irr::video::S3DVertex2TCoords(ToIrrVect(vertices[2]->v.v),normal,irr::video::SColor(255,255,255,255),irr::core::vector2df()));
				buf->Vertices.push_back(irr::video::S3DVertex2TCoords(ToIrrVect(vertices[3]->v.v),normal,irr::video::SColor(255,255,255,255),irr::core::vector2df()));

				buf->Indices.push_back(size+3);
				buf->Indices.push_back(size+4);
				buf->Indices.push_back(size+5);				break;

			default:
				//Teselate(&f, buf);
				Teselate(vertices, normal, buf);
			}

			for(int x=buf->Vertices.size()-1;x>=size;x--){
				matrix.transformVect(buf->Vertices[x].Pos);
				matrix.rotateVect(buf->Vertices[x].Normal);
				bbox.addInternalPoint(buf->Vertices[x].Pos);
			}
		}
	}

	

	buf->setBoundingBox(bbox);
	buf->recalculateBoundingBox();
	mesh->addMeshBuffer(buf);
	buf->drop();
	mesh->recalculateBoundingBox();


	return mesh;
}

#endif

}}
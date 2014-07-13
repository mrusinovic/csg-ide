#include "stdafx.h"
#include "SGTeselator.h"

#include "carve/carve.hpp"
#include "carve/poly.hpp"
#include "carve/matrix.hpp"

#include <gl/GLU.h>
#include "CSGProcessor.h"

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


static void WINAPI _tessCombine(GLdouble coords[3], void* vertex_data[4], GLfloat weight[4], void** outData, irr::scene::SMeshBuffer* buf )
{
	irr::core::vector3df n;
	for(int x=0;x<4 && vertex_data[x];x++){
		short index = (short)(vertex_data[x])-1;
		n += buf->Vertices[index].Normal;
	}

	buf->Vertices.push_back(irr::video::S3DVertex(ToIrrVect(coords),n.normalize(),irr::video::SColor(255,255,255,255),irr::core::vector2df()));
	*outData = (void*)buf->Vertices.size();
}

static void WINAPI _tessVertex(void *vertex_data, irr::scene::SMeshBuffer* buf) 
{
	buf->Indices.push_back(((short)vertex_data)-1);
}

void Teselate(carve::poly::Face<3> *f, irr::scene::SMeshBuffer* buf)
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

	int size = buf->Vertices.size();

	for (size_t i = 0, l = f->nVertices(); i != l; ++i) {		
		buf->Vertices.push_back(irr::video::S3DVertex(ToIrrVect(f->vertex(i)->v.v),ToIrrVect(f->plane_eqn.N.v),irr::video::SColor(255,255,255,255),irr::core::vector2df()));
		gluTessVertex(tess, (GLdouble *)f->vertex(i)->v.v, (GLvoid *)(size+i+1));
	}

	gluTessEndContour(tess);
	gluTessEndPolygon(tess);

	gluDeleteTess(tess);
}


irr::scene::IMesh* ConvertPolyhedronToIrrMesh( carve::poly::Polyhedron* poly, irr::core::matrix4 matrix, int group )
{
	if (!poly) return NULL;

	//TODO: change this so that we do not save duplicated vertices (if possible)
	//it will make life easier for the mesh simplification that will be used later

	irr::scene::SMesh* mesh=new irr::scene::SMesh;
	irr::scene::SMeshBuffer* buf = new irr::scene::SMeshBuffer;

	irr::core::aabbox3df bbox;
	bbox.reset(irr::core::vector3df());

	for (size_t i = 0, l = poly->faces.size(); i != l; ++i) {
		carve::poly::Face<3> &f = poly->faces[i];
		if (group == -1 || f.manifold_id == group) {

			int size = buf->Vertices.size();

			switch(f.nVertices()){
			case 3:
				buf->Vertices.push_back(irr::video::S3DVertex(ToIrrVect(f.vertex(0)->v.v),ToIrrVect(f.plane_eqn.N.v),irr::video::SColor(255,255,255,255),irr::core::vector2df()));
				buf->Vertices.push_back(irr::video::S3DVertex(ToIrrVect(f.vertex(1)->v.v),ToIrrVect(f.plane_eqn.N.v),irr::video::SColor(255,255,255,255),irr::core::vector2df()));
				buf->Vertices.push_back(irr::video::S3DVertex(ToIrrVect(f.vertex(2)->v.v),ToIrrVect(f.plane_eqn.N.v),irr::video::SColor(255,255,255,255),irr::core::vector2df()));

				buf->Indices.push_back(size);
				buf->Indices.push_back(size+1);
				buf->Indices.push_back(size+2);				break;

			case 4:
				buf->Vertices.push_back(irr::video::S3DVertex(ToIrrVect(f.vertex(0)->v.v),ToIrrVect(f.plane_eqn.N.v),irr::video::SColor(255,255,255,255),irr::core::vector2df()));
				buf->Vertices.push_back(irr::video::S3DVertex(ToIrrVect(f.vertex(1)->v.v),ToIrrVect(f.plane_eqn.N.v),irr::video::SColor(255,255,255,255),irr::core::vector2df()));
				buf->Vertices.push_back(irr::video::S3DVertex(ToIrrVect(f.vertex(2)->v.v),ToIrrVect(f.plane_eqn.N.v),irr::video::SColor(255,255,255,255),irr::core::vector2df()));

				buf->Indices.push_back(size);
				buf->Indices.push_back(size+1);
				buf->Indices.push_back(size+2);
				buf->Vertices.push_back(irr::video::S3DVertex(ToIrrVect(f.vertex(0)->v.v),ToIrrVect(f.plane_eqn.N.v),irr::video::SColor(255,255,255,255),irr::core::vector2df()));
				buf->Vertices.push_back(irr::video::S3DVertex(ToIrrVect(f.vertex(2)->v.v),ToIrrVect(f.plane_eqn.N.v),irr::video::SColor(255,255,255,255),irr::core::vector2df()));
				buf->Vertices.push_back(irr::video::S3DVertex(ToIrrVect(f.vertex(3)->v.v),ToIrrVect(f.plane_eqn.N.v),irr::video::SColor(255,255,255,255),irr::core::vector2df()));

				buf->Indices.push_back(size+3);
				buf->Indices.push_back(size+4);
				buf->Indices.push_back(size+5);				break;

			default:
				Teselate(&f, buf);
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
}
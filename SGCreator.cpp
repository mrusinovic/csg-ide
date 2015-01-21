#include "stdafx.h"
#include "SGCreator.h"
#include "carve/input.hpp"
#include <iostream>
#include "SGException.h"

namespace CSGProcessor
{
namespace SGCreator
{

/*
	creates this
		/\
	   /  \
	   \  /
	    \/
	if needed
*/
carve::mesh::MeshSet<3> *makeSomething(int dtheta, int dphi, const carve::math::Matrix &transform)
{
	carve::input::PolyhedronData data;

	data.addVertex(transform * carve::geom::VECTOR(0,0.5,0));// 0
	data.addVertex(transform * carve::geom::VECTOR(0.5,0,0));// 1
	data.addVertex(transform * carve::geom::VECTOR(0,0,-0.5));//2
	data.addVertex(transform * carve::geom::VECTOR(-0.5,0,0));//3
	data.addVertex(transform * carve::geom::VECTOR(0,-0.5,0));//6  4
	data.addVertex(transform * carve::geom::VECTOR(0,0,0.5));// 13 5

	data.addFace(0,1,2);
	data.addFace(3,0,2);
	data.addFace(4,3,2);
	data.addFace(1,4,2);
	data.addFace(0,5,1);
	data.addFace(3,5,0);
	data.addFace(4,5,3);
	data.addFace(1,5,4);

	return data.createMesh(carve::input::opts());
}
carve::mesh::MeshSet<3> *makeSphere(
	CarveVertexTexCoords &fv_tex,CarveFaceMaterials &f_tex_num,	SGMaterial* tex,
	float texX, float texY,
	int subdivisions, const carve::math::Matrix &transform)
{

	if (!tex) SG_THROW("Texture not set!");

	carve::math::Matrix trans = transform * carve::math::Matrix::SCALE(0.5,0.5,0.5);

	int i,j;
	double t1,t2,t3;
	int n=subdivisions;
	float r = 1;
// 	n = 6;
	std::vector<size_t> patch(4);
	std::vector<carve::mesh::MeshSet<3>::vertex_t> v;
	std::vector<carve::mesh::MeshSet<3>::face_t *> faces;

	v.reserve(n*n*4);
	carve::geom3d::Vector vts[4];

	carve::geom3d::Vector e,p;
	SGTextureAttrCarve atr[4];


	for (j=0;j<n/2;j++) {

		t1 = -M_PI/2.0 + j * M_PI / (n/2);
		t2 = -M_PI/2.0 + (j + 1) * M_PI / (n/2);

		int index = 0;

		for (i=0;i<=n;i++) {


			t3 = i * 2*M_PI / n;

			e.x = cos(t1) * cos(t3);
			e.y = sin(t1);
			e.z = cos(t1) * sin(t3);
			p.x = r * e.x;
			p.y = r * e.y;
			p.z = r * e.z;

			vts[index] = p;
			atr[index] = SGTextureAttrCarve(-i/(double)n * texX, -2*j/(double)n * texY);

			index++;

			e.x = cos(t2) * cos(t3);
			e.y = sin(t2);
			e.z = cos(t2) * sin(t3);
			p.x = r * e.x;
			p.y = r * e.y;
			p.z = r * e.z;

			vts[index] = p;
			atr[index] = SGTextureAttrCarve(-i/(double)n *texX , -2*(j+1)/(double)n * texY);
			index++;

			if (index==4){
				index=2;
				struct FindMe{
					static size_t find(std::vector<carve::mesh::MeshSet<3>::vertex_t>& vs, 
						const carve::mesh::MeshSet<3>::vertex_t& v){
						for(auto i = vs.begin();i!=vs.end();i++){
							if (irr::core::equals(v.v.x,i->v.x,0.005) &&
								irr::core::equals(v.v.y,i->v.y,0.005) &&
								irr::core::equals(v.v.z,i->v.z,0.005)){
									return i-vs.begin();
							}
						}
						vs.push_back(v);
						return vs.size()-1;
					}
				};

				size_t iss[]={
					FindMe::find(v, trans * vts[0]),
					FindMe::find(v, trans * vts[1]),
					FindMe::find(v, trans * vts[3]),
					FindMe::find(v, trans * vts[2])
				};

				patch.clear();

				for(int x=0;x<4;x++){
					if (std::find(patch.begin(),patch.end(), iss[x])==patch.end())
						patch.push_back(iss[x]);
				}

				if (patch.size()==3){

						faces.push_back(new carve::mesh::MeshSet<3>::face_t(&v[patch[0]],&v[patch[1]],&v[patch[2]]));
						fv_tex.setAttribute(faces.back(), 0, atr[0]);
						fv_tex.setAttribute(faces.back(), 1, atr[1]);
						fv_tex.setAttribute(faces.back(), 2, atr[3]);

						f_tex_num.setAttribute(faces.back(), tex);

// 						std::stringstream str;
// 						str << "i=" << i << " " << "j=" << j << " total: " << v.size() << "(" << patch[0] << "," << patch[1] << "," << patch[2] << ")" << std::endl;
// 						OutputDebugStringA(str.str().c_str());

				}else{

// 					std::stringstream str;
// 					str << "i=" << i << " " << "j=" << j << " total: " << v.size() << "(" << iss[0] << "," << iss[1] << "," << iss[2] << "," << iss[3] << ")" << std::endl;
// 					OutputDebugStringA(str.str().c_str());

					faces.push_back(new carve::mesh::MeshSet<3>::face_t(&v[iss[0]],&v[iss[1]],&v[iss[2]],&v[iss[3]]));

					fv_tex.setAttribute(faces.back(), 0, atr[0]);
					fv_tex.setAttribute(faces.back(), 1, atr[1]);
					fv_tex.setAttribute(faces.back(), 2, atr[3]);
					fv_tex.setAttribute(faces.back(), 3, atr[2]);

					f_tex_num.setAttribute(faces.back(), tex);
				
				}
				vts[0] = vts[2];
				vts[1] = vts[3];
				atr[0] = atr[2];
				atr[1] = atr[3];
			}
		}
	}

	carve::mesh::MeshSet<3> *poly = new carve::mesh::MeshSet<3>(faces);
	return poly;
}

#if 0
//ovo je neki drugi algoritam koji proizvodi puno bolje balune ali ima problema sa teksturiranjem jer 
//patch prelazi preko xz= 0 i proizvodi iste koordinate za livu(- stranu) i desnu(+stranu)
carve::mesh::MeshSet<3> *makeSphere(
	CarveVertexTexCoords &fv_tex,CarveFaceMaterials &f_tex_num,	CarveMaterial* tex,
	int dtheta, int dphi, const carve::math::Matrix &transform)
{

	carve::math::Matrix trans = transform * carve::math::Matrix::SCALE(0.5,0.5,0.5);

	//carve::input::PolyhedronData data;
	std::vector<carve::mesh::MeshSet<3>::vertex_t> v;
	std::vector<carve::mesh::MeshSet<3>::face_t *> faces;

	v.reserve(180/dtheta*360/dphi*5);

	const double DTOR = M_PI / 180.0;

	carve::geom3d::Vector p[4];

	for (int theta=-90;theta<=90-dtheta;theta+=dtheta) {

		for (int phi=0;phi<=360-dphi;phi+=dphi) {

			if (theta > 20) {

				if (phi<180 && phi > 90){
					int wewefwe=9;
				}
			}

			if (theta>-15 && theta < 0) {

				if (phi<90){
					int fwefwe=0;
				}

				if (phi<180 && phi > 90){
					int dwefwefwe=0;
				}
			}

			int n = 0;
			if (theta > -90 && theta < 90) {
				p[n].x = cos(theta*DTOR) * cos((phi+dphi)*DTOR);
				p[n].y = cos(theta*DTOR) * sin((phi+dphi)*DTOR);
				p[n].z = sin(theta*DTOR);
				n++;
			}
			p[n].x = cos((theta+dtheta)*DTOR) * cos((phi+dphi)*DTOR);
			p[n].y = cos((theta+dtheta)*DTOR) * sin((phi+dphi)*DTOR);
			p[n].z = sin((theta+dtheta)*DTOR);
			n++;
			p[n].x = cos((theta+dtheta)*DTOR) * cos(phi*DTOR);
			p[n].y = cos((theta+dtheta)*DTOR) * sin(phi*DTOR);
			p[n].z = sin((theta+dtheta)*DTOR);
			n++;
			p[n].x = cos(theta*DTOR) * cos(phi*DTOR);
			p[n].y = cos(theta*DTOR) * sin(phi*DTOR);
			p[n].z = sin(theta*DTOR);
			n++;

			int i[4];

			for(int x=0;x<n;x++){

				p[x] = trans * p[x];

				int index = 0;
				bool found=false;

				//TODO: make me better! Could this be done with some clever thinking?
				for(auto& pt : v){
					if (irr::core::equals(pt.v.x,p[x].x,0.001) &&
						irr::core::equals(pt.v.y,p[x].y,0.001) &&
						irr::core::equals(pt.v.z,p[x].z,0.001)){
							i[x]=index;
							found=true;
							break;
					}
					index++;
				}

				if (!found){
					i[x] = v.size();//data.getVertexCount();
					v.push_back(p[x]);
				}
			}


			//upper algorithm creates rectangles for the bottom cap
			//those rectangles have 2 points at the same position
			//that kills Carve
			if (n==4){

				int ii[4];
				int index = 1;
				ii[0]=i[0];
				for(int x=1;x<n;x++){
					if (i[x-1]!=i[x]){
						ii[index++]=i[x];
					}
				}
				n=index;
				memcpy(i,ii,sizeof(int)*4);
			}

			if (n==4) {
				faces.push_back(new carve::mesh::MeshSet<3>::face_t(&v[i[0]],&v[i[1]],&v[i[2]],&v[i[3]]));

				//std::cerr << data.getVertex(i[0]) << ", " << data.getVertex(i[1]) << ", " << data.getVertex(i[2]) << ", " << data.getVertex(i[3]) << std::endl;
				//std::cerr << i[0] << ", " << i[1] << ", " << i[2] << ", " << i[3] << std::endl;
			}
			else {
				//data.addFace(i[0],i[1],i[2]);
				faces.push_back(new carve::mesh::MeshSet<3>::face_t(&v[i[0]],&v[i[1]],&v[i[2]]));

				//std::cerr << data.getVertex(i[0]) << ", " << data.getVertex(i[1]) << ", " << data.getVertex(i[2]) << std::endl;
				//std::cerr << i[0] << ", " << i[1] << ", " << i[2] << std::endl;
			}

			for(int x=0;x<n;x++){
				//double tu = asin(faces.back()->plane.N.x)/PI+0.5f;
				//double tv = asin(faces.back()->plane.N.y)/PI+0.5f;
				auto vert = v[i[x]].v;//.normalized();
				//double tu = asin(vert.x)/PI+0.5;
				//double tv = asin(vert.y)/PI+0.5;

				auto len = sqrt(vert.x * vert.x + vert.y * vert.y + vert.z * vert.z);
				double tu = acos(vert.y / len) / M_PI;
				
				double tv = (atan2(vert.z, vert.x) / M_PI + 1.0f) * 0.5f;

// 				double tu = atan2(vert.x, vert.z ); 
// 				double tv = atan2(vert.y, sqrt( vert.x*vert.x+vert.z*vert.z ) );

				fv_tex.setAttribute(faces.back(), x, SGTextureAttrCarve(tu,tv));
			}

			f_tex_num.setAttribute(faces.back(), tex);
		}
	}
	carve::mesh::MeshSet<3> *poly = new carve::mesh::MeshSet<3>(faces);
	return poly;
}

#endif

carve::mesh::MeshSet<3> *makeCube(
	CarveVertexTexCoords &fv_tex,
	CarveFaceMaterials &f_tex_num,
	SGMaterial* tex,
	float texX, float texY,
	const carve::math::Matrix &transform /*= carve::math::Matrix::IDENT()*/)
{
	if (!tex) SG_THROW("Texture not set!");

	std::vector<carve::mesh::MeshSet<3>::vertex_t> v;
	v.push_back(carve::mesh::MeshSet<3>::vertex_t(transform * carve::geom::VECTOR(+0.5, +0.5, +0.5)));
	v.push_back(carve::mesh::MeshSet<3>::vertex_t(transform * carve::geom::VECTOR(-0.5, +0.5, +0.5)));
	v.push_back(carve::mesh::MeshSet<3>::vertex_t(transform * carve::geom::VECTOR(-0.5, -0.5, +0.5)));
	v.push_back(carve::mesh::MeshSet<3>::vertex_t(transform * carve::geom::VECTOR(+0.5, -0.5, +0.5)));
	v.push_back(carve::mesh::MeshSet<3>::vertex_t(transform * carve::geom::VECTOR(+0.5, +0.5, -0.5)));
	v.push_back(carve::mesh::MeshSet<3>::vertex_t(transform * carve::geom::VECTOR(-0.5, +0.5, -0.5)));
	v.push_back(carve::mesh::MeshSet<3>::vertex_t(transform * carve::geom::VECTOR(-0.5, -0.5, -0.5)));
	v.push_back(carve::mesh::MeshSet<3>::vertex_t(transform * carve::geom::VECTOR(+0.5, -0.5, -0.5)));

	std::vector<carve::mesh::MeshSet<3>::face_t *> faces;

	faces.reserve(6);
	faces.push_back(new carve::mesh::MeshSet<3>::face_t(&v[0], &v[1], &v[2], &v[3]));
	faces.push_back(new carve::mesh::MeshSet<3>::face_t(&v[7], &v[6], &v[5], &v[4]));
	faces.push_back(new carve::mesh::MeshSet<3>::face_t(&v[0], &v[4], &v[5], &v[1]));
	faces.push_back(new carve::mesh::MeshSet<3>::face_t(&v[1], &v[5], &v[6], &v[2]));
	faces.push_back(new carve::mesh::MeshSet<3>::face_t(&v[2], &v[6], &v[7], &v[3]));
	faces.push_back(new carve::mesh::MeshSet<3>::face_t(&v[3], &v[7], &v[4], &v[0]));

	for (size_t i = 0; i < 6; ++i) {
		
		//0,1 = +Z-
		//2,3 = +X-
		//2,5 = +Y-

		fv_tex.setAttribute(faces[i], 0, SGTextureAttrCarve(0.0f * texX, 1.0f * texY));
		fv_tex.setAttribute(faces[i], 1, SGTextureAttrCarve(1.0f * texX, 1.0f * texY));
		fv_tex.setAttribute(faces[i], 2, SGTextureAttrCarve(1.0f * texX, 0.0f * texY));
		fv_tex.setAttribute(faces[i], 3, SGTextureAttrCarve(0.0f * texX, 0.0f * texY));

		f_tex_num.setAttribute(faces[i], tex);
	}

	carve::mesh::MeshSet<3> *poly = new carve::mesh::MeshSet<3>(faces);
	return poly;
}

carve::mesh::MeshSet<3> *makeTorus(	
	CarveVertexTexCoords &fv_tex,CarveFaceMaterials &f_tex_num,SGMaterial* tex,
	float texX, float texY,
	int slices,int rings,double rad1, double rad2, const carve::math::Matrix &transform) 
{
	if (!tex) SG_THROW("Texture not set!");

	carve::input::PolyhedronData data;
	data.reserveVertices(slices * rings);

	for (int i = 0; i < slices; i++) {
		double a1 = i * M_PI * 2.0 / slices;
		double dy = cos(a1);
		double dx = sin(a1);
		for (int j = 0; j < rings; j++) {
			double a2 = j * M_PI * 2.0 / rings;
			double x = dx * (rad1 + cos(a2) * rad2);
			double y = dy * (rad1 + cos(a2) * rad2);
			double z = sin(a2) * rad2;
			data.addVertex(transform * carve::geom::VECTOR(x, y, z));
		}
	}

#define V(i, j) ((i) * rings + (j))

	data.reserveFaces(slices * rings, 4);
	for (int i = 0; i < slices; i++) {
		int i2 = (i + 1) % slices;
		for (int j = 0; j < rings; j++) {
			int j2 = (j + 1) % rings;
			data.addFace(V(i, j), V(i, j2), V(i2, j2), V(i2, j));
		}
	}
#undef V

	return data.createMesh(carve::input::opts());
}

carve::mesh::MeshSet<3> *makeCylinder(
	CarveVertexTexCoords &fv_tex,CarveFaceMaterials &f_tex_num,SGMaterial* tex,
	float texX, float texY,
	int slices, double rad,  double height, const carve::math::Matrix &transform) 
{
	if (!tex) SG_THROW("Texture not set!");

	std::vector<carve::mesh::MeshSet<3>::vertex_t> v;
	std::vector<carve::mesh::MeshSet<3>::face_t *> faces;

	v.reserve(slices * 3);
	
	v.push_back(transform * carve::geom::VECTOR(0, 0, +height/2));
	v.push_back(transform * carve::geom::VECTOR(0, 0, -height/2));

	for (int i = 0; i <= slices+1; i++) {
		double a1 = i * M_PI * 2.0 / slices;
		double y = cos(a1) * rad;
		double x = sin(a1) * rad;
		v.push_back(transform * carve::geom::VECTOR(x, y, +height/2));
		v.push_back(transform * carve::geom::VECTOR(x, y, -height/2));
	}

	
	faces.reserve((slices+1) * 3);

	for (int i = 0; i < slices; i++) {//gori

		carve::mesh::MeshSet<3>::vertex_t* vts[]={&v[0], &v[2 + ((i+1) % slices) * 2], &v[2 + i * 2]};

		faces.push_back(new carve::mesh::MeshSet<3>::face_t(vts[0],vts[1],vts[2]));

		fv_tex.setAttribute(faces.back(), 0, SGTextureAttrCarve(0.5f * texX, 0.5f  * texY));
		fv_tex.setAttribute(faces.back(), 1, SGTextureAttrCarve( (vts[1]->v.y/M_PI+0.5) * texX, (vts[1]->v.x/M_PI+0.5) * texY)); //ako se skine PI onda se dobije puna textura
		fv_tex.setAttribute(faces.back(), 2, SGTextureAttrCarve( (vts[2]->v.y/M_PI+0.5) * texX, (vts[2]->v.x/M_PI+0.5) * texY )); //izgleda bolje za neke tex.
		f_tex_num.setAttribute(faces.back(), tex);
	}

	for (int i = 0; i < slices; i++) {//tijelo

		carve::mesh::MeshSet<3>::vertex_t* vts[]={&v[2 + i * 2], &v[2 + ((i+1) % slices) * 2], &v[3 + ((i+1) % slices) * 2], &v[3 + i * 2]};

		faces.push_back(new carve::mesh::MeshSet<3>::face_t(vts[0],vts[1],vts[2], vts[3]));

		//double tx = i/(slices/M_PI);
		//double tx1 = (i+1)/(slices/M_PI);

		double tx = i/((double)slices); //ako se slices podjeli s nekim brojem onda se dobije tex. vise puta okolo
		double tx1 = (i+1)/((double)slices);

		fv_tex.setAttribute(faces.back(), 0, SGTextureAttrCarve(tx * texX,  (vts[0]->v.z+0.5) * texY));
		fv_tex.setAttribute(faces.back(), 1, SGTextureAttrCarve(tx1 * texX, (vts[1]->v.z+0.5) * texY));
		fv_tex.setAttribute(faces.back(), 2, SGTextureAttrCarve(tx1 * texX, (vts[2]->v.z+0.5) * texY));
		fv_tex.setAttribute(faces.back(), 3, SGTextureAttrCarve(tx * texX,  (vts[3]->v.z+0.5) * texY));


		f_tex_num.setAttribute(faces.back(), tex);

	}

	for (int i = 0; i < slices; i++) {//doli
		carve::mesh::MeshSet<3>::vertex_t* vts[]={&v[1],&v[3 + i * 2], &v[3 + ((i+1) % slices) * 2]};
		faces.push_back(new carve::mesh::MeshSet<3>::face_t(vts[0],vts[1],vts[2]));

		fv_tex.setAttribute(faces.back(), 0, SGTextureAttrCarve(+0.5 * texX, +0.5 * texY));
		fv_tex.setAttribute(faces.back(), 1, SGTextureAttrCarve( (vts[1]->v.y/M_PI+0.5) * texX, (vts[1]->v.x/M_PI+0.5) * texY));
		fv_tex.setAttribute(faces.back(), 2, SGTextureAttrCarve( (vts[2]->v.y/M_PI+0.5) * texX, (vts[2]->v.x/M_PI+0.5) * texY));
		f_tex_num.setAttribute(faces.back(), tex);

	}

	carve::mesh::MeshSet<3> *poly = new carve::mesh::MeshSet<3>(faces);
	return poly;
}

carve::mesh::MeshSet<3> *makeCone(
	CarveVertexTexCoords &fv_tex,CarveFaceMaterials &f_tex_num,SGMaterial* tex,
	float texX, float texY,
	int slices, double rad,  double height, const carve::math::Matrix &transform) 
{
	if (!tex) SG_THROW("Texture not set!");

	std::vector<carve::mesh::MeshSet<3>::vertex_t> v;
	std::vector<carve::mesh::MeshSet<3>::face_t *> faces;

	v.reserve(slices + 3);

	v.push_back(transform * carve::geom::VECTOR(0, +height/2,0));
	v.push_back(transform * carve::geom::VECTOR(0, -height/2,0));

	for (int i = 0; i < slices; i++) {
		double a1 = i * M_PI * 2.0 / slices;
		double y = cos(a1) * rad;
		double x = sin(a1) * rad;
		v.push_back(transform * carve::geom::VECTOR(x, -height/2,y));
	}

	faces.reserve(slices * 2);
	for (int i = 0; i < slices; i++) {

		carve::mesh::MeshSet<3>::vertex_t* vts[]={&v[2 + i],&v[2 + ((i+1) % slices)], &v[0]};
		faces.push_back(new carve::mesh::MeshSet<3>::face_t(vts[0],vts[1],vts[2]));

		fv_tex.setAttribute(faces.back(), 0, SGTextureAttrCarve(vts[0]->v.z * texX, vts[0]->v.x * texY));
		fv_tex.setAttribute(faces.back(), 1, SGTextureAttrCarve(vts[1]->v.z * texX, vts[1]->v.x * texY));
		fv_tex.setAttribute(faces.back(), 2, SGTextureAttrCarve(0, 0));

		f_tex_num.setAttribute(faces.back(), tex);

	}
	for (int i = 0; i < slices; i++) {

		carve::mesh::MeshSet<3>::vertex_t* vts[]={&v[2 + ((i+1) % slices)],&v[2 + i], &v[1]};
		faces.push_back(new carve::mesh::MeshSet<3>::face_t(vts[0],vts[1],vts[2]));

		fv_tex.setAttribute(faces.back(), 0, SGTextureAttrCarve( (vts[0]->v.z/M_PI+0.5) * texX, (vts[0]->v.x/M_PI+0.5) * texY));
		fv_tex.setAttribute(faces.back(), 1, SGTextureAttrCarve( (vts[1]->v.z/M_PI+0.5) * texX, (vts[1]->v.x/M_PI+0.5) * texY));
		fv_tex.setAttribute(faces.back(), 2, SGTextureAttrCarve(+0.5 * texX, +0.5 * texY));

		f_tex_num.setAttribute(faces.back(), tex);
	}

	carve::mesh::MeshSet<3> *poly = new carve::mesh::MeshSet<3>(faces);
	return poly;

}

}
}
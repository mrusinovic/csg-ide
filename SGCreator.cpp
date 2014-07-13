#include "stdafx.h"
#include "SGCreator.h"
#include "carve/input.hpp"
#include <iostream>


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
carve::poly::Polyhedron *makeSomething(int dtheta, int dphi, const carve::math::Matrix &transform)
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

	return data.create();
}

carve::poly::Polyhedron *makeSphere(int dtheta, int dphi, const carve::math::Matrix &transform)
{

	carve::math::Matrix trans = transform * carve::math::Matrix::SCALE(0.5,0.5,0.5);

	carve::input::PolyhedronData data;

	const double PI	= 3.14159265359;
	const double DTOR = PI / 180.0;

	carve::geom3d::Vector p[4];

	for (int theta=-90;theta<=90-dtheta;theta+=dtheta) {
		for (int phi=0;phi<=360-dphi;phi+=dphi) {
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
				for(auto& pt : data.points){
					if (irr::core::equals(pt.x,p[x].x) &&
						irr::core::equals(pt.y,p[x].y) &&
						irr::core::equals(pt.z,p[x].z)){
							i[x]=index;
							found=true;
							break;
					}
					index++;
				}

				if (!found){
					i[x] = data.getVertexCount();
					data.addVertex(p[x]);
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
				data.addFace(i[0],i[1],i[2],i[3]);
				//std::cerr << data.getVertex(i[0]) << ", " << data.getVertex(i[1]) << ", " << data.getVertex(i[2]) << ", " << data.getVertex(i[3]) << std::endl;
				//std::cerr << i[0] << ", " << i[1] << ", " << i[2] << ", " << i[3] << std::endl;
			}
			else {
				data.addFace(i[0],i[1],i[2]);
				//std::cerr << data.getVertex(i[0]) << ", " << data.getVertex(i[1]) << ", " << data.getVertex(i[2]) << std::endl;
				//std::cerr << i[0] << ", " << i[1] << ", " << i[2] << std::endl;
			}
		}
	}
	return data.create();
}

carve::poly::Polyhedron *makeCube(const carve::math::Matrix &transform) {
	carve::input::PolyhedronData data;

	data.addVertex(transform * carve::geom::VECTOR(+0.5, +0.5, +0.5));
	data.addVertex(transform * carve::geom::VECTOR(-0.5, +0.5, +0.5));
	data.addVertex(transform * carve::geom::VECTOR(-0.5, -0.5, +0.5));
	data.addVertex(transform * carve::geom::VECTOR(+0.5, -0.5, +0.5));
	data.addVertex(transform * carve::geom::VECTOR(+0.5, +0.5, -0.5));
	data.addVertex(transform * carve::geom::VECTOR(-0.5, +0.5, -0.5));
	data.addVertex(transform * carve::geom::VECTOR(-0.5, -0.5, -0.5));
	data.addVertex(transform * carve::geom::VECTOR(+0.5, -0.5, -0.5));

	data.addFace(0, 1, 2, 3);
	data.addFace(7, 6, 5, 4);
	data.addFace(0, 4, 5, 1);
	data.addFace(1, 5, 6, 2);
	data.addFace(2, 6, 7, 3);
	data.addFace(3, 7, 4, 0);

	return data.create();
}

carve::poly::Polyhedron *makeTorus(int slices,int rings,double rad1, double rad2, const carve::math::Matrix &transform) {
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

	return data.create();
}

carve::poly::Polyhedron *makeCylinder(int slices, double rad,  double height, const carve::math::Matrix &transform) {
	carve::input::PolyhedronData data;
	data.reserveVertices(slices * 2 + 2);

	data.addVertex(transform * carve::geom::VECTOR(0, 0, +height/2));
	data.addVertex(transform * carve::geom::VECTOR(0, 0, -height/2));

	for (int i = 0; i < slices; i++) {
		double a1 = i * M_PI * 2.0 / slices;
		double y = cos(a1) * rad;
		double x = sin(a1) * rad;
		data.addVertex(transform * carve::geom::VECTOR(x, y, +height/2));
		data.addVertex(transform * carve::geom::VECTOR(x, y, -height/2));
	}

	data.reserveFaces(slices * 3, 4);
	for (int i = 0; i < slices; i++) {
		data.addFace(0,
			2 + ((i+1) % slices) * 2,
			2 + i * 2);
	}
	for (int i = 0; i < slices; i++) {
		data.addFace(2 + i * 2,
			2 + ((i+1) % slices) * 2,
			3 + ((i+1) % slices) * 2,
			3 + i * 2);
	}
	for (int i = 0; i < slices; i++) {
		data.addFace(1,
			3 + i * 2,
			3 + ((i+1) % slices) * 2);
	}

	return data.create();
}

carve::poly::Polyhedron *makeCone(int slices, double rad,  double height, const carve::math::Matrix &transform) {
	carve::input::PolyhedronData data;
	data.reserveVertices(slices + 2);

	data.addVertex(transform * carve::geom::VECTOR(0, +height/2,0));
	data.addVertex(transform * carve::geom::VECTOR(0, -height/2,0));

	for (int i = 0; i < slices; i++) {
		double a1 = i * M_PI * 2.0 / slices;
		double y = cos(a1) * rad;
		double x = sin(a1) * rad;
		data.addVertex(transform * carve::geom::VECTOR(x, -height/2,y));
	}
	data.reserveFaces(slices * 2, 3);
	for (int i = 0; i < slices; i++) {
		data.addFace(2 + i,
			2 + ((i+1) % slices),
			0);
	}
	for (int i = 0; i < slices; i++) {
		data.addFace(2 + ((i+1) % slices),
			2 + i,
			1);
	}

	return data.create();
}

}
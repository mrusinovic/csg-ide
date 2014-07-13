#pragma once


#include "carve/poly.hpp"
#include "carve/matrix.hpp"

namespace SGCreator
{

	carve::poly::Polyhedron *makeCube(const carve::math::Matrix &transform = carve::math::Matrix());

	carve::poly::Polyhedron *makeTorus(
		int slices=14, 
		int rings=14, 
		double rad1=0.45, 
		double rad2=0.15, 
		const carve::math::Matrix &transform = carve::math::Matrix());
	carve::poly::Polyhedron *makeCylinder(int slices=12, 
		double rad=0.5,	
		double height=1, 
		const carve::math::Matrix &transform = carve::math::Matrix());
	carve::poly::Polyhedron *makeCone(int slices=12, 
		double rad=0.5, 
		double height=1, 
		const carve::math::Matrix &transform = carve::math::Matrix());

	carve::poly::Polyhedron *makeSphere(
		int dtheta=10, 
		int dphi=10,
		const carve::math::Matrix &transform = carve::math::Matrix());

};

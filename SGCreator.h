#pragma once


#include "carve/poly.hpp"
#include "carve/matrix.hpp"
#include "SGMaterial.h"

namespace CSGProcessor
{
namespace SGCreator
{

	carve::mesh::MeshSet<3> *makeCube(	
		CarveVertexTexCoords &fv_tex,
		CarveFaceMaterials &f_tex_num,
		SGMaterial* tex,
		float texX=1.f, float texY=1.f,
		const carve::math::Matrix &transform = carve::math::Matrix::IDENT());

	carve::mesh::MeshSet<3> *makeTorus(
		CarveVertexTexCoords &fv_tex,
		CarveFaceMaterials &f_tex_num,
		SGMaterial* tex,
		float texX=1.f, float texY=1.f,
		int slices=14, 
		int rings=14, 
		double rad1=0.45, 
		double rad2=0.15, 
		const carve::math::Matrix &transform = carve::math::Matrix::IDENT());

	carve::mesh::MeshSet<3> *makeCylinder(
		CarveVertexTexCoords &fv_tex,
		CarveFaceMaterials &f_tex_num,
		SGMaterial* tex,
		float texX=1.f, float texY=1.f,
		int slices=12, 
		double rad=0.5,	
		double height=1, 
		const carve::math::Matrix &transform = carve::math::Matrix::IDENT());

	carve::mesh::MeshSet<3> *makeCone(
		CarveVertexTexCoords &fv_tex,
		CarveFaceMaterials &f_tex_num,
		SGMaterial* tex,
		float texX=1.f, float texY=1.f,
		int slices=12, 
		double rad=0.5, 
		double height=1, 
		const carve::math::Matrix &transform = carve::math::Matrix::IDENT());

	carve::mesh::MeshSet<3> *makeSphere(
		CarveVertexTexCoords &fv_tex,
		CarveFaceMaterials &f_tex_num,
		SGMaterial* tex,
		float texX=1.f, float texY=1.f,
		int subdivisions = 15,
		//int dtheta=20, 
		//int dphi=20,
		const carve::math::Matrix &transform = carve::math::Matrix::IDENT());

}
}

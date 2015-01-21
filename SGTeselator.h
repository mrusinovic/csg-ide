#pragma once

#include "SGMaterial.h"

namespace CSGProcessor
{
namespace SGCreator
{
	//irr::scene::SMesh* 
	std::map<SGMaterial*, irr::scene::SMeshBufferLightMap*>	ConvertPolyhedronToIrrMesh(
		std::shared_ptr<carve::mesh::MeshSet<3>> const& poly, 
		CarveVertexTexCoords& fvTex,
		CarveFaceMaterials& fTexNum);
}}
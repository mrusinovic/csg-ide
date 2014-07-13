#pragma once

#include "carve/poly.hpp"

namespace SGCreator
{
	irr::scene::IMesh* ConvertPolyhedronToIrrMesh(carve::poly::Polyhedron* p, irr::core::matrix4 matrix, int group = -1);

}
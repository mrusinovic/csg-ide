#pragma once

namespace carve
{
	namespace poly
	{
		struct Polyhedron;
	}
}

namespace SGCreator
{
	irr::scene::IMesh* ConvertPolyhedronToIrrMesh(carve::poly::Polyhedron* p, irr::core::matrix4 matrix, int group = -1);

}
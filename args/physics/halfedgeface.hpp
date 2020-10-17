#pragma once
#include <core/core.hpp>

namespace args::physics
{
	struct HalfEdgeEdge;

	struct HalfEdgeFace
	{
		HalfEdgeEdge* startEdge;
		math::vec3 normal;
		math::vec3 centroid;
        std::string id;

		HalfEdgeFace(HalfEdgeEdge* newStartEdge, math::vec3 newNormal);

		/** @brief given a function that takes in a HalfEdgeEdge*, 
		* executes the function on each edge connected to 'startEdge'
		*/
		void forEachEdge(args::core::delegate< void(HalfEdgeEdge*)> functionToExecute);
		

		~HalfEdgeFace();


	};
}
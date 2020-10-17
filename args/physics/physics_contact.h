#pragma once

#include <physics/components/rigidbody.hpp>
#include <core/core.hpp>

namespace args::physics
{
	struct physics_contact
	{
		// these might change later so i wont comment them yet, dont @ me for this.

		ecs::component_handle<rigidbody> rbRef;
		ecs::component_handle<rigidbody> rbInc;

		math::vec3 worldContactRef;
		math::vec3 worldContactInc;

		math::mat4 refTransform;
		math::mat4 incTransform;


		void preCalculateEffectiveMass()
		{

		}

		void resolveContactConstraint()
		{

		}

		void resolveFrictionConstraint()
		{

		}


	};
}
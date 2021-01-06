#pragma once
#include <core/core.hpp>
#include <core/math/math.hpp>
#include "../systems/testsystem.hpp"
#include "../systems/testsystemconvexhull.hpp"
#include "../systems/testsystem2.hpp"
#include"../systems/pointcloudtestsystem2.hpp"
#include "../systems/simplecameracontroller.hpp"
#include "../systems/gui_test.hpp"
#include "../systems/testsystem2.hpp"
#include"../systems/pointcloudtestsystem2.hpp"
#include "../systems/simplecameracontroller.hpp"
#include "../systems/scenetestsystem1.hpp"
#include "../systems/lightsystem.hpp"
#include "../systems/starsystem.hpp"

#include <rendering/systems/pointcloudgeneration.hpp>

using namespace legion;

class TestModule : public Module
{
public:
    virtual void setup() override
    {
        app::WindowSystem::requestWindow(world_entity_id, math::ivec2(1920, 1080), "LEGION Engine", "Legion Icon", nullptr, nullptr, 1); // Create the request for the main window.
        reportComponentType<sah>();

        reportComponentType<planet>();
        reportSystem<StarSystem>();
        reportSystem<LightManager>();
        reportSystem<TestSystem>();
        //reportSystem<SceneTestSystem1>();
        reportSystem<SimpleCameraController>();
        reportSystem<GuiTestSystem>();
    }

    virtual priority_type priority() override
    {
        return default_priority;
    }
};


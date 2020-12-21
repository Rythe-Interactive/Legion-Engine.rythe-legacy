#pragma once
#include <core/core.hpp>
#include <core/math/math.hpp>
#include "../systems/testsystem.hpp"

#include "../systems/testsystemconvexhull.hpp"
//#include "../systems/testsystem2.hpp"
//#include"../systems/pointcloudtestsystem2.hpp"
#include "../systems/simplecameracontroller.hpp"
#include "../systems/gui_test.hpp"

#include <rendering/systems/pointcloudgeneration.hpp>
#include "../systems/shaderguikitchen.hpp"

using namespace legion;

class TestModule : public Module
{
public:
    virtual void setup() override
    {
        reportSystem<TestSystemConvexHull>();
        reportSystem<SimpleCameraController>();
     //   reportSystem<GuiTestSystem>();
        reportSystem<ShaderGuiKitchen>();
    }

    virtual priority_type priority() override
    {
        return default_priority;
    }
};


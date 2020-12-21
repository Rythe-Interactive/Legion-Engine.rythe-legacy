#pragma once
#include <core/core.hpp>
#include <rendering/rendering.hpp>
#include <rendering/util/gui.hpp>
#include <rendering/pipeline/gui/stages/imguirenderstage.hpp>

using namespace legion::rendering;

class ShaderGuiKitchen : public System<ShaderGuiKitchen>
{
    ecs::EntityQuery cameraQuery = createQuery<camera, position, rotation, scale>();


    material_handle vertexColorMaterial;
    model_handle cubeModel;
    ecs::entity_handle cubeEntity;


    char guiTextBuffer[512]{ 0 };

    math::mat4 view = math::mat4(1.0f);
    math::mat4 projection = math::mat4(1.0f);
    math::mat4 model = math::mat4(1.0f);
    void setup() override
    {

        static_cast<DefaultPipeline*>(Renderer::getMainPipeline())->attachStage<ImGuiStage>();

        app::window window = m_ecs->world.get_component_handle<app::window>().read();

        {
            application::context_guard guard(window);

            cubeModel = ModelCache::create_model("cube", "assets://models/cube.obj"_view);
            vertexColorMaterial = MaterialCache::create_material("color shader", "assets://shaders/texture.shs"_view);
        }


        cubeEntity = createEntity();

        cubeEntity.add_components<transform>(position(), rotation(), scale());
        cubeEntity.add_components<mesh_renderable>(mesh_filter(cubeModel.get_mesh()), mesh_renderer(vertexColorMaterial));


        //gui code goes here
        ImGuiStage::addGuiRender<ShaderGuiKitchen, &ShaderGuiKitchen::onGUI>(this);
        createProcess<&ShaderGuiKitchen::update>("Update");

    }
    void onGUI()
    {
        ImGuiIO& io = ImGui::GetIO();
        if (true)
        {
            ImVec4* colors = ImGui::GetStyle().Colors;
            colors[ImGuiCol_Text] = ImVec4(0.33f, 1.00f, 0.02f, 1.00f);
            colors[ImGuiCol_TextDisabled] = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
            colors[ImGuiCol_WindowBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.94f);
            colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            colors[ImGuiCol_PopupBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.94f);
            colors[ImGuiCol_Border] = ImVec4(0.35f, 0.82f, 0.52f, 0.50f);
            colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
            colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.54f);
            colors[ImGuiCol_FrameBgHovered] = ImVec4(0.78f, 0.16f, 0.16f, 0.39f);
            colors[ImGuiCol_FrameBgActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.67f);
            colors[ImGuiCol_TitleBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
            colors[ImGuiCol_TitleBgActive] = ImVec4(0.33f, 0.27f, 0.27f, 1.00f);
            colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
            colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
            colors[ImGuiCol_ScrollbarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.53f);
            colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.19f, 0.52f, 0.11f, 1.00f);
            colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.78f, 0.16f, 0.16f, 0.39f);
            colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.87f, 1.00f, 0.87f, 1.00f);
            colors[ImGuiCol_CheckMark] = ImVec4(0.16f, 0.72f, 0.31f, 1.00f);
            colors[ImGuiCol_SliderGrab] = ImVec4(0.07f, 1.00f, 0.00f, 1.00f);
            colors[ImGuiCol_SliderGrabActive] = ImVec4(0.58f, 0.97f, 0.50f, 1.00f);
            colors[ImGuiCol_Button] = ImVec4(0.27f, 0.27f, 0.27f, 0.40f);
            colors[ImGuiCol_ButtonHovered] = ImVec4(0.78f, 0.16f, 0.16f, 0.39f);
            colors[ImGuiCol_ButtonActive] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
            colors[ImGuiCol_Header] = ImVec4(0.65f, 0.67f, 0.70f, 0.31f);
            colors[ImGuiCol_HeaderHovered] = ImVec4(0.78f, 0.16f, 0.16f, 0.39f);
            colors[ImGuiCol_HeaderActive] = ImVec4(0.44f, 0.44f, 0.44f, 1.00f);
            colors[ImGuiCol_Separator] = ImVec4(1.00f, 0.00f, 0.00f, 0.50f);
            colors[ImGuiCol_SeparatorHovered] = ImVec4(1.00f, 0.00f, 0.00f, 0.78f);
            colors[ImGuiCol_SeparatorActive] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
            colors[ImGuiCol_ResizeGrip] = ImVec4(0.05f, 1.00f, 0.00f, 0.25f);
            colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.64f, 1.00f, 0.56f, 0.67f);
            colors[ImGuiCol_ResizeGripActive] = ImVec4(0.34f, 1.00f, 0.39f, 0.95f);
            colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.16f, 0.17f, 0.86f);
            colors[ImGuiCol_TabHovered] = ImVec4(0.78f, 0.16f, 0.16f, 0.39f);
            colors[ImGuiCol_TabActive] = ImVec4(0.71f, 0.00f, 0.00f, 1.00f);
            colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
            colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
            colors[ImGuiCol_PlotLines] = ImVec4(0.57f, 1.00f, 0.54f, 1.00f);
            colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.73f, 1.00f, 0.56f, 1.00f);
            colors[ImGuiCol_PlotHistogram] = ImVec4(0.56f, 1.00f, 0.50f, 1.00f);
            colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.57f, 1.00f, 0.57f, 1.00f);
            colors[ImGuiCol_TextSelectedBg] = ImVec4(0.61f, 0.61f, 0.61f, 0.35f);
            colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 0.00f, 0.00f, 0.90f);
            colors[ImGuiCol_NavHighlight] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
            colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.00f, 1.00f, 0.02f, 0.70f);
            colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.20f);
            colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.60f, 0.60f, 0.60f, 0.35f);

        }
        //uniqueNodeID++;
        //imgui::nodes::BeginNode(uniqueNodeID);
        //ImGui::Text("Node A");
        //imgui::nodes::beginp
        ////  imgui::nodes::BeginInputAttribute
        //imgui::nodes::EndNode();

    /*     imgui::nodes::BeginPin(uniqueId++, ed::PinKind::Input);
         ImGui::Text("-> In");
         ed::EndPin();
         ImGui::SameLine();
         ed::BeginPin(uniqueId++, ed::PinKind::Output);
         ImGui::Text("Out ->");
         ed::EndPin();
         ed::EndNode();*/


        setProjectionAndView(io.DisplaySize.x / io.DisplaySize.y);


        using namespace imgui;
        //  base::ShowDemoWindow();
        gizmo::SetOrthographic(false);
        gizmo::BeginFrame();
        nodes::BeginNode(0);
        nodes::EndNode();

        base::Begin("Hello World");
        // gizmo::EditTransform(value_ptr(view), value_ptr(projection), value_ptr(model), true);
        base::End();

        base::Begin("Window");
        base::Text("Hello World!");


        base::End();


        //gizmo::ViewManipulate(value_ptr(view), 1.0f, ImVec2(io.DisplaySize.x - 128, 0), ImVec2(128, 128), 0x10101010);
    }



    void update(time::span dt)
    {

        if (cubeEntity.valid())
        {
            auto [mposh, mroth, mscaleh] = cubeEntity.get_component_handles<transform>();
            math::vec3 mpos, mscale;
            math::quat mrot;
            decompose(model, mscale, mrot, mpos);
            mposh.write(mpos);
            mroth.write(mrot);
            mscaleh.write(mscale);
        }

    }

    void setProjectionAndView(float aspect)
    {
        cameraQuery.queryEntities();
        ecs::entity_handle cam_ent = cameraQuery[0];
        auto [cposh, croth, cscaleh] = cam_ent.get_component_handles<transform>();

        math::mat4 temp(1.0f);
        math::compose(temp, cscaleh.read(), croth.read(), cposh.read());
        view = inverse(temp);

        const auto cam = cam_ent.get_component_handle<camera>().read();
        const float ratio = 16.0f / 9.0f;
        projection = math::perspective(math::deg2rad(cam.fov * aspect), aspect, cam.nearz, cam.farz);
    }
};


#include "qbpch.h"
#include "QbitAppLayer.h"

#include <imgui/imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Cloth.h"
#include <Timestep.h>

// Include your Pendulum header




QbitAppLayer::QbitAppLayer()
    : Layer("QbitAppLayer"),
    m_CameraController(1600.0f / 900.0f)
{
    initialize_cloth(cloth, 10.0, 10.0, 20, 20, M_PI / 6);
}

void QbitAppLayer::OnAttach()
{
    QB_PROFILE_FUNCTION();
}

void QbitAppLayer::OnDetach()
{
    QB_PROFILE_FUNCTION();
}

static void DrawDoublePendulum(const Pendulum& pendulum1, const Pendulum& pendulum2, const State& state)
{
    glm::vec2 size = { 0.1f, 0.1f }; // Size of the quad representing a mass
    glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f }; // White color for masses
    glm::vec4 rod_color = { 0.8f, 0.8f, 0.8f, 1.0f }; // Light gray color for rods

    glm::vec2 origin = { 0.0f, 0.0f };
    glm::vec2 position1 = {
        origin.x + pendulum1.length * sin(state.theta1),
        origin.y - pendulum1.length * cos(state.theta1)
    };
    glm::vec2 position2 = {
        position1.x + pendulum2.length * sin(state.theta2),
        position1.y - pendulum2.length * cos(state.theta2)
    };

    // Draw rods
    Qbit::Renderer2D::DrawLine(glm::vec3{ origin.x, origin.y, 0.0f }, glm::vec3{ position1.x, position1.y, 0.0f }, rod_color);
    Qbit::Renderer2D::DrawLine(glm::vec3{ position1.x, position1.y, 0.0f }, glm::vec3{ position2.x, position2.y, 0.0f }, rod_color);

    // Draw masses
    Qbit::Renderer2D::DrawQuad(position1, size, color);
    Qbit::Renderer2D::DrawQuad(position2, size, color);
}

static void DrawCloth(const Cloth& cloth)
{
    glm::vec2 size = { 0.1f, 0.1f }; // Size of the quad representing a particle
    glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f }; // White color for particles

    for (const auto& particle : cloth.particles) {
        glm::vec2 position = { particle.position.x, particle.position.y };
        Qbit::Renderer2D::DrawQuad(position, size, color);
    }

    glm::vec4 spring_color = { 0.8f, 0.8f, 0.8f, 1.0f }; // Light gray color for springs
    for (const auto& spring : cloth.springs) {
        glm::vec3 p0 = { cloth.particles[spring.p1].position.x, cloth.particles[spring.p1].position.y, 0.0f };
        glm::vec3 p1 = { cloth.particles[spring.p2].position.x, cloth.particles[spring.p2].position.y, 0.0f };
        Qbit::Renderer2D::DrawLine(p0, p1, spring_color);
    }
}

void QbitAppLayer::OnUpdate(Qbit::Timestep ts)
{
    QB_PROFILE_FUNCTION();

    // Update camera
    m_CameraController.OnUpdate(ts);

    // Render
    Qbit::Renderer2D::ResetStats();
    {
        QB_PROFILE_SCOPE("RendererPrep");
        Qbit::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        Qbit::RenderCommand::Clear();
    }
    {
        QB_PROFILE_SCOPE("Renderer - Draw");
        Qbit::Renderer2D::BeginScene(m_CameraController.GetCamera());




        QP::Timestep ts = 0.0016;
        

#if 1
        runge_kutta_step(pendulum1, pendulum2, params, state, ts);

        DrawDoublePendulum(pendulum1, pendulum2, state);
#endif

#if 0
        update_cloth(cloth, ts);

        DrawCloth(cloth);
#endif

        Qbit::Renderer2D::EndScene();
    }
}

void QbitAppLayer::OnImGuiRender()
{
    QB_PROFILE_FUNCTION();

    ImGui::Begin("Settings");

    auto stats = Qbit::Renderer2D::GetStats();
    ImGui::Text("Renderer2D Stats:");
    ImGui::Text("Draw Calls: %d", stats.DrawCalls);
    ImGui::Text("Quads: %d", stats.QuadCount);
    ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
    ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

    float fps = ImGui::GetIO().Framerate;
    ImGui::Text("FPS: %.1f    %.2f ms", fps, 1000 * 1.0f / fps);

    ImGui::End();
}

void QbitAppLayer::OnEvent(Qbit::Event& e)
{
    m_CameraController.OnEvent(e);
}

#include "qbpch.h"
#include "QbitAppLayer.h"

#include <imgui/imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Timestep.h>

enum SimulationMode
{
    SIMULATION_DOUBLE_PENDULUM,
    SIMULATION_CLOTH,
    SIMULATION_ROPE
};

static SimulationMode g_CurrentSimulationMode = SIMULATION_DOUBLE_PENDULUM;

QbitAppLayer::QbitAppLayer()
    : Layer("QbitAppLayer"),
    m_CameraController(1600.0f / 900.0f), rope({ 0.0f, 0.0f }, { 0.4f, -0.5f }, 20), cloth(10, 10)
{
}

void QbitAppLayer::OnAttach()
{
    QB_PROFILE_FUNCTION();
}

void QbitAppLayer::OnDetach()
{
    QB_PROFILE_FUNCTION();
}

static void DrawDoublePendulum(const QP::Pendulum& pendulum1, const QP::Pendulum& pendulum2, const QP::State& state)
{
    glm::vec2 size = { 0.1f, 0.1f };
    glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
    glm::vec4 rod_color = { 0.8f, 0.8f, 0.8f, 1.0f };

    glm::vec2 origin = { 0.0f, 0.0f };
    glm::vec2 position1 = {
        origin.x + pendulum1.length * sin(state.theta1),
        origin.y - pendulum1.length * cos(state.theta1)
    };
    glm::vec2 position2 = {
        position1.x + pendulum2.length * sin(state.theta2),
        position1.y - pendulum2.length * cos(state.theta2)
    };

    Qbit::Renderer2D::DrawLine(glm::vec3{ origin.x, origin.y, 0.0f }, glm::vec3{ position1.x, position1.y, 0.0f }, rod_color);
    Qbit::Renderer2D::DrawLine(glm::vec3{ position1.x, position1.y, 0.0f }, glm::vec3{ position2.x, position2.y, 0.0f }, rod_color);

    Qbit::Renderer2D::DrawQuad(position1, size, color);
    Qbit::Renderer2D::DrawQuad(position2, size, color);
}

static void DrawCloth(const QP::Cloth& cloth)
{
    for (size_t y = 0; y < cloth.height; ++y) {
        for (size_t x = 0; x < cloth.width; ++x) {
            if (x < cloth.width - 1) {
                // Draw horizontal lines
                Qbit::Renderer2D::DrawLine(
                    glm::vec3{ cloth.particles[y * cloth.width + x].position.x, cloth.particles[y * cloth.width + x].position.y, 0.0f },
                    glm::vec3{ cloth.particles[y * cloth.width + (x + 1)].position.x, cloth.particles[y * cloth.width + (x + 1)].position.y, 0.0f },
                    glm::vec4(1.0f));
            }
            if (y < cloth.height - 1) {
                // Draw vertical lines
                Qbit::Renderer2D::DrawLine(
                    glm::vec3{ cloth.particles[y * cloth.width + x].position.x, cloth.particles[y * cloth.width + x].position.y, 0.0f },
                    glm::vec3{ cloth.particles[(y + 1) * cloth.width + x].position.x, cloth.particles[(y + 1) * cloth.width + x].position.y, 0.0f },
                    glm::vec4(1.0f));
            }
        }
    }
}

QP::Timestep ts = 0.0016;

static void DrawRope(const QP::Rope& rope)
{
    auto& particles = rope.particles;

    for (size_t i = 0; i < particles.size() - 1; ++i) {
        auto& pos = particles[i].position;
        auto& pos1 = particles[i + 1].position;
        Qbit::Renderer2D::DrawLine(glm::vec3{ pos.x, pos.y, 0.0f }, glm::vec3{ pos1.x, pos1.y, 0.0f }, glm::vec4(1.0f));
    }
}

void QbitAppLayer::OnUpdate(Qbit::Timestep ts)
{
    QB_PROFILE_FUNCTION();

    m_CameraController.OnUpdate(ts);

    Qbit::Renderer2D::ResetStats();
    {
        QB_PROFILE_SCOPE("RendererPrep");
        Qbit::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        Qbit::RenderCommand::Clear();
    }
    {
        QB_PROFILE_SCOPE("Renderer - Draw");
        Qbit::Renderer2D::BeginScene(m_CameraController.GetCamera());


        switch (g_CurrentSimulationMode)
        {
        case SIMULATION_DOUBLE_PENDULUM:
            if (Qbit::Input::IsMouseButtonPressed(Qbit::Mouse::Button0))
            {
                auto mousePos = Qbit::Input::GetMousePosition();
                QP::Vec2 mouseForce(mousePos.x, mousePos.y);
                QP::applyMouseForce(pendulum1, pendulum2, state, params, mouseForce);
            }
            runge_kutta_step(pendulum1, pendulum2, params, state, ts);
            DrawDoublePendulum(pendulum1, pendulum2, state);
            break;

        case SIMULATION_CLOTH:
            if (Qbit::Input::IsMouseButtonPressed(Qbit::Mouse::Button0))
            {
                auto mousePos = Qbit::Input::GetMousePosition();
                QP::Vec2 mouseForce(mousePos.x, mousePos.y);

                cloth.applyMouseForce(mouseForce * 0.01f);
            }
            cloth.update(ts);
            DrawCloth(cloth);
            break;

        case SIMULATION_ROPE:
            if (Qbit::Input::IsMouseButtonPressed(Qbit::Mouse::Button0))
            {
                auto mousePos = Qbit::Input::GetMousePosition();
                QP::Vec2 mouseForce(mousePos.x, mousePos.y);
                rope.applyMouseForce(mouseForce * 0.01f);
            }
            rope.update(ts);
            DrawRope(rope);
            break;
        }

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

    const char* simulationModes[] = { "Double Pendulum", "Cloth", "Rope" };
    int currentSimulationMode = static_cast<int>(g_CurrentSimulationMode);
    if (ImGui::Combo("Simulation Mode", &currentSimulationMode, simulationModes, IM_ARRAYSIZE(simulationModes)))
    {
        g_CurrentSimulationMode = static_cast<SimulationMode>(currentSimulationMode);
    }

    ImGui::Text("Timestep: %.4f ms", (float)ts);

    ImGui::End();
}

void QbitAppLayer::OnEvent(Qbit::Event& e)
{
    m_CameraController.OnEvent(e);
}

#include "simulation.cpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include <Headers/configuration.hpp>
#include <SFML/Graphics.hpp>
#include <thread>

class GUI
{
private:
    int count = 100000;
    int row_size = 600;
    int substeps = 1;
    int max_framerate = 170;

    float radius = 1;
    float constraintx = 1920;
    float constrainty = 1080;

    double mod = 2.5;
    double restitution = 1;
    double startingvel = 50000;
    double timescale = 1;

    uint32_t threadpoolsize = 1;
    uint32_t numThreads = std::thread::hardware_concurrency();
    uint32_t step = 1;

    bool ThermalColors = true;

    void SetTheme()
    {
        ImGuiIO& io = ImGui::GetIO();
        ImFont* font = io.Fonts->AddFontFromFileTTF("files\\Consolas.ttf", 24);
        ImGui::SFML::UpdateFontTexture();

        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;
        colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);

        style.WindowRounding = 5.0f;
        style.FrameRounding = 3.0f;
        style.GrabRounding = 2.0f;
    }

public:


    void Start()
    {
        static bool isRunning = false;

        sf::RenderWindow window(sf::VideoMode(conf::gui_size.x, conf::gui_size.y), "SFML Physics Project Initializer", sf::Style::Titlebar | sf::Style::Close);
        window.setFramerateLimit(conf::max_framerate);

        ImGui::SFML::Init(window, false);

        SetTheme();

        sf::Clock deltaClock;
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                ImGui::SFML::ProcessEvent(window, event);

                if (event.type == sf::Event::Closed) {
                    window.close();
                }
            }

            ImGui::SFML::Update(window, deltaClock.restart());

            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(conf::gui_size.x, conf::gui_size.y), ImGuiCond_Always);

            ImGui::Begin("SFML Verlet Physics", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

            if (ImGui::CollapsingHeader("Ball Settings", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::InputInt("Balls", &count, 100, 1000);
                count = std::clamp(count, 1, 2000000);

                ImGui::InputFloat("Radius", &radius, 1.0f, 10.0f);
                radius = std::clamp(radius, 0.5f, 1000.0f);

                ImGui::InputInt("Row Size", &row_size, 50, 100);
                row_size = std::clamp(row_size, 1, 2000);

                ImGui::InputDouble("Ball Spacing", &mod, 1, 2);
                mod = std::clamp(mod, 2.0, 10.0);

                ImGui::Checkbox("Thermal Colors", &ThermalColors);
            }

            if (ImGui::CollapsingHeader("Simulation Settings", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::InputFloat("Constraint X", &constraintx, 100.0f, 200.0f);
                constraintx = std::clamp(constraintx, 100.0f, 5000.0f);

                ImGui::InputFloat("Constraint Y", &constrainty, 1.0f, 10.0f);
                constrainty = std::clamp(constrainty, 100.0f, 5000.0f);

                ImGui::InputInt("Max Framerate", &max_framerate, 10, 50);
                max_framerate = std::clamp(max_framerate, 10, 9999);

                ImGui::InputInt("Substeps", &substeps, 1, 10);
                substeps = std::clamp(substeps, 1, 100);

                ImGui::InputDouble("Starting Velocity", &startingvel, 1000, 10000);
                startingvel = std::clamp(startingvel, 0.0, 500000.0);

                ImGui::InputDouble("Restitution", &restitution, 0.01, 0.2);
                restitution = std::clamp(restitution, 0.0, 1.0);

                ImGui::InputDouble("Timescale", &timescale, 0.2, 1);
                timescale = std::clamp(timescale, 0.01, 10.0);

                ImGui::InputScalar("Threadpool Size", ImGuiDataType_U32, &threadpoolsize, &step, &step);
                threadpoolsize = std::clamp(threadpoolsize, 1u, numThreads);
            }

            if (ImGui::Button("Start Simulation") && !isRunning)
            {
                isRunning = true;
                Simulation* sim = new Simulation(count, row_size, substeps, max_framerate, radius, constraintx, constrainty, mod, restitution, startingvel, (1.0/static_cast<double>(max_framerate)) * timescale, ThermalColors, threadpoolsize);
                sim->Solution();
                isRunning = false;
                delete sim;
            }
            ImGui::End();

            window.clear();
            ImGui::SFML::Render(window);
            window.display();
        }

        ImGui::SFML::Shutdown();
    }
};
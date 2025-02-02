#include <thread>
#include <algorithm>
#include <memory>
#include <fstream>
#include <iostream>

#include <SFML/Graphics.hpp>
#include "imgui.h"
#include "imgui-SFML.h"

#include "simulation.cpp"

#include <Headers/configuration.hpp>


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

    uint32_t resolutionx = 1920;
    uint32_t resolutiony = 1080;
    uint32_t resolutionstep = 100;

    double mod = 2.5;
    double restitution = 1;
    double startingvel = 1;
    double timescale = 1;
    
    double gravity = 9.8;

    uint32_t threadpoolsize = 1;
    uint32_t numThreads = std::thread::hardware_concurrency();
    uint32_t step = 1;

    bool ThermalColors = true;
    bool fullscreen = false;

    friend std::istream& operator >> (std::istream& in, GUI& gui) {
        in >> gui.count
            >> gui.row_size
            >> gui.substeps
            >> gui.max_framerate
            >> gui.radius
            >> gui.constraintx
            >> gui.constrainty
            >> gui.resolutionx
            >> gui.resolutiony
            >> gui.resolutionstep
            >> gui.mod
            >> gui.restitution
            >> gui.startingvel
            >> gui.timescale
            >> gui.gravity
            >> gui.threadpoolsize
            >> gui.numThreads
            >> gui.step
            >> gui.ThermalColors
            >> gui.fullscreen;
        return in;
    }

    friend std::ostream& operator << (std::ostream& out, const GUI& gui) {
        out << gui.count << " "
            << gui.row_size << " "
            << gui.substeps << " "
            << gui.max_framerate << " "
            << gui.radius << " "
            << gui.constraintx << " "
            << gui.constrainty << " "
            << gui.resolutionx << " "
            << gui.resolutiony << " "
            << gui.resolutionstep << " "
            << gui.mod << " "
            << gui.restitution << " "
            << gui.startingvel << " "
            << gui.timescale << " "
            << gui.gravity << " "
            << gui.threadpoolsize << " "
            << gui.numThreads << " "
            << gui.step << " "
            << gui.ThermalColors << " "
            << gui.fullscreen;
        return out;
    }

    void load()
    {
        std::ifstream inFile("save.txt");

        if (!inFile) {
            return;
        }

        inFile >> *this;

        if (inFile.fail()) {
            inFile.close();
            return;
        }

        inFile.close();
    }

    void save()
    {
        std::ofstream outFile("save.txt");

        if (!outFile)
        {
            return;
        }

        outFile << *this;

        if (outFile.fail())
        {
            outFile.close();
            return;
        }

        outFile.close();
    }

    void SetTheme()
    {
        ImGuiIO& io = ImGui::GetIO();
        ImFont* font = io.Fonts->AddFontFromFileTTF("files\\Consolas.ttf", 24);
        (void)ImGui::SFML::UpdateFontTexture();

        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.70f);
        colors[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.50f, 0.50f, 0.50f, 0.00f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        style.Colors[ImGuiCol_Separator] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);

        style.WindowRounding = 0.0f;
        style.FrameRounding = 0.0f;
        style.GrabRounding = 0.0f;
    }

public:

    void Start()
    {
        static bool isRunning = false;

        sf::RenderWindow window(sf::VideoMode({ conf::gui_size.x, conf::gui_size.y }), "SFML Physics Project Initializer", sf::Style::Titlebar | sf::Style::Close);
        window.setFramerateLimit(conf::max_framerate);

        (void)ImGui::SFML::Init(window, false);

        SetTheme();

        sf::Clock deltaClock;
        while (window.isOpen()) {
            while (const auto event = window.pollEvent())
            {
                ImGui::SFML::ProcessEvent(window, *event);

                if (event->is<sf::Event::Closed>())
                {
                    window.close();
                }
            }

            ImGui::SFML::Update(window, deltaClock.restart());

            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(conf::gui_size.x, conf::gui_size.y), ImGuiCond_Always);

            ImGui::Begin("SFML Verlet Physics", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

            ImGui::Separator();

            if (ImGui::CollapsingHeader("Ball Settings", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::InputInt("Balls", &count, 100, 1000);
                count = std::clamp(count, 1, 5000000);

                ImGui::InputFloat("Radius", &radius, 1.0f, 10.0f);
                radius = std::clamp(radius, 0.5f, 1000.0f);

                ImGui::InputInt("Row Size", &row_size, 50, 100);
                row_size = std::clamp(row_size, 1, 2000);

                ImGui::InputDouble("Ball Spacing", &mod, 1, 2);
                mod = std::clamp(mod, 2.0, 10.0);

                ImGui::Checkbox("Thermal Colors", &ThermalColors);
            }

            ImGui::Separator();

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

                ImGui::InputDouble("Starting Velocity", &startingvel, 0.25, 2);
                startingvel = std::clamp(startingvel, 0.0, 20.0);

                ImGui::InputDouble("Restitution", &restitution, 0.01, 0.2);
                restitution = std::clamp(restitution, 0.0, 1.0);

                ImGui::InputDouble("Timescale", &timescale, 0.2, 1);
                timescale = std::clamp(timescale, 0.01, 10.0);

                ImGui::InputDouble("Gravity", &gravity, 50, 200);
                gravity = std::clamp(gravity, -275.0, 275.0);

                ImGui::InputScalar("Threadpool Size", ImGuiDataType_U32, &threadpoolsize, &step, &step);
                threadpoolsize = std::clamp(threadpoolsize, 1u, numThreads);
            }

            ImGui::Separator();

            if (ImGui::CollapsingHeader("Window Settings", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Checkbox("Fullscreen", &fullscreen);

                ImGui::InputScalar("Resolution X", ImGuiDataType_U32, &resolutionx, &resolutionstep, &resolutionstep);
                resolutionx = std::clamp(resolutionx, 600u, 3840u);

                ImGui::InputScalar("Resolution Y", ImGuiDataType_U32, &resolutiony, &resolutionstep, &resolutionstep);
                resolutiony = std::clamp(resolutiony, 400u, 2160u);
            }

            ImGui::Separator();

            if (ImGui::CollapsingHeader("Saves"))
            {
                if (ImGui::Button("Load Settings"))
                {
                    load();
                }

                if (ImGui::Button("Save Settings"))
                {
                    save();
                }
            }

            ImGui::Separator();

            if (ImGui::Button("Start Simulation") && !isRunning)
            {
                isRunning = true;
                {
                    auto sim = std::make_unique<Simulation>(
                        count, row_size, substeps, max_framerate, radius,
                        constraintx, constrainty, mod, restitution,
                        startingvel, (1.0 / static_cast<double>(max_framerate)) * timescale,
                        ThermalColors, threadpoolsize, gravity * 10, fullscreen, resolutionx, resolutiony
                    );
                    sim->Solution();
                }
                isRunning = false;
            }
            ImGui::End();

            window.clear();
            ImGui::SFML::Render(window);
            window.display();
        }

        ImGui::SFML::Shutdown();
    }
};
#include "simulation.cpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include <Headers/configuration.hpp>

#include <SFML/Graphics.hpp>

class GUI
{
public:
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

    void Start()
    {
        static bool isRunning = false;

        sf::RenderWindow window(sf::VideoMode(800, 600), "SFML Physics Project Initializer", sf::Style::Titlebar | sf::Style::Close);
        window.setFramerateLimit(conf::max_framerate);
        ImGui::SFML::Init(window);

        sf::CircleShape shape(100.f);
        shape.setPosition(sf::Vector2f(800/2 - shape.getRadius(), 600/2 - shape.getRadius()));
        shape.setFillColor(sf::Color(128,128,128));

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
            ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Always);

            ImGui::Begin("SFML Verlet Physics", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

            ImGui::InputInt("Balls", &count, 100, 1000);
            count = std::clamp(count, 1, 2000000);

            ImGui::InputFloat("Radius", &radius, 1.0f, 10.0f);
            radius = std::clamp(radius, 0.5f, 1000.0f);

            ImGui::InputInt("Row Size", &row_size, 50, 100);
            row_size = std::clamp(row_size, 1, 2000);

            ImGui::InputDouble("Mod", &mod, 1, 2);
            mod = std::clamp(mod, 2.0, 10.0);

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

            ImGui::InputDouble("Timescale", &restitution, 0.2, 1);
            timescale = std::clamp(timescale, 0.01, 10.0);

            if (ImGui::Button("Start") && !isRunning)
            {
                isRunning = true;
                Simulation* sim = new Simulation(count, row_size, substeps, max_framerate, radius, constraintx, constrainty, mod, restitution, startingvel, (1.0/static_cast<double>(max_framerate)) * timescale);
                sim->Solution();
                isRunning = false;
                delete sim;
            }
            ImGui::End();

            window.clear();
            window.draw(shape);
            ImGui::SFML::Render(window);
            window.display();
        }

        ImGui::SFML::Shutdown();
    }
};
#include <SFML/Graphics.hpp>

#include <Headers/configuration.hpp>
#include <Headers/verlet.hpp>

#include <SFML/Audio.hpp>
#include <iostream>

#include "solver.cpp"

class Simulation
{
private:
    int count;
    const int row_size;
    const int substeps;
    const int max_framerate;
    const bool ThermalColors;

    const float radius;
    const float constraintx;
    const float constrainty;

    const double mod;
    const double restitution;
    const double startingvel;
    const double dt;
    const uint32_t threadpoolsize;

    const bool fullscreen;
    const uint32_t resolutionx;
    const uint32_t resolutiony;


    sf::Vector2f const constraints;

    double gravity;

    void loadfiles(sf::Font& consolas, sf::Text& energy, sf::Text& deltatime, sf::Text& count, sf::Texture& texture)
    {
        if (!consolas.loadFromFile("files\\Consolas.ttf"))
        {
            std::cout << "Error\n";
        }
        if (!texture.loadFromFile("files\\circle.png"))
        {
            std::cout << "Error\n";
        }

        energy.setFont(consolas);
        deltatime.setFont(consolas);
        deltatime.setPosition(sf::Vector2f(0, 100));
        count.setFont(consolas);
        count.setPosition(sf::Vector2f(0, 200));
    }

    void checkinput(sf::RenderWindow& window, Solver& solver, float& zoomFactor, int& deltaY, int& deltaX, sf::View& view, sf::Text& count)
    {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) 
        {
            sf::Vector2i mousepos = sf::Mouse::getPosition(window);
            sf::Vector2f globalpos = window.mapPixelToCoords(mousepos, window.getView());
            solver.AddBall(globalpos);
            count.setString(std::to_string(solver.count) + " Balls");
        }
        if (sf::Mouse::isButtonPressed(sf::Mouse::Middle))
        {
            sf::Vector2i mousepos = sf::Mouse::getPosition(window);
            sf::Vector2f globalpos = window.mapPixelToCoords(mousepos, window.getView());
            solver.mouse(globalpos, dt);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) 
        {
            zoomFactor += 0.10 * (constraints.x/1920);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && zoomFactor > 0.01f) {
            zoomFactor -= 0.10 * (constraints.x / 1920);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) && constraints.y / 2 + deltaY > 0) {
            deltaY -= constraints.x / 300;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) && constraints.y / 2 + deltaY < constraints.y) {
            deltaY += constraints.x / 300;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) && constraints.x / 2 + deltaX > 0) {
            deltaX -= constraints.x / 300;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) && constraints.x / 2 + deltaX < constraints.x) {
            deltaX += constraints.x / 300;
        }
        view.setSize(resolutionx * zoomFactor, resolutiony * zoomFactor);
        view.setCenter(resolutionx / 2 + deltaX, resolutiony / 2 + deltaY);
    }


public:

    bool isRunning = true;

    Simulation(
        const int _count = 100000,
        const int _row_size = 600,
        const int _substeps = 1,
        const int _max_framerate = 170,
        const float _radius = 1.0f,
        const float _constraintx = 1920.0f,
        const float _constrainty = 1080.0f,
        const double _mod = 2.5,
        const double _restitution = 1.0,
        const double _startingvel = 5.0,
        const double _dt = 1.0 / 170.0,
        const bool _ThermalColors = true,
        const uint32_t _threadpoolsize = 16,
        const double _gravity = 98.0,
        const bool _fullscreen = false,
        uint32_t _resolutionx = 1920,
        uint32_t _resolutiony = 1080
    ) :
        count(_count),
        row_size(_row_size),
        substeps(_substeps),
        max_framerate(_max_framerate),
        radius(_radius),
        constraintx(_constraintx),
        constrainty(_constrainty),
        mod(_mod),
        restitution(_restitution),
        startingvel(_startingvel),
        constraints(sf::Vector2f(_constraintx, constrainty)),
        dt(_dt),
        ThermalColors(_ThermalColors),
        threadpoolsize(_threadpoolsize),
        gravity(_gravity),
        fullscreen(_fullscreen),
        resolutionx(_resolutionx),
        resolutiony(_resolutiony)
    {}

    void Solution()
    {
        tp::ThreadPool thread_pool(threadpoolsize);

        sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();

        sf::RenderWindow window;

        if (fullscreen)
        {
            window.create(desktopMode, "SFML Verlet Integration", sf::Style::Fullscreen);
        }
        else
        {
            window.create(sf::VideoMode(resolutionx, resolutiony), "SFML Verlet Integration", sf::Style::Titlebar | sf::Style::Close);
        }

        window.setFramerateLimit(max_framerate);
        sf::View view(sf::FloatRect(0, 0, resolutionx, resolutiony));
        view.setCenter(resolutionx / 2, resolutiony / 2);
        window.setView(view);

        sf::VertexArray vertices(sf::Quads);

        Solver solver(thread_pool, vertices, count, substeps, row_size, radius, restitution, startingvel, mod, constraints, ThermalColors);

        sf::Font consolas;
        sf::Text energy;
        sf::Text deltatime;
        sf::Text count;
        sf::Clock clock;
        sf::Texture texture;

        loadfiles(consolas, energy, deltatime, count, texture);

        sf::RectangleShape blackBackground(sf::Vector2f(constraints.x, constraints.y));
        blackBackground.setPosition(0, 0);
        blackBackground.setFillColor(sf::Color(0, 0, 0));

        float zoomFactor = 1;
        int deltaY = 0;
        int deltaX = 0;

        sf::RenderStates states;
        states.texture = &texture;

        count.setString(std::to_string(solver.count) + " Balls");

        while (window.isOpen())
        {

            for (auto event = sf::Event(); window.pollEvent(event);)
            {
                if (event.type == sf::Event::Closed)
                {
                    window.close();
                }
                if ((event.type == sf::Event::KeyReleased) && (event.key.code == sf::Keyboard::Escape))
                {
                    if (isRunning)
                    {
                        isRunning = false;
                    }
                    else
                    {
                        isRunning = true;
                    }
                }
                if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::R))
                {
                    view.setCenter(resolutionx / 2, resolutiony / 2);
                    zoomFactor = 1;
                    deltaY = 0;
                    deltaX = 0;
                }
            }

            float currenttime = clock.restart().asSeconds();

            if (isRunning)
            {
                vertices.clear();
                window.clear(sf::Color(100, 100, 100));
                window.draw(blackBackground);

                solver.update(dt, gravity);

                if (window.hasFocus())
                {
                    checkinput(window, solver, zoomFactor, deltaY, deltaX, view, count);
                }

                energy.setString(std::to_string(solver.Total_energy) + " kilogram pixels\xB2/second\xB2");
                deltatime.setString(std::to_string(1.0f / currenttime) + " FPS");
                window.setView(view);
                window.draw(vertices, states);
                window.draw(deltatime);
                window.draw(energy);
                window.draw(count);
                window.display();
                currenttime = 0;
            }

        }
    }
};
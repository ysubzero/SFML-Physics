#pragma once
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

    const float radius;
    const float constraintx;
    const float constrainty;

    const double mod;
    const double restitution;
    const double startingvel;
    const double dt;

    sf::Vector2f const constraints;

    void loadfiles(sf::Font& consolas, sf::Text& energy, sf::Text& deltatime, sf::Text& count, sf::Texture& texture)
    {
        if (!consolas.loadFromFile("C:\\Users\\Jandy\\source\\SFML-Physics\\SFML-Physics\\files\\Consolas.ttf"))
        {
            std::cout << "Error\n";
        }
        if (!texture.loadFromFile("C:\\Users\\Jandy\\source\\SFML-Physics\\SFML-Physics\\files\\circle.png"))
        {
            std::cout << "Error\n";
        }

        energy.setFont(consolas);
        deltatime.setFont(consolas);
        deltatime.setPosition(sf::Vector2f(0, 100));
        count.setFont(consolas);
        count.setPosition(sf::Vector2f(0, 200));
    }

    void checkinput(sf::RenderWindow& window, Solver& solver, float& zoomFactor, int& deltaY, int& deltaX, sf::View& view)
    {
        static int stopinfiniteball = 2;
        sf::Vector2i mousepos = sf::Mouse::getPosition(window);
        sf::Vector2f globalpos = window.mapPixelToCoords(mousepos, window.getView());
        if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && stopinfiniteball < 0) {
            solver.AddBall(globalpos);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            zoomFactor += 1 * dt;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            zoomFactor += 1 * dt;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && zoomFactor > 0.01f) {
            zoomFactor -= 1 * dt;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) && constraints.y / 2 + deltaY > 0) {
            deltaY -= 10;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) && constraints.y / 2 + deltaY < constraints.y) {
            deltaY += 10;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) && constraints.x / 2 + deltaX > 0) {
            deltaX -= 10;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) && constraints.x / 2 + deltaX < constraints.x) {
            deltaX += 10;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
            view.setCenter(conf::window_size.x / 2, conf::window_size.y / 2);
            zoomFactor = 1;
            deltaY = 0;
            deltaX = 0;
        }
        view.setSize(conf::window_size.x * zoomFactor, conf::window_size.y * zoomFactor);
        view.setCenter(conf::window_size.x / 2 + deltaX, conf::window_size.y / 2 + deltaY);
        stopinfiniteball--;
    }


public:
    Simulation(
        int _count = 100000,
        int _row_size = 600,
        int _substeps = 1,
        int _max_framerate = 170,
        float _radius = 1.0f,
        float _constraintx = 1920.0f,
        float _constrainty = 1080.0f,
        double _mod = 2.5,
        double _restitution = 1.0,
        double _startingvel = 100.0,
        double _dt = 1.0/170.0
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
        dt(_dt)
    {}

    void Solution()
    {
        tp::ThreadPool thread_pool(16);

        sf::RenderWindow window({ conf::window_size.x, conf::window_size.y }, "SFML Verlet Integration");
        window.setFramerateLimit(conf::max_framerate);
        sf::View view(sf::FloatRect(0, 0, conf::window_size.x, conf::window_size.y));
        view.setCenter(conf::window_size.x / 2, conf::window_size.y / 2);
        window.setView(view);

        sf::VertexArray vertices(sf::Quads);

        Solver solver(thread_pool, vertices, count, substeps, row_size, radius, restitution, startingvel, mod, constraints);

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


        while (window.isOpen())
        {
            vertices.clear();
            window.clear(sf::Color(100, 100, 100));
            window.draw(blackBackground);

            for (auto event = sf::Event(); window.pollEvent(event);)
            {
                if (event.type == sf::Event::Closed)
                {
                    window.close();
                }
            }

            solver.update(dt);
            window.draw(vertices, states);

            if (window.hasFocus())
            {
                checkinput(window, solver, zoomFactor, deltaY, deltaX, view);
            }

            energy.setString(std::to_string(solver.Total_energy) + " kilogram pixels\xB2/second\xB2");
            float currenttime = clock.restart().asSeconds();
            deltatime.setString(std::to_string(1.0f / currenttime) + " FPS");
            count.setString(std::to_string(solver.count) + " Balls");
            window.setView(view);
            window.draw(deltatime);
            window.draw(energy);
            window.draw(count);
            window.display();
            solver.Total_energy = 0;
            currenttime = 0;

        }
    }
};
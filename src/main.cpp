#include <SFML/Graphics.hpp>
#include <configuration.hpp>
#include <verlet.hpp>
#include <verletpendulum.hpp>
#include <SFML/Audio.hpp>
#include <iostream>

int main()
{
    Solver solver;

    sf::RenderWindow window({ conf::window_size.x, conf::window_size.y }, "SFML Verlet Integration");
    window.setFramerateLimit(conf::max_framerate);

    sf::View view(sf::FloatRect(0, 0, conf::window_size.x, conf::window_size.y));

    view.setCenter(conf::window_size.x / 2, conf::window_size.y / 2);

    window.setView(view);

    sf::Font consolas;
    consolas.loadFromFile("C:\\Users\\Jandy\\source\\SFML-Physics\\SFML-Physics\\files\\Consolas.ttf");

    sf::Text energy;
    energy.setFont(consolas);
    sf::Text deltatime;
    deltatime.setFont(consolas);
    deltatime.setPosition(sf::Vector2f(0, 100));


    sf::SoundBuffer collision;
    if (!collision.loadFromFile("C:\\Users\\Jandy\\source\\SFML-Physics\\SFML-Physics\\files\\collision.wav"))
    {
        std::cout << "Error" << std::endl;
    }
    sf::Sound collisionsound;
    collisionsound.setBuffer(collision);
    sf::Clock clock;

    sf::Texture texture;
    if (!texture.loadFromFile("C:\\Users\\Jandy\\source\\SFML-Physics\\SFML-Physics\\files\\circle.png"))
    {
    }

    sf::VertexArray vertices(sf::Quads);

    sf::RectangleShape blackBackground(sf::Vector2f(conf::constraints.x, conf::constraints.y));
    blackBackground.setPosition(0, 0);
    blackBackground.setFillColor(sf::Color::Black);

    float zoomFactor = 1;
    int deltaY = 0;
    int deltaX = 0;

    while (window.isOpen())
    {
        window.clear(sf::Color(128,128,128));


        for (auto event = sf::Event(); window.pollEvent(event);)
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        solver.update(conf::dt);

        for (int i = 0; i < solver.count; i++)
        {
            if (solver.ball[i].collide == true)
            {
               // collisionsound.play();
            }
       }

        window.draw(blackBackground);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            zoomFactor += 1 * conf::dt;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && zoomFactor > 0.01f) {
            zoomFactor -= 1 *conf::dt;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) && conf::constraints.y / 2 + deltaY > 0) {
            deltaY -= 10;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) && conf::constraints.y / 2 + deltaY < conf::constraints.y) {
            deltaY += 10;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) && conf::constraints.x / 2 + deltaX > 0) {
            deltaX -= 10;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) && conf::constraints.x / 2 + deltaX < conf::constraints.x) {
            deltaX += 10;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
            sf::View view(sf::FloatRect(0, 0, conf::window_size.x, conf::window_size.y));
            view.setCenter(conf::window_size.x / 2, conf::window_size.y / 2);
            zoomFactor = 1;
            deltaY = 0;
            deltaX = 0;
        }

        view.setSize(conf::window_size.x * zoomFactor, conf::window_size.y * zoomFactor);
        view.setCenter(conf::window_size.x / 2 + deltaX, conf::window_size.y / 2 + deltaY);

        energy.setString(std::to_string(solver.energy));
        float currenttime = clock.restart().asSeconds();
        deltatime.setString(std::to_string(1.0f/currenttime));
        window.setView(view);
        window.draw(deltatime);
        window.draw(energy);
        solver.toVertexArray(vertices, solver.ball);
        sf::RenderStates states;
        states.texture = &texture;
        window.draw(vertices, states);
        window.display();
        solver.energy = 0;
        currenttime = 0;

    }
}

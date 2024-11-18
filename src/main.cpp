#include <SFML/Graphics.hpp>
#include <configuration.hpp>
#include <verlet.hpp>
#include <verletpendulum.hpp>
#include <SFML/Audio.hpp>
#include <iostream>

int main()
{
    PendulumSolver solver;

    auto window = sf::RenderWindow({ conf::window_size.x, conf::window_size.y }, "SFML Verlet Integration");
    window.setFramerateLimit(conf::max_framerate);

    sf::Font consolas;
    consolas.loadFromFile("C:\\Users\\Jandy\\source\\SFML-Physics\\SFML-Physics\\files\\Consolas.ttf");

    sf::Text energy;
    energy.setFont(consolas);

    sf::Text angle;
    angle.setFont(consolas);
    angle.setPosition(sf::Vector2f(0, conf::window_size.y - 50));

    sf::SoundBuffer collision;
    if (!collision.loadFromFile("C:\\Users\\Jandy\\source\\SFML-Physics\\SFML-Physics\\files\\collision.wav"))
    {
        std::cout << "Error" << std::endl;
    }
    sf::Sound collisionsound;
    collisionsound.setBuffer(collision);

    while (window.isOpen())
    {
        window.clear();

        for (auto event = sf::Event(); window.pollEvent(event);)
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        solver.update(conf::dt, window);


        if (solver.pendulum.collide == true)
        {
            collisionsound.play();
        }

        energy.setString(std::to_string(solver.pendulum.Energy));
        angle.setString(std::to_string(std::abs(fmod(solver.pendulum.angle ,(3.14159 * 2)))));

        window.draw(energy);
        window.draw(angle);

        solver.render(window);

        window.display();

    }
}

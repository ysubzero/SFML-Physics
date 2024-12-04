#include <SFML/Graphics.hpp>
#include <configuration.hpp>
#include <verlet.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include "solver.cpp"

void loadfiles(sf::Font &consolas, sf::Text &energy, sf::Text &deltatime, sf::SoundBuffer &collision, sf::Sound collisionsound, sf::Texture &texture)
{
    if (!consolas.loadFromFile("C:\\Users\\Jandy\\source\\SFML-Physics\\SFML-Physics\\files\\Consolas.ttf"))
    {

    }
    if (!collision.loadFromFile("C:\\Users\\Jandy\\source\\SFML-Physics\\SFML-Physics\\files\\collision.wav"))
    {
        std::cout << "Error" << std::endl;
    }
    if (!texture.loadFromFile("C:\\Users\\Jandy\\source\\SFML-Physics\\SFML-Physics\\files\\circle.png"))
    {
    }
    energy.setFont(consolas);
    deltatime.setFont(consolas);
    deltatime.setPosition(sf::Vector2f(0, 100));
    collisionsound.setBuffer(collision);
}

void checkinput(sf::RenderWindow &window, Solver &solver,float &zoomFactor, int &deltaY, int &deltaX, sf::View &view)
{
    static int stopinfiniteball = 2;
    sf::Vector2i mousepos = sf::Mouse::getPosition(window);
    sf::Vector2f globalpos = window.mapPixelToCoords(mousepos, window.getView());
    if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && stopinfiniteball < 0) {
        solver.AddBall(globalpos);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
        zoomFactor += 1 * conf::dt;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
        zoomFactor += 1 * conf::dt;
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && zoomFactor > 0.01f) {
        zoomFactor -= 1 * conf::dt;
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
        view.setCenter(conf::window_size.x / 2, conf::window_size.y / 2);
        zoomFactor = 1;
        deltaY = 0;
        deltaX = 0;
    }
    stopinfiniteball--;
}

int main()
{
    tp::ThreadPool thread_pool(16);
    Solver solver(thread_pool);

    sf::RenderWindow window({ conf::window_size.x, conf::window_size.y }, "SFML Verlet Integration");
    window.setFramerateLimit(conf::max_framerate);
    sf::View view(sf::FloatRect(0, 0, conf::window_size.x, conf::window_size.y));
    view.setCenter(conf::window_size.x / 2, conf::window_size.y / 2);
    window.setView(view);

    sf::Font consolas;
    sf::Text energy;
    sf::Text deltatime;
    sf::SoundBuffer collision;
    sf::Sound collisionsound;
    sf::Clock clock;
    sf::Texture texture;

    loadfiles(consolas, energy, deltatime, collision, collisionsound, texture);

    sf::VertexArray vertices(sf::Quads);
    sf::RectangleShape blackBackground(sf::Vector2f(conf::constraints.x, conf::constraints.y));
    blackBackground.setPosition(0, 0);
    blackBackground.setFillColor(sf::Color::Black);

    float zoomFactor = 1;
    int deltaY = 0;
    int deltaX = 0;

    sf::RenderStates states;
    states.texture = &texture;


    while (window.isOpen())
    {
        vertices.clear();
        window.clear(sf::Color(128,128,128));
        window.draw(blackBackground);


        for (auto event = sf::Event(); window.pollEvent(event);)
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        solver.update(conf::dt, vertices);
        window.draw(vertices, states);

       // for (int i = 0; i < solver.count; i++)
        //{
            //if (solver.ball[i].collide == true)
            //{
               // collisionsound.play();
            //}
       //}
        if (window.hasFocus())
        {
            checkinput(window, solver, zoomFactor, deltaY, deltaX, view);
        }

        view.setSize(conf::window_size.x * zoomFactor, conf::window_size.y * zoomFactor);
        view.setCenter(conf::window_size.x / 2 + deltaX, conf::window_size.y / 2 + deltaY);

        energy.setString(std::to_string(solver.energy));
        float currenttime = clock.restart().asSeconds();
        deltatime.setString(std::to_string(1.0f/currenttime));
        window.setView(view);
        window.draw(deltatime);
        window.draw(energy);
        window.display();
        solver.energy = 0;
        currenttime = 0;

    }
}

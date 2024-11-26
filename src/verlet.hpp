#include <math.hpp>
#include <configuration.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <omp.h>

struct VerletBall
{
	sf::Vector2<double> position;
	sf::Vector2<double> position_last;
	sf::Vector2<double> position_next;
	sf::Vector2<double> displacement;
	sf::Vector2<double> acceleration;
	sf::Vector2<double> center;
	double Energy;
	float radius;
	bool collide;
	sf::Color color;

	VerletBall()
		: position((conf::window_sizef.x) / 2, 0.0f),
		position_last((conf::window_sizef.x) / 2, 0.0f),
		displacement(position - position_last),
		acceleration(0, 0),
		center(position.x + radius, position.y + radius),
		radius(10.0f),
		Energy(0.0),
		collide(false),
		color(sf::Color::White)
	{}

	void collisioncheck(sf::Vector2<double> position_next, sf::Vector2<double> displacement)
	{
		if (position_next.x > conf::window_sizef.x - 2 * radius || position_next.x < 0)
		{
			if (position_next.x > conf::window_sizef.x - 2 * radius)
			{
				position.x = conf::window_sizef.x - 2 * radius;
			}
			else
			{
				position.x = 0;
			}
			position_last.x = position.x + displacement.x;
			collide = true;
		}

		if (position_next.y > conf::window_sizef.y - 2 * radius || position_next.y < 0)
		{
			if (position_next.y > conf::window_sizef.y - 2 * radius)
			{
				position.y = conf::window_sizef.y - 2 * radius;
			}
			else
			{
				position.y = 0;
			}

			position_last.y = position.y + displacement.y;
			acceleration = sf::Vector2<double>(0.0f, 0.0f);
			collide = true;
		}
	}

	void update(double dt, double gravity)
	{
		collide = false;
		displacement = position - position_last;
		position_next = position + displacement + acceleration * dt * dt;

		position_last = position;
		position = position_next;
		center = sf::Vector2<double>(position.x + radius, position.y + radius);

		acceleration = sf::Vector2<double>(0.0f, gravity);
	}

	void render(sf::RenderWindow& window, sf::Texture& texture)
	{
		sf::Sprite sprite;
		sprite.setTexture(texture);
		sf::Vector2f positionset = sf::Vector2f(static_cast<float>(position.x), static_cast<float>(position.y));
		sprite.setPosition(positionset);
		sprite.setColor(sf::Color(color));
		float scale = radius / 50.0f;
		sprite.setScale(sf::Vector2f(scale, scale));
		window.draw(sprite);
	}
};

class Solver
{
private:
	static const int rowsize = 10;
	static constexpr float radius = 30;
	static constexpr double restitution = 1;
	static constexpr double startingvel = 400000.0f;
	static const int mod = 3;

	void collisionwitheachother(int i, float dt)
	{
		for (int j = 0; j < count; j++)
		{
			const sf::Vector2<double> distVect = ball[i].center - ball[j].center;
			const float distance = std::sqrt(distVect.x * distVect.x + distVect.y * distVect.y);
			const float nx = distVect.x / distance;
			const float ny = distVect.y / distance;

			if (i != j && (distance) < (ball[i].radius + ball[j].radius))
			{
					ball[i].position.x += nx * (ball[i].radius + ball[j].radius - (distance)) * 0.5;
					ball[j].position.x -= nx * (ball[i].radius + ball[j].radius - (distance)) * 0.5;

					ball[i].position.y += ny * (ball[i].radius + ball[j].radius - (distance)) * 0.5;
					ball[j].position.y -= ny * (ball[i].radius + ball[j].radius - (distance)) * 0.5;

				const sf::Vector2<double> ivel = ball[i].displacement;
				const sf::Vector2<double> jvel = ball[j].displacement;
				ball[i].displacement = ivel - distVect * (Math::dot((ivel - jvel), distVect) / (distVect.x * distVect.x + distVect.y * distVect.y)) * restitution;
				ball[j].displacement = jvel - distVect * (Math::dot((jvel - ivel), distVect) / (distVect.x * distVect.x + distVect.y * distVect.y)) * restitution;
				ball[j].position_last = ball[j].position - ball[j].displacement;
				ball[i].position_last = ball[i].position - ball[i].displacement;
				ball[i].collide = true;
			}
		}

		if (ball[i].position.x > conf::window_sizef.x - 2 * ball[i].radius || ball[i].position.x < 0)
		{
			if (ball[i].position.x > conf::window_sizef.x - 2 * ball[i].radius)
			{
				ball[i].position.x = conf::window_sizef.x - 2 * ball[i].radius;
			}
			else
			{
				ball[i].position.x = 0;
			}
			ball[i].position_last.x = ball[i].position.x + (ball[i].displacement.x * restitution);
			ball[i].collide = true;
		}

		if (ball[i].position.y > conf::window_sizef.y - 2 * ball[i].radius || ball[i].position.y < 0)
		{
			if (ball[i].position.y > conf::window_sizef.y - 2 * ball[i].radius)
			{
				ball[i].position.y = conf::window_sizef.y - 2 * ball[i].radius;
				ball[i].acceleration = sf::Vector2<double>(0.0f, 0.0f);
			}
			else
			{
				ball[i].position.y = 0;
			}

			ball[i].position_last.y = ball[i].position.y + (ball[i].displacement.y * restitution);
			ball[i].collide = true;
		}
	}
public:
	static const int count = 30;
	static const int substep = 10;
	double energy = 0;
	VerletBall* ball;
	sf::Texture texture;

	Solver() {
		ball = new VerletBall[count];
		for (int i = 0; i < count; i++)
		{
			ball[i].radius = radius;
			ball[i].position = sf::Vector2<double>(radius * mod * (i % rowsize) + 100, radius * mod * (i / rowsize) + 300);
			ball[i].position_last = sf::Vector2<double>(radius * mod * (i % rowsize) + 100, radius * mod * (i / rowsize) + 300);
			ball[i].acceleration = sf::Vector2<double>(startingvel, 0);
		}
		ball[0].color = sf::Color::Yellow;
		if (!texture.loadFromFile("C:\\Users\\Jandy\\source\\SFML-Physics\\SFML-Physics\\files\\circle.png"))
		{
		}
	}

	void update(float dt)
	{
		for (int t = 0; t < substep; t++)
		{
			for (int i = 0; i < count; i++)
			{
				ball[i].update(dt/substep, Math::gravity);
				collisionwitheachother(i, dt/substep);
				double velocity = (std::sqrt((ball[i].displacement.x * ball[i].displacement.x) + (ball[i].displacement.y * ball[i].displacement.y)));
				ball[i].Energy = (((velocity * velocity) / ((2.0) * dt/substep * dt / substep)));
				energy += ball[i].Energy;
			}
		}
	}

	void render(sf::RenderWindow& window)
	{
		for (int i = 0; i < count; i++)
		{
		ball[i].render(window, texture);
		}
	}
};

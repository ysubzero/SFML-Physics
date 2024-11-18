#include <math.hpp>
#include <configuration.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>

struct VerletBall
{
	sf::Vector2f position;
	sf::Vector2f position_last;
	sf::Vector2f acceleration;
	double Energy;
	float radius;
	bool collide;
	sf::Color color;

	VerletBall()
		: position((conf::window_sizef.x) / 2, 0.0f),
		position_last((conf::window_sizef.x) / 2, 0.0f),
		acceleration(100000000.0f, Math::gravity),
		radius(100.0f),
		Energy(0.0),
		collide(false),
		color(sf::Color::White)
	{}

	void collisioncheck(sf::Vector2f position_next, sf::Vector2f displacement)
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
			acceleration = sf::Vector2f(0.0f, 0.0f);
			collide = true;
		}
	}

	void update(float dt, float gravity)
	{
		collide = false;
		sf::Vector2f displacement = position - position_last;
		sf::Vector2f position_next = position + displacement + acceleration * dt * dt;

		position_last = position;
		position = position_next;

		acceleration = sf::Vector2f(0.0f, Math::gravity);

		collisioncheck(position_next, displacement);

		sf::Vector2f velocitydisp = position - position_last;

		double velocity = (std::sqrt((velocitydisp.x * velocitydisp.x) + (velocitydisp.y * velocitydisp.y)));

		Energy = (((velocity * velocity) / ((2.0) * dt * dt)) + (Math::gravity * ((conf::window_sizef.y - 2 * radius) - position.y )));
	}

	void render(sf::RenderWindow& window)
	{
		sf::CircleShape circle(radius);
		circle.setPosition(position);
		circle.setFillColor(color);
		window.draw(circle);
	}
};

class Solver
{
public:
	VerletBall ball;

	void update(float dt)
	{
		ball.update(dt, Math::gravity);
	}

	void render(sf::RenderWindow& window)
	{
		ball.render(window);
	}
};

#include <math.hpp>
#include <configuration.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>

struct VerletPendulum
{
	float angle;
	float angle_last;
	sf::Vector2f pivot;
	float length;
	sf::Vector2f position;
	sf::Vector2f position_last;
	float acceleration;
	double Energy;
	float radius;
	float pivotradius;
	bool collide;
	sf::Color color;

	VerletPendulum()
		: angle(1),
		angle_last(1),
		pivot((conf::window_sizef.x) / 2, (conf::window_sizef.y) / 2),
		length(300.0f),
		position(pivot.x + length, pivot.y),
		position_last(pivot.x + length, pivot.y),
		acceleration(Math::gravity),
		radius(50.0f),
		pivotradius(10.0f),
		Energy(0.0),
		collide(false),
		color(sf::Color::White)
	{}

	void collisioncheck(sf::Vector2f position_next, sf::Vector2f displacement)
	{
		if (position_next.x > conf::window_sizef.x - 1 * radius || position_next.x < 0)
		{
			if (position_next.x > conf::window_sizef.x - 1 * radius)
			{
				position.x = conf::window_sizef.x - 1 * radius;
			}
			else
			{
				position.x = 0;
			}
			position_last.x = position.x + displacement.x;
			acceleration = -acceleration;
			collide = true;
		}

		if (position_next.y > conf::window_sizef.y - 1 * radius || position_next.y < 0)
		{
			if (position_next.y > conf::window_sizef.y - 1 * radius)
			{
				position.y = conf::window_sizef.y - 1 * radius;
			}
			else
			{
				position.y = 0;
			}

			position_last.y = position.y + displacement.y;
			acceleration = -acceleration;
			collide = true;
		}
	}

	void update(float dt, float gravity, sf::RenderWindow& window)
	{
		sf::Vector2i localPosition = sf::Mouse::getPosition(window);

		if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && (localPosition.x > position.x - radius && localPosition.x < position.x + radius && localPosition.y > position.y - radius && localPosition.y < position.y + radius))
		{
			position.x = localPosition.x;
			position.y = localPosition.y;
			position_last = position;
			angle = std::atan2(pivot.y - position.y, position.x - pivot.x) - 3.14159/2;
			angle_last = angle;
		}
		else {
			collide = false;

			float AngleDisplacement = angle - angle_last;
			float newAngle = angle + AngleDisplacement + acceleration * dt * dt;

			angle_last = angle;
			angle = newAngle;

			sf::Vector2f position_next = sf::Vector2f(pivot.x + pivotradius - (std::sin(angle) * length), pivot.y + pivotradius - (std::cos(angle) * length));

			position_last = position;
			position = position_next;

			acceleration = (Math::gravity * std::sin(angle));
			sf::Vector2f displacement = position - position_last;
			collisioncheck(position_next, displacement);

			double velocity = (std::sqrt((displacement.x / 10 * displacement.x / 10) + (displacement.y / 10 * displacement.y / 10)));

			Energy = (((velocity * velocity) / ((2.0))) + (Math::gravity / 10 * ((conf::window_sizef.y - 2 * radius) / 10 - position.y / 10)));
		}
	}

	void render(sf::RenderWindow& window)
	{
		sf::Vertex line[] = {
		sf::Vertex(sf::Vector2f(pivot.x + pivotradius, pivot.y + pivotradius)),
		sf::Vertex(sf::Vector2f(position.x, position.y))
		};
		window.draw(line, 15, sf::Lines);

		sf::CircleShape circle(radius);
		circle.setPosition(sf::Vector2f(position.x - radius, position.y - radius));
		circle.setFillColor(color);

		sf::CircleShape pivoter(pivotradius);
		pivoter.setPosition(pivot);
		pivoter.setFillColor(color);

		window.draw(circle);
		window.draw(pivoter);
	}
};

class PendulumSolver
{
public:
	VerletPendulum pendulum;

	void update(float dt, sf::RenderWindow& window)
	{
		pendulum.update(dt, Math::gravity, window);
	}

	void render(sf::RenderWindow& window)
	{
		pendulum.render(window);
	}
};

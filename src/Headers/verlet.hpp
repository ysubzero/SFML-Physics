#pragma once
#include <Headers/math.hpp>
#include <Headers/configuration.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <omp.h>
#include <vector>

struct VerletBall
{
	sf::Vector2<double> position;
	sf::Vector2<double> position_last;
	sf::Vector2<double> position_next;
	sf::Vector2<double> displacement;
	sf::Vector2<double> acceleration;
	float radius;
	double Mass;
	double Energy;
	sf::Color color;

	VerletBall()
		: position(0.0, 0.0),
		position_last(0.0, 0.0),
		displacement(position - position_last),
		acceleration(0, 0),
		Mass(1),
		Energy(0),
		radius(1.0f),
		color(sf::Color::White)
	{}

	void update(const double dt, const double gravity)
	{
		displacement = position - position_last;
		position_next = position + displacement + acceleration * dt * dt;

		position_last = position;
		position = position_next;
		acceleration = sf::Vector2<double>(0.0f, gravity);
	}

};

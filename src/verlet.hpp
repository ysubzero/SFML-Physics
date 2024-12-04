#pragma once
#include <math.hpp>
#include <collisiongrid.hpp>
#include <configuration.hpp>
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
	sf::Vector2<double> center;
	double Energy;
	float radius;
	sf::Color color;
	int GridX;
	int GridY;

	VerletBall()
		: position((conf::constraints.x) / 2, 0.0f),
		position_last((conf::constraints.x) / 2, 0.0f),
		displacement(position - position_last),
		acceleration(0, 0),
		center(position.x + radius, position.y + radius),
		radius(10.0f),
		Energy(0.0),
		color(sf::Color::White),
		GridX(0),
		GridY(0)
	{}

	void updateGrid()
	{
		GridX = position.x / (radius * 3);
		GridY = position.y / (radius * 3);
	}

	void update(double dt, double gravity)
	{
		displacement = position - position_last;
		position_next = position + displacement + acceleration * dt * dt;

		position_last = position;
		position = position_next;
		center = sf::Vector2<double>(position.x + radius, position.y + radius);

		acceleration = sf::Vector2<double>(0.0f, gravity);
		updateGrid();
	}

};

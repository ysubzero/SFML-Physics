#pragma once
#include <SFML/Graphics.hpp>
#include <Headers/verlet.hpp>
#include <Headers/collisiongrid.hpp>
#include <Headers/thread_pool.hpp>
#include <random>
#include <algorithm>

class Solver
{
public:
	int count;
	const int substep;
	const int rowsize;
	const float radius;
	const double restitution;
	const double startingvel;
	const double mod;
	double collisionrestitution;
	sf::Vector2f const constraints;

	double Total_energy = 0;
	std::vector<VerletBall> balls;

	std::random_device rd;
	std::mt19937 gen;
	std::uniform_int_distribution<> dis;
	std::uniform_int_distribution<> clr;

	tp::ThreadPool& thread_pool;
	CollisionGrid grid;
	sf::VertexArray& vertices;

	Solver(
		tp::ThreadPool& tp,
		sf::VertexArray& _vertices,
		const int _count = 200000,
		const int _substep = 1,
		const int _rowsize = 600,
		const float _radius = 1.0f,
		const double _restitution = 1.0,
		const double _startingvel = 300000.0,
		const double _mod = 2.5,
		sf::Vector2f const _constraints = sf::Vector2f(1920, 1080)
	)
		:
		clr(50, 255),
		constraints(_constraints),
		grid((constraints.x / (2 * radius)) + 2, (constraints.y / (2 * radius)) + 2),
		thread_pool(tp),
		vertices(_vertices),
		count(_count),
		substep(_substep),
		rowsize(_rowsize),
		radius(_radius),
		restitution(_restitution),
		startingvel(_startingvel),
		gen(rd()),
		dis(-startingvel, startingvel),
		mod(_mod),
		collisionrestitution(1.0 - ((1 - restitution) / 2.0))
	{
		grid.InitializeNeighbors();
		balls.resize(count);
		for (int i = 0; i < count; i++) {
			balls[i].radius = radius;
			balls[i].position = sf::Vector2<double>(radius * mod * (i % rowsize) + 100, radius * mod * (i / rowsize) + 100);
			balls[i].position_last = balls[i].position;
			balls[i].acceleration = sf::Vector2<double>(dis(gen), dis(gen));
			balls[i].color = sf::Color(clr(gen), clr(gen), clr(gen));
		}
	}

	//balls[0].color = sf::Color::Yellow;
	void AddBall(const sf::Vector2f& Coords)
	{
		VerletBall newBall;
		newBall.position = sf::Vector2<double>(Coords.x, Coords.y);
		newBall.position_last = sf::Vector2<double>(Coords.x, Coords.y);
		newBall.radius = radius;
		newBall.color = sf::Color(clr(gen), clr(gen), clr(gen));
		balls.push_back(newBall);
		count++;
	}

	void updateObjects_multi(const double dt)
	{
		thread_pool.dispatch(static_cast<uint32_t>(grid.cells.size()), [&](uint32_t start, uint32_t end) {
			for (uint32_t i{ start }; i < end; ++i) {
				grid.cells[i].clear();
			}
			});
		thread_pool.dispatch(static_cast<uint32_t>(balls.size()), [&](uint32_t start, uint32_t end) {
			for (uint32_t i{ start }; i < end; ++i) {
				VerletBall& ball = balls[i];
				ball.update(dt, Math::gravity);
				addGrid(i);
			}
			});
		thread_pool.dispatch(static_cast<uint32_t>(grid.cells.size()), [&](uint32_t start, uint32_t end) {
			for (uint32_t i{ start }; i < end; ++i) {
				solveCollision(i);
			}
			});
		thread_pool.dispatch(static_cast<uint32_t>(balls.size()), [&](uint32_t start, uint32_t end) {
			for (uint32_t i{ start }; i < end; ++i) {
				VerletBall& ball = balls[i];
				constrain(ball);
				misc(ball);
			}
			});
	}

	void update(const double dt)
	{
		for (int t = 0; t < substep; t++)
		{
			updateObjects_multi(dt / substep);
		}
		toVertexArraymulti();
		for (int i = 0; i < count; i++)
		{
			VerletBall& ball = balls[i];
			Total_energy += ball.Energy;
		}
	}

	void misc(VerletBall& ball)
	{
		//float energymodifier = 5;

		double velocity = Math::magnitude_squared(ball.displacement);
		ball.Energy = ((velocity) / ((2.0))) * substep * substep;

		//int scaledEnergy = static_cast<int>((ball.Energy / (energymodifier)) * 255);
		//scaledEnergy = std::min(255, std::max(0, scaledEnergy));

		//int red = scaledEnergy;
		//int greenquadratic = ((scaledEnergy - 128) * (scaledEnergy - 128) * -0.01556) + 255;
		//int green = std::min(255, std::max(0, greenquadratic));
		//int blue = std::max(0, 255 - scaledEnergy * 2);

		//ball.color = (sf::Color(red, green, blue));
	}

	void toVertexArraymulti()
	{
		vertices.resize(balls.size() * 4);
		const float textsize = 100;
		thread_pool.dispatch(static_cast<uint32_t>(balls.size()), [&](uint32_t start, uint32_t end) {
			for (uint32_t i{ start }; i < end; ++i)
			{
				const VerletBall& ball = balls[i];
				const uint32_t index = i << 2;
				vertices[index + 0].position = sf::Vector2f{ static_cast<float>(-radius + ball.position.x), static_cast<float>(-radius + ball.position.y) };
				vertices[index + 1].position = sf::Vector2f{ static_cast<float>(radius + ball.position.x), static_cast<float>(-radius + ball.position.y) };
				vertices[index + 2].position = sf::Vector2f{ static_cast<float>(radius + ball.position.x), static_cast<float>(radius + ball.position.y) };
				vertices[index + 3].position = sf::Vector2f{ static_cast<float>(-radius + ball.position.x), static_cast<float>(radius + ball.position.y) };
				vertices[index + 0].texCoords = { 0.0f        , 0.0f };
				vertices[index + 1].texCoords = { textsize, 0.0f };
				vertices[index + 2].texCoords = { textsize, textsize };
				vertices[index + 3].texCoords = { 0.0f, textsize };

				const sf::Color color = ball.color;
				vertices[index + 0].color = color;
				vertices[index + 1].color = color;
				vertices[index + 2].color = color;
				vertices[index + 3].color = color;
			}
			});
	}

private:
	void addGrid(const uint32_t i)
	{
		VerletBall& me = balls[i];
		int GridX = me.position.x / (2 * radius) + 1;
		int GridY = me.position.y / (2 * radius) + 1;
		int index = GridY * grid.columns + GridX;
		index = std::clamp(index, 0, (grid.columns * grid.rows)-1);
		grid.cells[index].addBall(i);
	}

	void solveCollision(const uint32_t i)
	{
		if (grid.cells[i].ball_count == 0)
		{
			return;
		}
		ProcessCell(grid.cells[i]);
	}

	void ProcessCell(const CollisionCell& c)
	{
		for (int i = 0; i < c.ball_count; i++)
		{
			const int index = c.ballIndexes[i];
			for (int j = 0; j < c.maxNeighbors; j++)
			{
				checkCells(index, c.neighbors[j]);
			}
		}
	}

	void checkCells(const int index, const int cellNeighborIndex)
	{
		CollisionCell& cell = grid.cells[cellNeighborIndex];
		for (int i = 0; i < cell.ball_count; i++)
		{
			collision(index, cell.ballIndexes[i]);
		}
	}

	void collision(const int index, const int jndex)
	{
		if (index == jndex)
		{
			return;
		}
		VerletBall& ball = balls[index];
		VerletBall& OtherBall = balls[jndex];
		float bothrad = ball.radius + OtherBall.radius;
		if (std::abs(ball.position.x - OtherBall.position.x) > (bothrad) ||
			std::abs(ball.position.y - OtherBall.position.y) > (bothrad))
		{
			return;
		}
		const sf::Vector2<double> distVect = ball.position - OtherBall.position;
		const double distsquared = Math::magnitude_squared(distVect);
		const double distance = std::sqrt(distsquared);
		const double nx = distVect.x / distance;
		const double ny = distVect.y / distance;
		const double massfactorOtherBall = (2 * ball.Mass) / (ball.Mass + OtherBall.Mass);
		const double massfactorBall = (2 * OtherBall.Mass) / (ball.Mass + OtherBall.Mass);


		if ((distance) < (bothrad))
		{
			ball.position.x += nx * (bothrad - (distance)) * 0.5;
			OtherBall.position.x -= nx * (bothrad - (distance)) * 0.5;

			ball.position.y += ny * (bothrad - (distance)) * 0.5;
			OtherBall.position.y -= ny * (bothrad - (distance)) * 0.5;

			const sf::Vector2<double> ivel = ball.displacement;
			const sf::Vector2<double> jvel = OtherBall.displacement;
			const float dotProduct = Math::dot((ivel - jvel), distVect);
			ball.displacement = ivel - distVect * (Math::dot((ivel - jvel), distVect) / (distsquared)) * collisionrestitution;
			OtherBall.displacement = jvel - distVect * (Math::dot((jvel - ivel), distVect) / (distsquared)) * collisionrestitution;
			OtherBall.position_last = OtherBall.position - (massfactorOtherBall * OtherBall.displacement);
			ball.position_last = ball.position - (massfactorBall * ball.displacement);
		}
	}

	void constrain(VerletBall& ball)
	{
		if (ball.position.x > conf::constraints.x - ball.radius || ball.position.x < ball.radius)
		{
			ball.position.x = std::clamp(ball.position.x, static_cast<double>(ball.radius), static_cast<double>(conf::constraints.x - ball.radius));
			ball.position_last.x = ball.position.x + (ball.displacement.x * restitution);
		}

		if (ball.position.y > conf::constraints.y - ball.radius || ball.position.y < ball.radius)
		{
			ball.position.y = std::clamp(ball.position.y, static_cast<double>(ball.radius), static_cast<double>(conf::constraints.y - ball.radius));
			ball.position_last.y = ball.position.y + (ball.displacement.y * restitution);
		}
	}
};
#include <verlet.hpp>
#include <collisiongrid.hpp>
#include "thread_pool.hpp"
#include <random>
#include <algorithm>

class Solver
{
private:
	static constexpr int rowsize = 400;
	static constexpr float radius = 1;
	static constexpr double restitution = 1;
	static constexpr double startingvel = 100000.0f;
	static constexpr int mod = 3;

	void addGrid(int i)
	{
		VerletBall& me = ball[i];
		int GridX = me.position.x / (2 * radius);
		int GridY = me.position.y / (2 * radius);
		int index = GridY * grid.columns + GridX;
		index = std::clamp(index, 0, (grid.columns * grid.rows)-1);
		grid.cells[index].addBall(i);
	}

	void solveCollision()
	{
		for (int i = 0; i < grid.cells.size(); i++)
		{
			ProcessCell(grid.cells[i]);
		}
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

	void checkCells(int index, int cellNeighborIndex)
	{
		CollisionCell& cell = grid.cells[cellNeighborIndex];
		for (int i = 0; i < cell.ball_count; i++)
		{
			collision(index, cell.ballIndexes[i]);
		}
	}

	void collision(int index, int jndex)
	{
		if (index == jndex)
		{
			return;
		}
		VerletBall& currentball = ball[index];
		VerletBall& OtherBall = ball[jndex];
		float bothrad = currentball.radius + OtherBall.radius;
		if (std::abs(currentball.position.x - OtherBall.position.x) > (bothrad) ||
			std::abs(currentball.position.y - OtherBall.position.y) > (bothrad))
		{
			return;
		}
		const sf::Vector2<double> distVect = currentball.position - OtherBall.position;
		const double distsquared = distVect.x * distVect.x + distVect.y * distVect.y;
		const double distance = std::sqrt(distsquared);
		const double nx = distVect.x / distance;
		const double ny = distVect.y / distance;

		if ((distance) < (bothrad))
		{
			currentball.position.x += nx * (bothrad - (distance)) * 0.5;
			OtherBall.position.x -= nx * (bothrad - (distance)) * 0.5;

			currentball.position.y += ny * (bothrad - (distance)) * 0.5;
			OtherBall.position.y -= ny * (bothrad - (distance)) * 0.5;

			const sf::Vector2<double> ivel = currentball.displacement;
			const sf::Vector2<double> jvel = OtherBall.displacement;
			const float dotProduct = Math::dot((ivel - jvel), distVect);
			currentball.displacement = ivel - distVect * (Math::dot((ivel - jvel), distVect) / (distsquared)) * restitution;
			OtherBall.displacement = jvel - distVect * (Math::dot((jvel - ivel), distVect) / (distsquared)) * restitution;
			OtherBall.position_last = OtherBall.position - OtherBall.displacement;
			currentball.position_last = currentball.position - currentball.displacement;
		}
	}

	void constrain(VerletBall& currentball)
	{
		if (currentball.position.x > conf::constraints.x - currentball.radius || currentball.position.x < currentball.radius)
		{
			currentball.position.x = std::clamp(currentball.position.x, static_cast<double>(currentball.radius), static_cast<double>(conf::constraints.x - currentball.radius));
			currentball.position_last.x = currentball.position.x + (currentball.displacement.x * restitution);
		}

		if (currentball.position.y > conf::constraints.y - currentball.radius || currentball.position.y < currentball.radius)
		{
			currentball.position.y = std::clamp(currentball.position.y, static_cast<double>(currentball.radius), static_cast<double>(conf::constraints.y - currentball.radius));
			currentball.position_last.y = currentball.position.y + (currentball.displacement.y * restitution);
		}
	}

public:
	int count = 100000;
	static constexpr int substep = 1;
	double energy = 0;
	std::vector<VerletBall> ball;

	std::random_device rd;
	std::mt19937 gen;
	std::uniform_int_distribution<> dis;
	std::uniform_int_distribution<> clr;

	tp::ThreadPool& thread_pool;
	CollisionGrid grid;

	Solver(tp::ThreadPool& tp)
		: gen(rd()),
		dis(-startingvel, startingvel),
		clr(50, 255),
		grid(conf::constraints.x / (2*radius), conf::constraints.y/ (2*radius)),
		thread_pool{tp}
	{
		grid.InitializeNeighbors();
		ball.resize(count);
		for (int i = 0; i < count; i++) {
			ball[i].radius = radius;
			ball[i].position = sf::Vector2<double>(radius * mod * (i % rowsize) + 100, radius * mod * (i / rowsize) + 300);
			ball[i].position_last = ball[i].position;
			ball[i].acceleration = sf::Vector2<double>(dis(gen), dis(gen));
			//ball[i].color = sf::Color(clr(gen), clr(gen), clr(gen));
		}
	}

	//ball[0].color = sf::Color::Yellow;
	void AddBall(sf::Vector2f& Coords)
	{
		VerletBall newBall;
		newBall.position = sf::Vector2<double>(Coords.x, Coords.y);
		newBall.position_last = sf::Vector2<double>(Coords.x, Coords.y);
		newBall.radius = radius;
		//newBall.color = sf::Color(clr(gen), clr(gen), clr(gen));
		ball.push_back(newBall);
		count++;
	}

	void updateObjects_multi(float dt)
	{
		grid.clear();
		thread_pool.dispatch(static_cast<uint32_t>(ball.size()), [&](uint32_t start, uint32_t end) {
			for (uint32_t i{ start }; i < end; ++i) {
				VerletBall& obj = ball[i];
				obj.update(dt, Math::gravity);
				addGrid(i);
			}
			});
		solveCollision();
		thread_pool.dispatch(static_cast<uint32_t>(ball.size()), [&](uint32_t start, uint32_t end) {
			for (uint32_t i{ start }; i < end; ++i) {
				VerletBall& obj = ball[i];
				constrain(obj);
			}
			});
	}

	void update(float dt, sf::VertexArray& vertices)
	{
		for (int t = 0; t < substep; t++)
		{
			updateObjects_multi(dt / substep);
		}
		for (int i = 0; i < count; i++)
		{
			VerletBall& obj = ball[i];
			misc(obj, vertices);
		}
	}

	void misc(VerletBall& currentball, sf::VertexArray& vertices)
	{
		float energymodifier = 5;

		double velocity = (((currentball.displacement.x * currentball.displacement.x) + (currentball.displacement.y * currentball.displacement.y)));
		currentball.Energy = ((velocity) / ((2.0))) * substep * substep;

		int scaledEnergy = static_cast<int>((currentball.Energy / (energymodifier)) * 255);
		scaledEnergy = std::min(255, std::max(0, scaledEnergy));

		int red = scaledEnergy;
		int greenquadratic = ((scaledEnergy - 128) * (scaledEnergy - 128) * -0.01556) + 255;
		int green = std::min(255, std::max(0, greenquadratic));
		int blue = std::max(0, 255 - scaledEnergy * 2);

		currentball.color = (sf::Color(red, green, blue));

		energy += currentball.Energy;

		toVertexArray(vertices, currentball);
	}

	void toVertexArray(sf::VertexArray& vertices, const VerletBall& ball)
	{
		uint32_t textsize = 100;
		sf::Vector2u textureSize = { textsize, textsize };
		float positionx = ball.position.x;
		float positiony = ball.position.y;
		float radius = ball.radius;
		sf::Vector2f TopLeft(positionx - radius, positiony - radius);
		sf::Vector2f TopRight(positionx + radius, positiony - radius);
		sf::Vector2f BottomLeft(positionx - radius, positiony + radius);
		sf::Vector2f BottomRight(positionx + radius, positiony + radius);

		sf::Vector2f texTopLeft(0, 0);
		sf::Vector2f texTopRight(textureSize.x, 0);
		sf::Vector2f texBottomRight(textureSize.x, textureSize.y);
		sf::Vector2f texBottomLeft(0, textureSize.y);

		vertices.append(sf::Vertex(TopLeft, ball.color, texTopLeft));
		vertices.append(sf::Vertex(TopRight, ball.color, texTopRight));
		vertices.append(sf::Vertex(BottomRight, ball.color, texBottomRight));
		vertices.append(sf::Vertex(BottomLeft, ball.color, texBottomLeft));
	}
};
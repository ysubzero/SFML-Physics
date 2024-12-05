#include <verlet.hpp>
//#include <collisiongrid.hpp>
#include "thread_pool.hpp"
#include <random>

class Solver
{
private:
	static constexpr int rowsize = 180;
	static constexpr float radius = 2;
	static constexpr double restitution = 1;
	static constexpr double startingvel = 1000.0f;
	static constexpr int mod = 3;

	void collisionwitheachother(int i, VerletBall& currentball, float dt)
	{
		for (int j = i + 1; j < count; j++)
		{
			VerletBall& OtherBall = ball[j];
			if (abs(currentball.GridX - OtherBall.GridX) <= 1 &&
				abs(currentball.GridY - OtherBall.GridY) <= 1)
			{
				float bothrad = currentball.radius + OtherBall.radius;
				if (std::abs(currentball.position.x - OtherBall.position.x) > (bothrad) ||
					std::abs(currentball.position.y - OtherBall.position.y) > (bothrad))
				{
					continue;
				}
				const sf::Vector2<double> distVect = currentball.center - OtherBall.center;
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
					currentball.updateGrid();
					OtherBall.updateGrid();
				}
			}
		}

		if (currentball.position.x > conf::constraints.x - 2 * currentball.radius || currentball.position.x < 0)
		{
			if (currentball.position.x > conf::constraints.x - 2 * currentball.radius)
			{
				currentball.position.x = conf::constraints.x - 2 * currentball.radius;
			}
			else
			{
				currentball.position.x = 0;
			}
			currentball.position_last.x = currentball.position.x + (currentball.displacement.x * restitution);
		}

		if (currentball.position.y > conf::constraints.y - 2 * currentball.radius || currentball.position.y < 0)
		{
			if (currentball.position.y > conf::constraints.y - 2 * currentball.radius)
			{
				currentball.position.y = conf::constraints.y - 2 * currentball.radius;
				currentball.acceleration = sf::Vector2<double>(0.0f, 0.0f);
			}
			else
			{
				currentball.position.y = 0;
			}
			currentball.position_last.y = currentball.position.y + (currentball.displacement.y * restitution);
		}
	}
public:
	int count = 10000;
	static constexpr int substep = 1;
	double energy = 0;
	std::vector<VerletBall> ball;

	std::random_device rd;
	std::mt19937 gen;
	std::uniform_int_distribution<> dis;
	std::uniform_int_distribution<> clr;

	tp::ThreadPool& thread_pool;
	sf::VertexArray& vertices;

	Solver(tp::ThreadPool& tp, sf::VertexArray& _vertices)
		: gen(rd()),
		dis(-startingvel, startingvel),
		clr(50, 255)
		, thread_pool{tp},
		vertices(_vertices)
	{
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
		thread_pool.dispatch(static_cast<uint32_t>(ball.size()), [&](uint32_t start, uint32_t end) {
			for (uint32_t i{ start }; i < end; ++i) {
				VerletBall& obj = ball[i];
				obj.update(dt, Math::gravity);
				collisionwitheachother(i, obj, dt);
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
		toVertexArraymulti();
	}

	void misc(VerletBall& currentball, sf::VertexArray& vertices)
	{
		float energymodifier = 6 / substep;

		double velocity = (((currentball.displacement.x * currentball.displacement.x) + (currentball.displacement.y * currentball.displacement.y)));
		currentball.Energy = ((velocity) / ((2.0)));

		int scaledEnergy = static_cast<int>((currentball.Energy / (energymodifier)) * 255);
		scaledEnergy = std::min(255, std::max(0, scaledEnergy));

		int red = scaledEnergy;
		int greenquadratic = ((scaledEnergy - 128) * (scaledEnergy - 128) * -0.01556) + 255;
		int green = std::min(255, std::max(0, greenquadratic));
		int blue = std::max(0, 255 - scaledEnergy * 2);

		currentball.color = (sf::Color(red, green, blue));

		energy += currentball.Energy;
	}

	void toVertexArraymulti()
	{
		vertices.resize(ball.size() * 4);
		const float textsize = 100;
		thread_pool.dispatch(static_cast<uint32_t>(ball.size()), [&](uint32_t start, uint32_t end) {
			for (uint32_t i{ start }; i < end; ++i)
			{
				const VerletBall& balle = ball[i];
				const uint32_t index = i << 2;
				vertices[index + 0].position = sf::Vector2f{ static_cast<float>(-radius + balle.position.x), static_cast<float>(-radius + balle.position.y) };
				vertices[index + 1].position = sf::Vector2f{ static_cast<float>(radius + balle.position.x), static_cast<float>(-radius + balle.position.y) };
				vertices[index + 2].position = sf::Vector2f{ static_cast<float>(radius + balle.position.x), static_cast<float>(radius + balle.position.y) };
				vertices[index + 3].position = sf::Vector2f{ static_cast<float>(-radius + balle.position.x), static_cast<float>(radius + balle.position.y) };
				vertices[index + 0].texCoords = { 0.0f        , 0.0f };
				vertices[index + 1].texCoords = { textsize, 0.0f };
				vertices[index + 2].texCoords = { textsize, textsize };
				vertices[index + 3].texCoords = { 0.0f, textsize };

				const sf::Color color = balle.color;
				vertices[index + 0].color = color;
				vertices[index + 1].color = color;
				vertices[index + 2].color = color;
				vertices[index + 3].color = color;
			}
			});
	}
};
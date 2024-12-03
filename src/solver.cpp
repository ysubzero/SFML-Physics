#include <verlet.hpp>
//#include <collisiongrid.hpp>
#include <random>

class Solver
{
private:
	static constexpr int rowsize = 90;
	static constexpr float radius = 3;
	static constexpr double restitution = 1;
	static constexpr double startingvel = 1000.0f;
	static constexpr int mod = 4;

	void collisionwitheachother(int i, VerletBall& currentball, float dt)
	{
		for (int j = i+1; j < count; j++)
		{
			VerletBall& OtherBall = ball[j];
			if (std::abs(currentball.position.x - OtherBall.position.x) > (currentball.radius + OtherBall.radius) ||
				std::abs(currentball.position.y - OtherBall.position.y) > (currentball.radius + OtherBall.radius))
			{
				continue;
			}
			const sf::Vector2<double> distVect = currentball.center - OtherBall.center;
			const float distance = std::sqrt(distVect.x * distVect.x + distVect.y * distVect.y);
			const float nx = distVect.x / distance;
			const float ny = distVect.y / distance;

			if ((distance) < (currentball.radius + OtherBall.radius))
			{
				currentball.position.x += nx * (currentball.radius + OtherBall.radius - (distance)) * 0.5;
				OtherBall.position.x -= nx * (currentball.radius + OtherBall.radius - (distance)) * 0.5;

				currentball.position.y += ny * (currentball.radius + OtherBall.radius - (distance)) * 0.5;
				OtherBall.position.y -= ny * (currentball.radius + OtherBall.radius - (distance)) * 0.5;

				const sf::Vector2<double> ivel = currentball.displacement;
				const sf::Vector2<double> jvel = OtherBall.displacement;
				const float dotProduct = Math::dot((ivel - jvel), distVect);
				currentball.displacement = ivel - distVect * (Math::dot((ivel - jvel), distVect) / (distVect.x * distVect.x + distVect.y * distVect.y)) * restitution;
				OtherBall.displacement = jvel - distVect * (Math::dot((jvel - ivel), distVect) / (distVect.x * distVect.x + distVect.y * distVect.y)) * restitution;
				OtherBall.position_last = OtherBall.position - OtherBall.displacement;
				currentball.position_last = currentball.position - currentball.displacement;
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
	int count = 5000;
	static constexpr int substep = 1;
	double energy = 0;
	std::vector<VerletBall> ball;

	std::random_device rd;
	std::mt19937 gen;
	std::uniform_int_distribution<> dis;
	std::uniform_int_distribution<> clr;

	Solver()
		: gen(rd()),
		dis(-startingvel, startingvel),
		clr(50, 255)
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
	
	void update(float dt, sf::VertexArray& vertices)
	{
		for (int t = 0; t < substep; t++)
		{
			for (int i = 0; i < count; i++)
			{
				VerletBall& currentball = ball[i];
				currentball.update(dt / substep, Math::gravity);
			}
			for (int i = 0; i < count; i++)
			{
				VerletBall& currentball = ball[i];
				collisionwitheachother(i, currentball, dt / substep);
			}
		}
		#pragma omp parallel for
		for (int i = 0; i < count; i++)
		{
			float energymodifier = 25 / substep;

			VerletBall& currentball = ball[i];

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

			toVertexArray(vertices, currentball);
		}
	}

	void toVertexArray(sf::VertexArray& vertices, const VerletBall& ball)
	{
		sf::Vector2u textureSize = { 100, 100 };
		float positionx = ball.position.x;
		float positiony = ball.position.y;
		float radius = ball.radius + ball.radius;
		sf::Vector2f TopLeft(positionx, positiony);
		sf::Vector2f TopRight(positionx + radius, positiony);
		sf::Vector2f BottomLeft(positionx, positiony + radius);
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
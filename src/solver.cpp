//Portions of this code are licensed and copyright as follows :
/*MIT License

Copyright(c) 2021 Jean Tampon

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

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
	const bool ThermalColors;
	std::vector<VerletBall> balls;

	Solver(
		tp::ThreadPool& tp,
		sf::VertexArray& _vertices,
		const int _count = 200000,
		const int _substep = 1,
		const int _rowsize = 600,
		const float _radius = 1.0f,
		const double _restitution = 1.0,
		const double _startingvel = 5.0,
		const double _mod = 2.5,
		sf::Vector2f const _constraints = sf::Vector2f(1920, 1080),
		const bool _ThermalColors = true
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
		collisionrestitution(1.0 - ((1 - restitution) / 2.0)),
		ThermalColors(_ThermalColors)
	{
		grid.InitializeNeighbors();
		balls.resize(count);
		thread_pool.dispatch(static_cast<uint32_t>(balls.size()), [&](uint32_t start, uint32_t end) {
			for (uint32_t i{ start }; i < end; ++i) {
				VerletBall& ball = balls[i];
				ball.radius = radius;
				ball.position = sf::Vector2<double>(radius * mod * (i % rowsize) + std::min((6 * radius + 1.0), 100.0), radius * mod * (i / rowsize) + std::min((6 * radius + 1.0), 100.0));
				ball.position_last = sf::Vector2<double>(ball.position.x + (dis(gen) / 3.0), ball.position.y + (dis(gen) / 3.0));
				if (!ThermalColors)
				{
					ball.color = sf::Color(clr(gen), clr(gen), clr(gen));
				}
			}
			});
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

	void mouse(const sf::Vector2f pos, const double dt)
	{
		int GridX = pos.x / (2 * radius) + 1;
		if (GridX < 0 || GridX >= grid.columns)
		{
			return;
		}
		int GridY = pos.y / (2 * radius) + 1;
		if (GridY < 0 || GridY >= grid.rows)
		{
			return;
		}
		int index = GridY * grid.columns + GridX;
		if (index < 0 || index > (grid.rows * grid.columns) - 1)
		{
			return;
		}
		for (int j = 0; j < grid.cells[index].maxNeighbors; ++j)
		{
			for (int k = 0; k < grid.cells[grid.cells[index].neighbors[j]].ball_count; ++k)
			{
				VerletBall& ball = balls[grid.cells[grid.cells[index].neighbors[j]].ballIndexes[k]];
				const sf::Vector2<double> direction = sf::Vector2<double>(pos.x, pos.y) - ball.position;
				const double dist = Math::magnitude(direction);
				ball.position_last -= sf::Vector2<double>(direction.x * -(0.12 - dist/radius), direction.y * -(0.12 - dist/radius));
			}
		}
	}

	void update(const double dt, const double gravity)
	{
		Total_energy = 0;
		for (int t = 0; t < substep; t++)
		{
			updateObjects_multi(dt / substep, gravity);
		}
		toVertexArraymulti();
		for (int i = 0; i < count; i++)
		{
			VerletBall& ball = balls[i];
			Total_energy += ball.Energy;
		}
	}


private:

	std::random_device rd;
	std::mt19937 gen;
	std::uniform_real_distribution<> dis;
	std::uniform_int_distribution<> clr;

	tp::ThreadPool& thread_pool;
	CollisionGrid grid;
	sf::VertexArray& vertices;


	void addGrid(const VerletBall& me, const int i)
	{
		int GridX = me.position.x / (2 * radius) + 1;
		int GridY = me.position.y / (2 * radius) + 1;
		int index = GridY * grid.columns + GridX;
		index = std::clamp(index, 0, (grid.columns * grid.rows)-1);
		grid.cells[index].addBall(i);
	}

	void solveCollision()
	{
		const uint32_t thread_count = thread_pool.m_thread_count;
		const uint32_t slice_count = thread_count * 2;
		const uint32_t slice_size = (grid.columns / slice_count) * grid.rows;
		const uint32_t last_cell = (2 * (thread_count - 1) + 2) * slice_size;

		for (uint32_t i{ 0 }; i < thread_count; ++i) {
			thread_pool.addTask([this, i, slice_size] 
			{
				uint32_t const start{ 2 * i * slice_size };
				uint32_t const end{ start + slice_size };
				for (uint32_t i{ start }; i < end; ++i) {
					ProcessCell(grid.cells[i]);
				}
			});
		}
		if (last_cell < grid.cells.size()) {
			thread_pool.addTask([this, last_cell] 
			{
				uint32_t const end = static_cast<uint32_t>(grid.cells.size());
				for (uint32_t i{ last_cell }; i < end; ++i) 
				{
					ProcessCell(grid.cells[i]);
				}
			});
		}
		thread_pool.waitForCompletion();
		for (uint32_t i{ 0 }; i < thread_count; ++i) {
			thread_pool.addTask([this, i, slice_size] 
			{
				uint32_t const start{ (2 * i + 1) * slice_size };
				uint32_t const end{ start + slice_size };
				for (uint32_t i{ start }; i < end; ++i) 
				{
					ProcessCell(grid.cells[i]);
				}
			});
		}
		thread_pool.waitForCompletion();
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
		const float bothrad = ball.radius + OtherBall.radius;

		if (std::abs(ball.position.x - OtherBall.position.x) > (bothrad) ||
			std::abs(ball.position.y - OtherBall.position.y) > (bothrad))
		{
			return;
		}
		const sf::Vector2<double> distVect = ball.position - OtherBall.position;
		const double distsquared = Math::magnitude_squared(distVect);
		const double distance = std::sqrt(distsquared);

		if ((distance) < (bothrad))
		{
			const double nx = distVect.x / distance;
			const double ny = distVect.y / distance;
			const double massfactorOtherBall = (2 * ball.Mass) / (ball.Mass + OtherBall.Mass);
			const double massfactorBall = (2 * OtherBall.Mass) / (ball.Mass + OtherBall.Mass);

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
		if (ball.position.x > constraints.x - ball.radius || ball.position.x < ball.radius)
		{
			ball.position.x = std::clamp(ball.position.x, static_cast<double>(ball.radius), static_cast<double>(constraints.x - ball.radius));
			ball.position_last.x = ball.position.x + (ball.displacement.x * restitution);
		}

		if (ball.position.y > constraints.y - ball.radius || ball.position.y < ball.radius)
		{
			ball.position.y = std::clamp(ball.position.y, static_cast<double>(ball.radius), static_cast<double>(constraints.y - ball.radius));
			ball.position_last.y = ball.position.y + (ball.displacement.y * restitution);
		}
	}

	void misc(VerletBall& ball)
	{
		double velocity = Math::magnitude_squared(ball.displacement);
		ball.Energy = ((velocity) / ((2.0))) * substep * substep;

		if (ThermalColors)
		{
			const float energymodifier = 5;
			int scaledEnergy = static_cast<int>((ball.Energy / (energymodifier)) * 255);
			scaledEnergy = std::min(255, std::max(0, scaledEnergy));

			int red = scaledEnergy;
			int greenquadratic = ((scaledEnergy - 128) * (scaledEnergy - 128) * -0.01556) + 255;
			int green = std::min(255, std::max(0, greenquadratic));
			int blue = std::max(0, 255 - scaledEnergy * 2);

			ball.color = (sf::Color(red, green, blue));
		}
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

	void updateObjects_multi(const double dt, const double gravity)
	{
		thread_pool.dispatch(static_cast<uint32_t>(grid.cells.size()), [&](uint32_t start, uint32_t end) {
			for (uint32_t i{ start }; i < end; ++i) {
				grid.cells[i].ball_count = 0;
			}
			});

		thread_pool.dispatch(static_cast<uint32_t>(balls.size()), [&](uint32_t start, uint32_t end) {
			for (uint32_t i{ start }; i < end; ++i) {
				VerletBall& ball = balls[i];
				ball.update(dt, gravity);
				addGrid(ball, i);
			}
			});

		solveCollision();

		thread_pool.dispatch(static_cast<uint32_t>(balls.size()), [&](uint32_t start, uint32_t end) {
			for (uint32_t i{ start }; i < end; ++i) {
				VerletBall& ball = balls[i];
				constrain(ball);
				misc(ball);
			}
			});
	}
};
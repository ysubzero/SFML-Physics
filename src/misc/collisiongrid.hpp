#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <verlet.hpp>
#include <configuration.hpp>

struct CollisionCell
{
	sf::Vector2<double> TopLeft;
	sf::Vector2<double> BottomRight;
	std::vector<VerletBall*> Balls;

	CollisionCell()
		: TopLeft(0.0,0.0),
		BottomRight(0.0,0.0)
	{}

    bool IsInGrid(const VerletBall &ball)
    {
        double x = ball.position.x;
        double y = ball.position.y;
        float diameter = ball.radius + ball.radius;

        return !(x + diameter < TopLeft.x || y + diameter < TopLeft.y || x > BottomRight.x || y > BottomRight.y);
    }

    void AddToGrid(VerletBall& ball)
    {
        if (IsInGrid(ball))
        {
            Balls.push_back(&ball);
        }
    }

    void Clear()
    {
        Balls.clear();
    }
};

struct CollisionGrid
{
	static constexpr double x_size = 10;
	static constexpr double y_size = 10;

    int row_length;
    int col_length;
    int CellCount;

    CollisionCell* cells;

    CollisionGrid()
        : row_length(static_cast<int>(conf::constraints.x / x_size)),
        col_length(static_cast<int>(conf::constraints.y / y_size)),
        CellCount(row_length* col_length)
    {
        cells = new CollisionCell[CellCount];

        for (int i = 0; i < CellCount; ++i)
        {
            int row = i / row_length;
            int col = i % row_length;
            cells[i].TopLeft = { col * x_size, row * y_size };
            cells[i].BottomRight = { (col + 1) * x_size, (row + 1) * y_size };
        }
    }

    void GridUpdate(VerletBall &balls)
    {
        std::vector<CollisionCell*> ballCells = GetBallCells(balls);
        if (ballCells.size() == 4)
        {
            return;
        }
        for (int g = 0; g < CellCount; g++)
        {
         cells[g].AddToGrid(balls);
        }
    }

    void GridClear()
    {
        for (int g = 0; g < CellCount; g++)
        {
            cells[g].Clear();
        }
    }

    ~CollisionGrid()
    {
        delete[] cells;
    }

    std::vector<CollisionCell*> GetBallCells(const VerletBall& ball)
    {
        std::vector<CollisionCell*> ballCells;

        int startX = static_cast<int>((ball.position.x - ball.radius) / CollisionGrid::x_size);
        int startY = static_cast<int>((ball.position.y - ball.radius) / CollisionGrid::y_size);
        int endX = static_cast<int>((ball.position.x + ball.radius) / CollisionGrid::x_size);
        int endY = static_cast<int>((ball.position.y + ball.radius) / CollisionGrid::y_size);

        for (int x = startX; x <= endX; ++x)
        {
            for (int y = startY; y <= endY; ++y)
            {
                if (x >= 0 && x < row_length && y >= 0 && y < col_length)
                {
                    int cellIndex = y * row_length + x;
                    ballCells.push_back(&cells[cellIndex]);
                }
            }
        }

        return ballCells;
    }

};

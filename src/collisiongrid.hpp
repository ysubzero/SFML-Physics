#pragma once
#include <iostream>
struct CollisionCell 
{
	static const int capacity = 4;
	static const int maxIndex = 3;
	static const int neighborCount = 9;

	int ball_count = 0;
	int maxNeighbors = 8;
	int ballIndexes[capacity] = {};
	int neighbors[neighborCount] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

	CollisionCell() = default;

	void addBall(int ball_index)
	{
		ballIndexes[ball_count] = ball_index;
		if (ball_count < maxIndex)
		{
			ball_count++;
		}
	}

	void clear()
	{
		ball_count = 0;
	}
};

struct CollisionGrid
{
	int columns, rows;
	std::vector<CollisionCell> cells;
	
	CollisionGrid()
		: columns(0)
		, rows(0)
	{}

	CollisionGrid(int _columns, int _rows)
		: columns(_columns)
		, rows(_rows)
	{
		cells.resize(columns * rows);
	}

	void InitializeNeighbors()
	{
		for (int i = 0; i < columns * rows; i++)
		{
			int neighborCount = 0;

			cells[i].neighbors[neighborCount++] = i;

			if (i % columns > 0)
			{
				cells[i].neighbors[neighborCount++] = i - 1;
			}

			if (i % columns < columns - 1)
			{
				cells[i].neighbors[neighborCount++] = i + 1;
			}

			if (i >= columns)
			{
				cells[i].neighbors[neighborCount++] = i - columns;
			}

			if (i < (columns * rows) - columns)
			{
				cells[i].neighbors[neighborCount++] = i + columns;
			}

			if (i >= columns && i % columns > 0)
			{
				cells[i].neighbors[neighborCount++] = i - columns - 1;
			}

			if (i >= columns && i % columns < columns - 1)
			{
				cells[i].neighbors[neighborCount++] = i - columns + 1;
			}

			if (i < (columns * rows) - columns && i % columns > 0)
			{
				cells[i].neighbors[neighborCount++] = i + columns - 1;
			}

			if (i < (columns * rows) - columns && i % columns < columns - 1)
			{
				cells[i].neighbors[neighborCount++] = i + columns + 1;
			}

			cells[i].maxNeighbors = neighborCount;
		}
	}

	void addBall(int x, int y, int ball)
	{
		int cellIndex = (x * rows) + y;
		cells[cellIndex].addBall(ball);
	}

	void IdxAddBall(int index, int ball)
	{
		cells[index].addBall(ball);
	}

	void clear()
	{
		for (int i = 0; i < (columns * rows); i++)
		{
			cells[i].clear();
		}
	}
};

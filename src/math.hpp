#pragma once
#include <SFML/System/Vector2.hpp>
#include <cmath>

struct Math
{
    static constexpr double PI = 3.14159;
    static constexpr double gravity = 98;

    static double dot(sf::Vector2<double> v1, sf::Vector2<double> v2)
    {
        return { v1.x * v2.x + v1.y * v2.y };
    }

    float arctan2(sf::Vector2<double> origin, sf::Vector2<double> end)
    {
        return std::atan2(origin.y - end.y, origin.x - end.x);
    }
};
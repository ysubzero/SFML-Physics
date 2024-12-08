#pragma once
#include <SFML/System/Vector2.hpp>
#include <cmath>

struct Math
{
    static constexpr double PI = 3.14159;
    static constexpr double gravity = 98;
    static constexpr double grav_const = 1.0;

    static double dot(sf::Vector2<double> v1, sf::Vector2<double> v2)
    {
        return ( v1.x * v2.x + v1.y * v2.y );
    }

    static double magnitude_squared(sf::Vector2<double> v1)
    {
        return ( v1.x * v1.x + v1.y * v1.y );
    }

    static double magnitude(sf::Vector2<double> v1)
    {
        return ( std::sqrt(magnitude_squared(v1)));
    }

    float arctan2(sf::Vector2<double> origin, sf::Vector2<double> end)
    {
        return std::atan2(origin.y - end.y, origin.x - end.x);
    }
};
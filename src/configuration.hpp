#pragma once
#include <SFML/System/Vector2.hpp>
#include <cstdint>

namespace conf
{
	sf::Vector2u const window_size = { 1920, 1080 };
	sf::Vector2f const window_sizef = static_cast<sf::Vector2f>(window_size);
	sf::Vector2f const constraints = sf::Vector2f( 1920, 1080 );
	uint32_t const max_framerate = 75;
	double const dt = 1 / static_cast<float>(max_framerate);
}
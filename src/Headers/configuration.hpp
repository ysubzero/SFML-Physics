#pragma once
#include <SFML/System/Vector2.hpp>
#include <cstdint>

namespace conf
{
	//sf::Vector2u const window_size = { 1920, 1080 }; //unused  legacy
	//sf::Vector2f const window_sizef = static_cast<sf::Vector2f>(window_size); //unused legacy
	sf::Vector2<unsigned int> const gui_size = { 1280, 960 };
	uint32_t const max_framerate = 170;
}
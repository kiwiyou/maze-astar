#include <algorithm>
#include <SFML/Graphics.hpp>
#include "Maze.h"

int main(int, char**) {
    sf::RenderWindow window(sf::VideoMode(900, 500), "Pathfinder");
    Maze maze(sf::Vector2f(500, 500));
    maze.set_position({200, 0});
    maze.generate();

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyReleased)
            {
                if (event.key.code == sf::Keyboard::F5)
                    maze.generate();
                else if (event.key.code == sf::Keyboard::Space)
                    maze.find_path();
            } 
            maze.on_event(event);
        }

        window.clear();
        maze.update();
        window.draw(maze);
        window.display();
    }

    return 0;
}

#ifndef _MAZETEST_MAZE_H_
#define _MAZETEST_MAZE_H_

#include <array>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <SFML/Graphics/Drawable.hpp>
#include "yield.h"
namespace sf
{
    class Color;
    class Event;
}

class Maze : public sf::Drawable
{
public:
    enum class Tile { EMPTY, WALL, START, END, ROAD, COLOR1, COLOR2, COLOR3 };
    const static std::size_t WIDTH = 51;
    const static std::size_t HEIGHT = 51;

public:
    Maze(const sf::Vector2f& size);

public:
    void set_position(const sf::Vector2f& position);
    void set_size(const sf::Vector2f& size);
    void generate();
    void on_event(const sf::Event& event);
    void find_path();
    void update();

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    static sf::Color color_for(const Tile& tile);

private:
    std::array<Tile, WIDTH * HEIGHT> tiles;
    
    sf::Vector2f offset;
    sf::Vector2f size;

    Tile* start;
    Tile* end;

    constexpr static float BORDER_THICKNESS = 1.f;
    constexpr static unsigned MS_PER_FRAME = 20;

    class PathfindAnimator
    {
    private:
        void* YIELD_POINTER;
        std::size_t source;
        std::size_t destination;
        bool finished = false;
        decltype(tiles)* tile_ref;

        // local variables
        using score_t = float;
        using score_map = std::unordered_map<std::size_t, score_t>;

        std::unordered_set<std::size_t> evaluated;
        std::unordered_set<std::size_t> discovered;
        std::unordered_map<std::size_t, std::size_t> efficient_previous;
        score_map g_score;
        score_map f_score;

        constexpr score_t heuristic(std::size_t from, std::size_t to);
        score_t f(std::size_t x);
        score_t g(std::size_t x);

    public:
        PathfindAnimator(std::size_t start, std::size_t end, decltype(tiles)& ref);
    
    public:
        bool end() const;
        void stop();
        void operator()();
    };
    PathfindAnimator animator;
};

#endif
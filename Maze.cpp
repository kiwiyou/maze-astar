#include <algorithm>
#include <stack>
#include <vector>
#include <random>
#include <cmath>
#include <limits>
#include <chrono>
#include <SFML/Graphics.hpp>

#include "Maze.h"

Maze::Maze(const sf::Vector2f& size)
    : offset{0, 0}, size{size}, tiles{}, animator(0, 0, tiles)
{
    start = end = nullptr;
}

void Maze::set_position(const sf::Vector2f& position)
{
    offset = position;
}

void Maze::set_size(const sf::Vector2f& size)
{
    this->size = size;
}

void Maze::generate()
{
    animator.stop();
    start = end = nullptr;
    auto available_width = std::ceil(WIDTH / 2.0);
    auto available_height = std::ceil(HEIGHT / 2.0);
    auto available_cells = available_width * available_height;
    std::random_device device;
    std::mt19937 randomizer{device()};
    for (std::size_t y = 0; y < HEIGHT; ++y)
    {
        for (std::size_t x = 0; x < WIDTH; ++x)
        {
            if (x % 2 == 0 && y % 2 == 0)
            {
                tiles[y * WIDTH + x] = Tile::EMPTY;
            }
            else
            {
                tiles[y * WIDTH + x] = Tile::WALL;
            }
            
        }
    }
    std::unordered_set<std::size_t> visited;
    tiles[0] = Tile::EMPTY;
    std::stack<std::size_t> neighbour;
    neighbour.push(0);
    std::size_t current = 0;
    visited.emplace(current);
    while (visited.size() < available_cells)
    {
        std::vector<std::size_t> candidates;
        if (current >= WIDTH * 2)
        {
            auto candidate = current - WIDTH * 2;
            if (visited.find(candidate) == visited.end())
                candidates.push_back(candidate);
        }
        if (current < WIDTH * (HEIGHT - 2))
        {
            auto candidate = current + WIDTH * 2;
            if (visited.find(candidate) == visited.end())
                candidates.push_back(candidate);
        }
        if (current % WIDTH > 1)
        {
            auto candidate = current - 2;
            if (visited.find(candidate) == visited.end())
                candidates.push_back(candidate);
        }
        if (current % WIDTH < WIDTH - 2)
        {
            auto candidate = current + 2;
            if (visited.find(candidate) == visited.end())
                candidates.push_back(candidate);
        }
        if (candidates.empty())
        {
            if (neighbour.empty())
                break;
            current = neighbour.top();
            neighbour.pop();
            continue;
        }
        std::uniform_int_distribution<std::size_t> distributor{0, candidates.size() - 1};
        auto chosen = candidates[distributor(randomizer)];
        neighbour.push(current);
        auto wall = (current + chosen) / 2;
        tiles[wall] = Tile::EMPTY;
        current = chosen;
        visited.insert(current);
    }
}

void Maze::on_event(const sf::Event& event)
{
    if (event.type == sf::Event::MouseButtonPressed)
    {
        auto mouse_event = event.mouseButton;
        sf::FloatRect region(offset.x, offset.y, size.x, size.y);
        if (region.contains(mouse_event.x, mouse_event.y))
        {
            constexpr auto horizontal_lines = WIDTH + 1;
            constexpr auto vertical_lines = HEIGHT + 1;
            auto block_width = (size.x - horizontal_lines * BORDER_THICKNESS) / WIDTH;
            auto block_height = (size.y - vertical_lines * BORDER_THICKNESS) / HEIGHT;
            auto click_x = std::min((std::size_t) std::floor((mouse_event.x - offset.x) / (BORDER_THICKNESS + block_width)), WIDTH - 1);
            auto click_y = std::min((std::size_t) std::floor((mouse_event.y - offset.y) / (BORDER_THICKNESS + block_height)), HEIGHT - 1);
            if (tiles[click_y * WIDTH + click_x] == Tile::WALL)
                return;
            if (mouse_event.button == sf::Mouse::Button::Left)
            {
                animator.stop();
                auto& target = tiles[click_y * WIDTH + click_x];
                if (target == Tile::START)
                {
                    target = Tile::EMPTY;
                    start = nullptr;
                }
                else
                {
                    if (target == Tile::END)
                        end = nullptr;
                    target = Tile::START;
                    if (start)
                        *start = Tile::EMPTY;
                    start = &target;
                }
            }
            else if (mouse_event.button == sf::Mouse::Button::Right)
            {
                animator.stop();
                auto& target = tiles[click_y * WIDTH + click_x];
                if (target == Tile::END)
                {
                    target = Tile::EMPTY;
                    end = nullptr;
                }
                else
                {
                    if (target == Tile::START)
                        start = nullptr;
                    target = Tile::END;
                    if (end)
                        *end = Tile::EMPTY;
                    end = &target;
                }
            }
        }
    }
}

void Maze::find_path()
{
    if (!start || !end)
        return;
    std::transform(tiles.begin(), tiles.end(), tiles.begin(), [](const auto& tile)
    {
        return (tile == Tile::COLOR1 || tile == Tile::ROAD) ? Tile::EMPTY : tile;
    });
    animator = PathfindAnimator(start - &tiles[0], end - &tiles[0], tiles);
}

void Maze::update()
{
    static auto prev_tick = std::chrono::system_clock::now();
    if (!animator.end())
    {
        auto elapsed = std::chrono::system_clock::now() - prev_tick;
        if (std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() >= MS_PER_FRAME)
        {
            prev_tick = std::chrono::system_clock::now();
            animator();
        }
    }
}

void Maze::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    constexpr auto horizontal_lines = WIDTH + 1;
    constexpr auto vertical_lines = HEIGHT + 1;
    auto block_width = (size.x - horizontal_lines * BORDER_THICKNESS) / WIDTH;
    auto block_height = (size.y - vertical_lines * BORDER_THICKNESS) / HEIGHT;
    for (auto yi = 0; yi < HEIGHT; ++yi)
    {
        auto y_position = yi * (BORDER_THICKNESS + block_height);
        for (auto xi = 0; xi < WIDTH; ++xi)
        {
            auto x_position = xi * (BORDER_THICKNESS + block_width);
            auto block = sf::RectangleShape(sf::Vector2f(block_width, block_height));
            block.setPosition(sf::Vector2f(offset.x + x_position + BORDER_THICKNESS, offset.y + y_position + BORDER_THICKNESS));
            block.setFillColor(color_for(tiles[yi * WIDTH + xi]));
            target.draw(block, states);
        }
    }
}

sf::Color Maze::color_for(const Tile& tile)
{
    switch (tile)
    {
        case Tile::EMPTY:
            return sf::Color::White;

        case Tile::START:
            return sf::Color::Red;
        
        case Tile::END:
            return sf::Color::Green;

        case Tile::ROAD:
            return sf::Color(0xFF, 0xFF, 0xBB);
        
        case Tile::COLOR1:
            return sf::Color(0x77, 0x77, 0x77);

        case Tile::COLOR2:
            return sf::Color(0xBB, 0xFF, 0xBB);

        case Tile::COLOR3:
            return sf::Color(0xFF, 0xBB, 0xFF);

        case Tile::WALL:
        default:
            return sf::Color::Black;
    }
}

Maze::PathfindAnimator::PathfindAnimator(std::size_t start, std::size_t end, decltype(Maze::tiles)& ref)
    : source{start}, destination{end}, tile_ref(&ref)
{
    YIELD_POINTER = nullptr;
    if (source == destination)
        finished = true;
}

bool Maze::PathfindAnimator::end() const
{
    return finished;
}

void Maze::PathfindAnimator::stop()
{
    finished = true;
}

constexpr typename Maze::PathfindAnimator::score_t Maze::PathfindAnimator::heuristic(std::size_t from, std::size_t to)
{
    long long from_y = from / Maze::WIDTH;
    long long from_x = from % Maze::WIDTH;

    long long to_y = to / Maze::WIDTH;
    long long to_x = to % Maze::WIDTH;

    return from_y - to_y + from_x - to_x;
}

typename Maze::PathfindAnimator::score_t Maze::PathfindAnimator::f(std::size_t x)
{
    if (auto it = f_score.find(x); it == f_score.end())
        return std::numeric_limits<score_t>::infinity();
    else
        return it->second;
}

typename Maze::PathfindAnimator::score_t Maze::PathfindAnimator::g(std::size_t x)
{
    if (auto it = g_score.find(x); it == g_score.end())
        return std::numeric_limits<score_t>::infinity();
    else
        return it->second;
}

void Maze::PathfindAnimator::operator()()
{
    yield_start();
    if (finished)
        return;
    discovered.insert(source);
    g_score[source] = 0;
    f_score[source] = heuristic(source, destination);
    while (!discovered.empty())
    {
        {
            auto current = *std::min_element(discovered.cbegin(), discovered.cend(), [this](std::size_t a, std::size_t b)
            {
                return f(a) < f(b);
            });
            if (current == destination)
            {
                while (efficient_previous.find(current) != efficient_previous.end())
                {
                    current = efficient_previous[current];
                    if (auto& tile = (*tile_ref)[current]; tile != Tile::START)
                        tile = Tile::ROAD;
                }
                finished = true;
                break;
            }

            discovered.erase(current);
            evaluated.insert(current);

            std::vector<std::size_t> neighbours;
            if (current >= WIDTH)
            {
                auto candidate = current - WIDTH;
                if (evaluated.find(candidate) == evaluated.end())
                    neighbours.push_back(candidate);
            }
            if (current < WIDTH * (HEIGHT - 1))
            {
                auto candidate = current + WIDTH;
                if (evaluated.find(candidate) == evaluated.end())
                    neighbours.push_back(candidate);
            }
            if (current % WIDTH > 0)
            {
                auto candidate = current - 1;
                if (evaluated.find(candidate) == evaluated.end())
                    neighbours.push_back(candidate);
            }
            if (current % WIDTH < WIDTH - 1)
            {
                auto candidate = current + 1;
                if (evaluated.find(candidate) == evaluated.end())
                    neighbours.push_back(candidate);
            }

            for (auto neighbour : neighbours)
            {
                auto& tile = (*tile_ref)[neighbour];
                if (tile == Tile::WALL)
                    continue;
                auto tentative_g = g(current) + heuristic(current, neighbour);

                discovered.insert(neighbour);
                if (tentative_g >= g(neighbour))
                    continue;

                efficient_previous[neighbour] = current;
                g_score[neighbour] = tentative_g;
                f_score[neighbour] = g_score[neighbour] + heuristic(neighbour, destination);
                if (!(tile == Tile::START || tile == Tile::END))
                    tile = Tile::COLOR1;
            }
        }
        yield();
    }
    yield_end();
}
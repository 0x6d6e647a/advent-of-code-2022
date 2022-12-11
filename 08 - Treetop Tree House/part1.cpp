#include <cstdint>
#include <iostream>
#include <map>

using Distance = int;
using CoordinatePair = std::pair<Distance, Distance>;
using Tree = std::uint8_t;

std::ostream & operator<<(std::ostream & os, const CoordinatePair & coordinates)
{
    return os << "(" << coordinates.first << ", " << coordinates.second << ")";
}

class Forest
{
private:
    std::map<CoordinatePair, Tree> _treeMap;
    Distance _height;
    Distance _width;

public:
    explicit Forest(std::istream & is)
    {
        Distance y = 0;

        for (std::string line; std::getline(is, line); ++y) {
            Distance x = 0;

            for (const auto & c : line) {
                _treeMap[{x, y}] = c;
                ++x;
            }

            _width = x;
        }

        _height = y;
    }

    ~Forest() = default;

    Tree getTree(const CoordinatePair & coordinates)
        const
    {
        try {
            return _treeMap.at(coordinates);
        }
        catch (const std::out_of_range & oor) {
            std::cerr << "tree out of forest bounds: " << coordinates << std::endl;
            std::terminate();
        }
    }

    bool onEdge(const CoordinatePair & coordinates)
        const
    {
        auto x = coordinates.first;
        auto y = coordinates.second;

        return ((x == 0)
                || (x == (_width - 1))
                || (y == 0)
                || (y == (_height - 1)));
    }

    bool isVisible(const CoordinatePair & coordinates)
        const
    {
        auto tree = getTree(coordinates);
        auto x = coordinates.first;
        auto y = coordinates.second;

        // Up
        bool upVisible = true;

        for (Distance yPrime = 0; yPrime < y; ++yPrime) {
            if (getTree({x, yPrime}) >= tree) {
                upVisible = false;
                break;
            }
        }

        if (upVisible) {
            return true;
        }

        // Down
        bool downVisible = true;

        for (Distance yPrime = y + 1; yPrime < _height; ++yPrime) {
            if (getTree({x, yPrime}) >= tree) {
                downVisible = false;
                break;
            }
        }

        if (downVisible) {
            return true;
        }

        // Left
        bool leftVisible = true;

        for (Distance xPrime = 0; xPrime < x; ++xPrime) {
            if (getTree({xPrime, y}) >= tree) {
                leftVisible = false;
                break;
            }
        }

        if (leftVisible) {
            return true;
        }

        // Right
        bool rightVisible = true;

        for (Distance xPrime = x + 1; xPrime < _width; ++xPrime) {
            if (getTree({xPrime, y}) >= tree) {
                rightVisible = false;
                break;
            }
        }

        if (rightVisible) {
            return true;
        }

        return false;
    }

    std::size_t countVisible()
        const
    {
        auto counter = 0;

        for (const auto & [coords, tree] : _treeMap) {
            if (onEdge(coords) || isVisible(coords)) {
                ++counter;
            }
        }

        return counter;
    }

#ifdef DEBUG
    void printVisibleMap()
    {
        for (Distance y = 0; y < _height; ++y) {
            for (Distance x = 0; x < _width; ++x) {
                if (onEdge({x, y})) {
                    std::cout << "E";
                }
                else if (isVisible({x, y})) {
                    std::cout << ".";
                }
                else {
                    std::cout << " ";
                }
            }
            std::cout << std::endl;
        }
    }
#endif
};

int main()
{
    Forest forest(std::cin);

#ifdef DEBUG
    forest.printVisibleMap();
#endif

    std::cout << forest.countVisible() << std::endl;

    return 0;
}

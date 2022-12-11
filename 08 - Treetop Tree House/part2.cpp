#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
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

    Distance senicScore(const CoordinatePair & coordinates)
        const
    {
        auto tree = getTree(coordinates);

        if (onEdge(coordinates)) {
            return 0;
        }

        auto x = coordinates.first;
        auto y = coordinates.second;

        // Up
        auto upScore = 0;

        for (Distance yPrime = y - 1; yPrime >= 0; --yPrime) {
            ++upScore;

            if (getTree({x, yPrime}) >= tree) {
                break;
            }
        }

        // Down
        auto downScore = 0;

        for (Distance yPrime = y + 1; yPrime < _height; ++yPrime) {
            ++downScore;

            if (getTree({x, yPrime}) >= tree) {
                break;
            }
        }

        // Left
        auto leftScore = 0;

        for (Distance xPrime = x - 1; xPrime >= 0; --xPrime) {
            ++leftScore;

            if (getTree({xPrime, y}) >= tree) {
                break;
            }
        }

        // Right
        auto rightScore = 0;

        for (Distance xPrime = x + 1; xPrime < _width; ++xPrime) {
            ++rightScore;

            if (getTree({xPrime, y}) >= tree) {
                break;
            }
        }

        return upScore
            * downScore
            * leftScore
            * rightScore;
    }

    Distance highestSenicScore()
    {
        Distance highest = 0;

        for (Distance y = 0; y < _height; ++y) {
            for (Distance x = 0; x < _width; ++x) {
                auto score = senicScore({x, y});

                if (score > highest) {
                    highest = score;
                }
            }
        }

        return highest;
    }

#ifdef DEBUG
    void printSenicMap()
    {
        std::ios::fmtflags oldFmtFlags = std::cout.flags();

        auto printWidth = std::to_string(std::pow(std::max(_height - 1, _width - 1), 4)).length();

        for (Distance y = 0; y < _height; ++y) {
            for (Distance x = 0; x < _width; ++x) {
                std::cout << "["
                          << std::setfill('0') << std::setw(printWidth) << senicScore({x, y})
                          << std::resetiosflags(oldFmtFlags)
                          << "]";
            }
            std::cout << std::endl;
        }

        std::cout.flags(oldFmtFlags);
    }
#endif
};

int main()
{
    Forest forest(std::cin);

#ifdef DEBUG
    forest.printSenicMap();
#endif

    std::cout << forest.highestSenicScore() << std::endl;

    return 0;
}

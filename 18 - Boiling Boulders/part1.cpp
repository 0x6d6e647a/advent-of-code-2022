#include <algorithm>
#include <array>
#include <iostream>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using Int = std::int32_t;

class Coord3D
    : public std::tuple<Int, Int, Int>
{
private:
    using Parent = std::tuple<Int, Int, Int>;

public:
    constexpr Coord3D()
        : Parent(0, 0, 0)
    {}

    constexpr Coord3D(Int x, Int y, Int z)
        : Parent(x, y, z)
    {}

    auto x() const { return std::get<0>(*this); };
    auto y() const { return std::get<1>(*this); };
    auto z() const { return std::get<2>(*this); };

    bool operator==(const Coord3D & other)
        const
    {
        return ((x() == other.x()) &&
                (y() == other.y()) &&
                (z() == other.z()));
    }

    Coord3D operator+(const Coord3D & other)
        const
    {
        return {
            x() + other.x(),
            y() + other.y(),
            z() + other.z()};
    }
};

class CoordCollection
{
private:
    std::unordered_map<Int, std::unordered_map<Int, std::unordered_set<Int>>> _tracker;

public:
    void insert(const Coord3D & coord)
    {
        _tracker[coord.x()][coord.y()].insert(coord.z());
    }

    bool contains(const Coord3D & coord)
        const
    {
        if (!_tracker.contains(coord.x())) {
            return false;
        }

        try {
            auto mapY = _tracker.at(coord.x());

            if (!mapY.contains(coord.y())) {
                return false;
            }

            auto setZ = mapY.at(coord.y());

            return setZ.contains(coord.z());
        }
        catch (const std::out_of_range & ex) {
            return false;
        }
    }
};

enum class Direction { posX, negX, posY, negY, posZ, negZ };

constexpr std::array<Direction, 6> directions{
    Direction::posX,
    Direction::negX,
    Direction::posY,
    Direction::negY,
    Direction::posZ,
    Direction::negZ};

constexpr Coord3D dirPosX{ 1,  0,  0};
constexpr Coord3D dirNegX{-1,  0,  0};
constexpr Coord3D dirPosY{ 0,  1,  0};
constexpr Coord3D dirNegY{ 0, -1,  0};
constexpr Coord3D dirPosZ{ 0,  0,  1};
constexpr Coord3D dirNegZ{ 0,  0, -1};

const Coord3D directionVector(const Direction & direction)
{
    switch (direction) {
    case Direction::posX: {
        return dirPosX;
    }
    case Direction::negX: {
        return dirNegX;
    }
    case Direction::posY: {
        return dirPosY;
    }
    case Direction::negY: {
        return dirNegY;
    }
    case Direction::posZ: {
        return dirPosZ;
    }
    case Direction::negZ: {
        return dirNegZ;
    }
    }

    std::cerr << "unknown direction" << std::endl;
    std::terminate();
}

class Lava
{
private:
    CoordCollection _coordCollection;
    std::vector<Coord3D> _droplets;

public:
    explicit Lava(std::istream & is)
    {
        for (std::string line; std::getline(is, line);) {
            std::replace(line.begin(), line.end(), ',', ' ');
            std::stringstream ss{line};
            Int x, y, z;
            ss >> x >> y >> z;
            Coord3D coord{x, y, z};
            _coordCollection.insert(coord);
            _droplets.push_back(coord);
        }
    }

    Int score()
        const
    {
        Int score = 0;

        for (const auto & droplet : _droplets) {
            for (const auto & direction : directions) {
                auto dv = directionVector(direction);

                if (!_coordCollection.contains(droplet + dv)) {
                    ++score;
                }
            }
        }

        return score;
    }
};

int main()
{
    Lava lava{std::cin};
    std::cout << lava.score() << std::endl;
    return 0;
}

#include <algorithm>
#include <array>
#include <deque>
#include <iostream>
#include <limits>
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
    CoordCollection _lavaDropletsCollection;
    std::vector<Coord3D> _lavaDroplets;
    CoordCollection _waterDropletsCollection;
    std::vector<Coord3D> _waterDroplets;
    std::unordered_map<std::string, Int> _bounds;

    void findBounds()
    {
        _bounds["minX"] = std::numeric_limits<Int>::max();
        _bounds["maxX"] = std::numeric_limits<Int>::min();
        _bounds["minY"] = std::numeric_limits<Int>::max();
        _bounds["maxY"] = std::numeric_limits<Int>::min();
        _bounds["minZ"] = std::numeric_limits<Int>::max();
        _bounds["maxZ"] = std::numeric_limits<Int>::min();

        for (const auto & droplet : _lavaDroplets) {
            auto x = droplet.x();
            auto y = droplet.y();
            auto z = droplet.z();

            if (x < _bounds["minX"]) { _bounds["minX"] = x; }
            if (x > _bounds["maxX"]) { _bounds["maxX"] = x; }
            if (y < _bounds["minY"]) { _bounds["minY"] = y; }
            if (y > _bounds["maxY"]) { _bounds["maxY"] = y; }
            if (z < _bounds["minZ"]) { _bounds["minZ"] = z; }
            if (z > _bounds["maxZ"]) { _bounds["maxZ"] = z; }
        }

        for (const auto & pair : _bounds) {
            auto label = pair.first;

            if (label.rfind("min", 0) == 0) {
                _bounds[label] -= 3;
            }
            else if (label.rfind("max", 0) == 0) {
                _bounds[label] += 3;
            }
        }
    }

    void fill()
    {
        Coord3D start{_bounds["minX"], _bounds["minY"], _bounds["minZ"]};
        std::deque<Coord3D> todo{{start}};

        while (!todo.empty()) {
            auto curr{todo.front()};
            todo.pop_front();

            _waterDropletsCollection.insert(curr);
            _waterDroplets.push_back(curr);

            for (const auto & direction : directions) {
                auto dv{directionVector(direction)};
                auto next{curr + dv};

                if (!_lavaDropletsCollection.contains(next)) {
                    if (!_waterDropletsCollection.contains(next)) {
                        if (inBounds(next)) {
                            if (std::find(
                                    todo.begin(),
                                    todo.end(),
                                    next) == std::end(todo)) {
                                todo.push_back(next);
                            }
                        }
                    }
                }
            }
        }
    }

public:
    explicit Lava(std::istream & is)
    {
        for (std::string line; std::getline(is, line);) {
            std::replace(line.begin(), line.end(), ',', ' ');
            std::stringstream ss{line};
            Int x, y, z;
            ss >> x >> y >> z;
            Coord3D coord{x, y, z};
            _lavaDropletsCollection.insert(coord);
            _lavaDroplets.push_back(coord);
        }

        findBounds();
        fill();
    }

    bool inBounds(const Coord3D & coord)
        const
    {
        auto x = coord.x();
        auto y = coord.y();
        auto z = coord.z();

        try {
            return ((x >= _bounds.at("minX")) &&
                    (x <= _bounds.at("maxX")) &&
                    (y >= _bounds.at("minY")) &&
                    (y <= _bounds.at("maxY")) &&
                    (z >= _bounds.at("minZ")) &&
                    (z <= _bounds.at("maxZ")));
        }
        catch (const std::out_of_range & ex) {
            std::cerr << "invalid boundary label" << std::endl;
            std::terminate();
        }
    }

    Int score()
        const
    {
        Int score = 0;

        for (const auto & droplet : _lavaDroplets) {
            for (const auto & direction : directions) {
                auto dv{directionVector(direction)};
                auto curr{droplet + dv};

                if ((!_lavaDropletsCollection.contains(curr)) &&
                    (_waterDropletsCollection.contains(curr))) {
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

#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <memory>
#include <queue>
#include <sstream>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using Int = std::int64_t;
using Size = std::size_t;
using String = std::string;

class CoordinatePair
    : public std::pair<Int, Int>
{
private:
    using Parent = std::pair<Int, Int>;

public:
    CoordinatePair()
        : Parent(0, 0)
    {}

    constexpr CoordinatePair(Int x, Int y)
        : Parent(x, y)
    {}

    auto x() const { return first; }
    auto y() const { return second; }

    String toString()
        const
    {
        std::stringstream ss;
        ss << "(" << x() << "," << y() << ")";
        return ss.str();
    }

    bool operator==(const CoordinatePair & other)
        const
    {
        if (x() != other.x()) {
            return false;
        }

        if (y() != other.y()) {
            return false;
        }

        return true;
    }

    CoordinatePair operator+(const CoordinatePair & other)
        const
    {
        return {first + other.first, second + other.second};
    }

    Int distance(const CoordinatePair & other)
        const
    {
        // Manhattan Distance
        return
            std::abs(x() - other.x()) +
            std::abs(y() - other.y());
    }
};

template<>
struct std::hash<CoordinatePair>
{
    std::size_t operator()(const CoordinatePair & coordinatePair)
        const
    {
        std::stringstream ss;
        ss << coordinatePair.x() << "," << coordinatePair.y();
        return std::hash<std::string>{}(ss.str());
    }
};

std::ostream & operator<<(std::ostream & os, const CoordinatePair & coord)
{
    os << coord.toString();
    return os;
}

enum class Direction { Up, Down, Left, Right };

Direction charToDirection(const char & c)
{
    switch (c) {
        case '^': {
            return Direction::Up;
        }
        case 'v': {
            return Direction::Down;
        }
        case '<': {
            return Direction::Left;
        }
        case '>': {
            return Direction::Right;
        }
    }

    std::cerr << "unable to get direction for character: " << c << std::endl;
    std::terminate();
}

std::array<Direction, 4> allDirections()
{
    static constexpr std::array<Direction, 4> allDirs{
        Direction::Up,
        Direction::Down,
        Direction::Left,
        Direction::Right,
    };

    return allDirs;
}

CoordinatePair directionToForceVector(const Direction & direction)
{
    switch (direction) {
        case Direction::Up: {
            return {0, -1};
        }
        case Direction::Down: {
            return {0, 1};
        }
        case Direction::Left: {
            return {-1, 0};
        }
        case Direction::Right: {
            return {1, 0};
        }
    }

    std::cerr << "unable to get force vector for direction" << std::endl;
    std::terminate();
}

String directionToString(const Direction & direction)
{
    switch (direction) {
        case Direction::Up: {
            return "Up";
        }
        case Direction::Down: {
            return "Down";
        }
        case Direction::Left: {
            return "Left";
        }
        case Direction::Right: {
            return "Right";
        }
    }

    std::cerr << "unable to get string for direction" << std::endl;
    std::terminate();
}

std::ostream & operator<<(std::ostream & os, const Direction & direction)
{
    os << directionToString(direction);
    return os;
}

using WallSet = std::unordered_set<CoordinatePair>;

class Blizzard
{
private:
    CoordinatePair _coord;
    Direction _direction;

public:
    Blizzard(CoordinatePair coord, Direction dir)
        : _coord(coord)
        , _direction(dir)
    {}

    Blizzard(Int x, Int y, Direction dir)
        : Blizzard({x, y}, dir)
    {}

    auto x() const { return _coord.x(); }
    auto y() const { return _coord.y(); }
    auto direction() const { return _direction; }

    Blizzard move(Int height, Int width, const std::shared_ptr<WallSet> & walls)
        const
    {
        Blizzard newBlizzard{*this};

        newBlizzard._coord = newBlizzard._coord + directionToForceVector(_direction);

        if (!walls->contains(newBlizzard._coord)) {
            return newBlizzard;
        }

        switch (_direction) {
            case Direction::Up: {
                newBlizzard._coord = {x(), height - 2};
                break;
            }
            case Direction::Down: {
                newBlizzard._coord = {x(), 1};
                break;
            }
            case Direction::Left: {
                newBlizzard._coord = {width - 2, y()};
                break;
            }
            case Direction::Right: {
                newBlizzard._coord = {1, y()};
                break;
            }
        } 

        return newBlizzard;
    }

    bool operator==(const Blizzard & other)
        const
    {
        return ((_coord == other._coord) &&
                (_direction == other._direction));
    }

    String toString()
        const
    {
        std::stringstream ss;

        ss << '(' << x() << ',' << y() << ',' << directionToString(_direction) << ')';

        return ss.str();
    }
};

using BlizzardMap = std::unordered_map<CoordinatePair, std::unordered_set<Blizzard>>;

std::ostream & operator<<(std::ostream & os, const Blizzard & blizzard)
{
    os << blizzard.toString();
    return os;
}

template<>
struct std::hash<Blizzard>
{
    std::size_t operator()(const Blizzard & blizzard)
        const
    {
        return std::hash<std::string>{}(blizzard.toString());
    }
};

class State
{
private:
    Size _minute;
    CoordinatePair _playerPosition;
    std::shared_ptr<WallSet> _walls;
    std::shared_ptr<BlizzardMap> _blizzards;
    Int _numWaited;

public:
    State()
        : _minute(0)
        , _numWaited(0)
    {}

    State(Size minute,
          CoordinatePair playerPosition,
          std::shared_ptr<WallSet> walls,
          std::shared_ptr<BlizzardMap> blizzards,
          Int numWaited = 0)
        : _minute(minute)
        , _playerPosition(playerPosition)
        , _walls(walls)
        , _blizzards(blizzards)
        , _numWaited(numWaited)
    {}

    auto minute() const { return _minute; }
    auto playerPosition() const { return _playerPosition; }
    auto blizzards() const { return _blizzards; }
    auto numWaited() const { return _numWaited; }

    String toString()
        const
    {
        std::stringstream ss;

        ss
            << "(" << _playerPosition.x()
            << "," << _playerPosition.y()
            << "," << _minute
            // << "," << _numWaited
            << ")";

        return ss.str();
    }

    bool operator==(const State & other)
        const
    {
        // return ((_roundIndex == other._roundIndex) &&
        //         (_playerPosition == other._playerPosition));

        return toString() == other.toString();
    }

    Int score(const CoordinatePair & end)
        const
    {
        return _playerPosition.distance(end);
    }
};

std::ostream & operator<<(std::ostream & os, const State & state)
{
    os << state.toString();
    return os;
}

template<>
struct std::hash<State>
{
    std::size_t operator()(const State & state)
        const
    {
        return std::hash<std::string>{}(state.toString());
    }
};

class GameBoard
{
private:
    CoordinatePair _start;
    CoordinatePair _end;
    Int _width;
    Int _height;
    std::shared_ptr<WallSet> _walls;
    std::unordered_map<Size, std::shared_ptr<BlizzardMap>> _blizzardsByTimeMap;

public:
    explicit GameBoard(std::istream & is)
        : _walls(std::make_shared<WallSet>())
    {
        String prevLine;
        Int y = 0;

        auto initBlizzards = std::make_shared<BlizzardMap>();

        for (String line; std::getline(is, line);) {
            if (line.length() > std::numeric_limits<Int>::max()) {
                std::cerr << "input line too long" << std::endl;
                std::terminate();
            }

            Int lineLength = static_cast<Int>(line.length());

            for (Int x = 0; x < lineLength; ++x) {
                if ((y == 0) && (line[x] == '.')) {
                    _start = {x, y};
                }
                else if (line[x] == '#') {
                    _walls->insert({x, y});
                }
                else if ((line[x] == '^') ||
                         (line[x] == 'v') ||
                         (line[x] == '<') ||
                         (line[x] == '>')) {
                    (*initBlizzards)[{x, y}].insert({x, y, charToDirection(line[x])});
                }
            }

            prevLine = line;
            ++y;
        }

        _width = static_cast<Int>(prevLine.length());
        _height = y;

        --y;

        for (Int x = 0; x < _width; ++x) {
            if (prevLine[x] == '.') {
                _end = {x, y};
            }
        }

        _blizzardsByTimeMap[0] = initBlizzards;
    }

    bool isValidPlayerPosition(const CoordinatePair & playerPosition, const std::shared_ptr<BlizzardMap> & blizzardMap)
        const
    {
        // Check that the player is still in bounds.
        if ((playerPosition.x() < 0) ||
            (playerPosition.x() > _width - 1) ||
            (playerPosition.y() < 0) ||
            (playerPosition.y() > _height - 1)) {
            return false;
        }
                
        // Check if player position collides with a wall.
        if (_walls->contains(playerPosition)) {
            return false;
        }

        // Check if player position collides with a blizzard.
        if (blizzardMap->contains(playerPosition)) {
            return false;
        }

        return true;
    }

    std::shared_ptr<BlizzardMap> getBlizzardsByMinute(Size minute)
    {
        // Return if we already generated one.
        if (_blizzardsByTimeMap.contains(minute)) {
            return _blizzardsByTimeMap[minute];
        }

        // Check if we already generated previous, if not generate it.
        std::shared_ptr<BlizzardMap> oldBlizzards =
            getBlizzardsByMinute(minute - 1);

        // Create new blizzard.
        auto newBlizzards{std::make_shared<BlizzardMap>()};

        for (const auto & [coord, blizzards] : *oldBlizzards) {
            for (const Blizzard & blizzard : blizzards) {
                Blizzard newBlizzard{blizzard.move(_height, _width, _walls)};

                auto x{newBlizzard.x()};
                auto y{newBlizzard.y()};

                (*newBlizzards)[{x, y}].insert(newBlizzard);
            }
        }

        // Store new blizzards.
        _blizzardsByTimeMap[minute] = newBlizzards;

        return newBlizzards;
    }

    Size getSearchMinuteLimit()
        const
    { 
        return (_height * _width) * 5;
    }

    Int breadthFirstSearch()
    {
        std::vector<State> states{{State(0, _start, _walls, _blizzardsByTimeMap[0])}};
        std::unordered_set<State> visited;

        for (Size minute = 0; minute < getSearchMinuteLimit(); ++minute) {
            std::vector<State> next;

            for (const auto & state : states) {
                if (visited.contains(state)) {
                    // std::cout << "skipped visited state: " << state << std::endl;
                    continue;
                }
                else {
                    visited.insert(state);
                }

                Size newMinute{state.minute() + 1};
                auto newBlizzards{getBlizzardsByMinute(newMinute)};

                // Move in every direction.
                for (const auto & direction : allDirections()) {
                    CoordinatePair newPlayerPosition = state.playerPosition() + directionToForceVector(direction);

                    // Skip invalid states.
                    if (!isValidPlayerPosition(newPlayerPosition, newBlizzards))
                    {
                        continue;
                    }

                    next.emplace_back(
                        newMinute,
                        newPlayerPosition,
                        _walls,
                        newBlizzards);
                }

                // Don't move if valid.
                if (isValidPlayerPosition(state.playerPosition(), newBlizzards)) {
                    next.emplace_back(
                        newMinute,
                        state.playerPosition(),
                        _walls,
                        newBlizzards);
                }
            }

            // Trim number of states by best results.
            static constexpr Size sizeLimit = 100'000'000;

            if (next.size() > sizeLimit) {
                std::sort(
                    next.begin(),
                    next.end(),
                    [*this](const State & a, const State & b) {
                        return a.score(_end) < b.score(_end);
                    });

                next.erase(std::next(next.begin(), sizeLimit), next.end());
            }

            // See if we found the end.
            auto findIter = std::find_if(
                next.begin(),
                next.end(),
                [*this](const State & state) {
                    return state.playerPosition() == _end;
                });

            if (findIter != next.end()) {
                return findIter->minute();
            }

            // Next states become current.
            states = next;
        }

        return -1;
    }
};

int main()
{
    GameBoard gameBoard(std::cin);
    std::cout << gameBoard.breadthFirstSearch() << std::endl;
    // std::cout << gameBoard.depthFirstSearch() << std::endl;
    return 0;
}

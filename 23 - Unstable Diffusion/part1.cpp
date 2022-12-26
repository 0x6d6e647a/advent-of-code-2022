#include <array>
#include <deque>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <sstream>
#include <unordered_map>
#include <utility>
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

enum class Direction { North, NorthEast, NorthWest, South, SouthEast, SouthWest, West, East };

String directionToString(const Direction & direction)
{
    switch (direction) {
        case Direction::North: {
            return "North";
        }
        case Direction::NorthEast: {
            return "Northeast";
        }
        case Direction::NorthWest: {
            return "Northwest";
        }
        case Direction::South: {
            return "South";
        }
        case Direction::SouthEast: {
            return "Southeast";
        }
        case Direction::SouthWest: {
            return "Southwest";
        }
        case Direction::West: {
            return "West";
        }
        case Direction::East: {
            return "East";
        }
    }

    std::cerr << "invalid direction for to string" << std::endl;
    std::terminate();
}

std::ostream & operator<<(std::ostream & os, const Direction & direction)
{
    os << directionToString(direction);
    return os;
}

CoordinatePair directionToForceVector(const Direction & direction)
{
    switch (direction) {
        case Direction::North: {
            return {0, -1};
        }
        case Direction::NorthEast: {
            return {1, -1};
        }
        case Direction::NorthWest: {
            return {-1, -1};
        }
        case Direction::South: {
            return {0, 1};
        }
        case Direction::SouthEast: {
            return {1, 1};
        }
        case Direction::SouthWest: {
            return {-1, 1};
        }
        case Direction::West: {
            return {-1, 0};
        }
        case Direction::East: {
            return {1, 0};
        }
    }

    std::cerr << "invalid direction for force vector" << std::endl;
    std::terminate();
}

const std::array<Direction, 8> & allDirections()
{
    static constexpr std::array<Direction, 8> allDirections{
        Direction::North,
        Direction::NorthEast,
        Direction::NorthWest,
        Direction::South,
        Direction::SouthEast,
        Direction::SouthWest,
        Direction::West,
        Direction::East
    };

    return allDirections;
}

const std::array<Direction, 3> & checkDirections(const Direction & direction)
{
    static constexpr std::array<Direction, 3> northDirs{ Direction::North, Direction::NorthEast, Direction::NorthWest, };
    static constexpr std::array<Direction, 3> southDirs{ Direction::South, Direction::SouthEast, Direction::SouthWest, };
    static constexpr std::array<Direction, 3> westDirs{ Direction::West, Direction::NorthWest, Direction::SouthWest, };
    static constexpr std::array<Direction, 3> eastDirs{ Direction::East, Direction::NorthEast, Direction::SouthEast, };

    switch (direction) {
        case Direction::North: {
            return northDirs;
        }
        case Direction::South: {
            return southDirs;
        }
        case Direction::West: {
            return westDirs;
        }
        case Direction::East: {
            return eastDirs;
        }
        case Direction::NorthEast:
        case Direction::NorthWest:
        case Direction::SouthEast:
        case Direction::SouthWest:
            std::cerr << "no check directions for direction: " << directionToString(direction) << std::endl;
            std::terminate();
    }

    std::cerr << "unknown direction for to check direction" << std::endl;
    std::terminate();
}

class Elf
{
private:
    CoordinatePair _coord;
    std::optional<CoordinatePair> _considering;

public:
    explicit Elf(const CoordinatePair & coord)
        : _coord(coord)
    {}
    
    Elf(Int x, Int y)
        : Elf(CoordinatePair(x, y))
    {}

    auto x() const { return _coord.x(); }
    auto y() const { return _coord.y(); }

    bool operator==(const Elf & other)
        const
    {
        return ((_coord == other._coord) &&
                sameConsidering(other));
    }

    bool isConsidering()
        const
    {
        return _considering.has_value();
    }

    bool sameConsidering(const Elf & other)
        const
    {
        return _considering == other._considering;
    }

    void considerMove(const std::unordered_map<CoordinatePair, Elf> & elves,
                      const std::deque<Direction> & considerDirections)
    {
        _considering.reset();

        // Check all directions, don't move if all empty.
        bool allEmpty = true;

        for (const auto & direction : allDirections()) {
            CoordinatePair coord{_coord + directionToForceVector(direction)};

            if (elves.contains(coord)) {
                allEmpty = false;
                break;
            }
        }

        if (allEmpty) {
            return;
        }

        // Check consider directions to see where to move.
        for (const auto & considerDirection : considerDirections) {
            // Check all check directions for direction being considered.
            bool allClear = true;

            for (const auto & checkDirection : checkDirections(considerDirection)) {
                CoordinatePair considerCoord{_coord + directionToForceVector(checkDirection)};

                if (elves.contains(considerCoord)) {
                    allClear = false;
                    break;
                }
            }

            // All directions are clear, use this as the considered move to spot.
            if (allClear) {
                _considering = _coord + directionToForceVector(considerDirection);
                break;
            }
        }
    }

    void doMove()
    {
        _coord = *_considering;
    }
};

class GameBoard
{
private:
    std::unordered_map<CoordinatePair, Elf> _elves;
    std::deque<Direction> _considerDirections;

    void rotateConsiderDirections()
    {
        auto tmp{_considerDirections.front()};
        _considerDirections.pop_front();
        _considerDirections.push_back(tmp);
    }

    struct BoundingBox
    {
        Int lowestX, highestX, lowestY, highestY;

        BoundingBox()
            : lowestX(std::numeric_limits<Int>::max())
            , highestX(std::numeric_limits<Int>::min())
            , lowestY(std::numeric_limits<Int>::max())
            , highestY(std::numeric_limits<Int>::min())
        {}

        void newLowestX(Int x)
        {
            if (x < lowestX) {
                lowestX = x;
            }
        }

        void newHighestX(Int x)
        {
            if (x > highestX) {
                highestX = x;
            }
        }

        void newLowestY(Int y)
        {
            if (y < lowestY) {
                lowestY = y;
            }
        }

        void newHighestY(Int y)
        {
            if (y > highestY) {
                highestY = y;
            }
        }
    };

    BoundingBox getBoundingBox()
        const
    {
        BoundingBox boundingBox;

        for (const auto & [coord, elf] : _elves) {
            boundingBox.newLowestX(coord.x());
            boundingBox.newHighestX(coord.x());
            boundingBox.newLowestY(coord.y());
            boundingBox.newHighestY(coord.y());
        }

        return boundingBox;
    }

    void considerPhase()
    {
        for (auto & [coord, elf] : _elves) {
            elf.considerMove(_elves, _considerDirections);
        }
    }

    void movePhase()
    {
        std::vector<Elf> newPositions;

        for (auto elfIter = _elves.begin();
             elfIter != _elves.end();
             ++elfIter) {
            auto elf = elfIter->second;

            if (!elf.isConsidering()) {
                newPositions.push_back(elf);
                continue;
            }

            // Don't move if another elf is considering the same position.
            bool isSameConsidering = false;

            for (const auto & [coord, elf2] : _elves) {
                // Ignore self.
                if (elf == elf2) {
                    continue;
                }

                if (elf.sameConsidering(elf2)) {
                    isSameConsidering = true;
                    break;
                }
            }

            if (!isSameConsidering) {
                elf.doMove();
            }

            newPositions.push_back(elf);
        }

        _elves.clear();

        for (const auto & newElfPos : newPositions) {
            CoordinatePair newCoord{newElfPos.x(), newElfPos.y()};
            _elves.insert({newCoord, newElfPos});
        }
    }
    
public:
    explicit GameBoard(std::istream & is)
        : _considerDirections({
                Direction::North,
                Direction::South,
                Direction::West,
                Direction::East,
            })
    {
        Int y = 0;


        for (String line; std::getline(is, line);) {
            if (line.length() > std::numeric_limits<Int>::max()) {
                std::cerr << "input line too long" << std::endl;
                std::terminate();
            }

            Int lineLength = static_cast<Int>(line.length());
            
            for (Int x = 0; x < lineLength; ++x) {
                if (line[x] == '#') {
                    CoordinatePair coord{x, y};
                    _elves.emplace(coord, coord);
                }
            }

            ++y;
        }
    }

    String toString()
        const
    {
        std::stringstream ss;

        BoundingBox boundingBox{getBoundingBox()};

        for (Int y = boundingBox.lowestY; y <= boundingBox.highestY; ++y) {
            for (Int x = boundingBox.lowestX; x <= boundingBox.highestX; ++x) {
                if (_elves.contains({x, y})) {
                    ss << '#';
                }
                else {
                    ss << '.';
                }
            }

            ss << std::endl;
        }

        return ss.str();
    }

    void playGame(Int numRounds = 10)
    {
        for (Int round = 0; round < numRounds; ++round) {
            considerPhase();
            movePhase();
            rotateConsiderDirections();
        }
    }

    Int score()
        const
    {
        Int score = 0;
        BoundingBox boundingBox{getBoundingBox()};

        for (Int y = boundingBox.lowestY; y <= boundingBox.highestY; ++y) {
            for (Int x = boundingBox.lowestX; x <= boundingBox.highestX; ++x) {
                if (!_elves.contains({x, y})) {
                    ++score;
                } 
            }
        }

        return score;
    }
};

std::ostream & operator<<(std::ostream & os, const GameBoard & gameBoard)
{
    os << gameBoard.toString();
    return os;
}

int main()
{
    GameBoard gameBoard{std::cin};

#ifdef DEBUG
    std::cout << gameBoard << std::endl;
#endif // DEBUG

    gameBoard.playGame();

#ifdef DEBUG
    std::cout << gameBoard << std::endl;
#endif // DEBUG

    std::cout << gameBoard.score() << std::endl;

    return 0;
}

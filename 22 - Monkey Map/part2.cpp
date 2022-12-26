#include <iostream>
#include <limits>
#include <optional>
#include <regex>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <variant>

using Int = std::int64_t;
using String = std::string;

enum class Turn { Clockwise, CounterClockwise };

Turn charToTurn(const char & c)
{
    switch (c) {
    case 'r':
    case 'R': {
        return Turn::Clockwise;
    }
    case 'l':
    case 'L': {
        return Turn::CounterClockwise;
    }
    }

    std::cerr << "invalid character for generating turn: " << c << std::endl;
    std::terminate();
}

enum class Direction { Up, Down, Left, Right };

Int directionToScore(const Direction & direction)
{
    switch (direction) {
    case Direction::Right: {
        return 0;
    }
    case Direction::Down: {
        return 1;
    }
    case Direction::Left: {
        return 2;
    }
    case Direction::Up: {
        return 3;
    }
    }

    std::cerr << "unable to score invalid direction" << std::endl;
    std::terminate();
}

Direction turnDirection(const Direction & direction, const Turn & turn)
{
    switch (turn) {
    case Turn::Clockwise: {
        switch (direction) {
        case Direction::Up: {
            return Direction::Right;
        }
        case Direction::Down: {
            return Direction::Left;
        }
        case Direction::Left: {
            return Direction::Up;
        }
        case Direction::Right: {
            return Direction::Down;
        }
        default:
            std::cerr << "invalid direction" << std::endl;
            std::terminate();
            break;
        }
        break;
    }
    case Turn::CounterClockwise: {
        switch (direction) {
        case Direction::Up: {
            return Direction::Left;
        }
        case Direction::Down: {
            return Direction::Right;
        }
        case Direction::Left: {
            return Direction::Down;
        }
        case Direction::Right: {
            return Direction::Up;
        }
        default:
            std::cerr << "invalid direction" << std::endl;
            std::terminate();
            break;
        }
        break;
    }
    default:
        std::cerr << "invalid turn" << std::endl;
        std::terminate();
    }
}

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

    std::cerr << "invalid direction for force vector" << std::endl;
    std::terminate();
}

class CoordinateCollection
{
private:
    std::unordered_map<Int, std::unordered_set<Int>> _tracker;

public:
    CoordinateCollection()
    {}

    void insert(const CoordinatePair & coord)
    {
        _tracker[coord.y()].insert(coord.x());
    }

    bool contains(const CoordinatePair & coord)
        const
    {
        if (!_tracker.contains(coord.y())) {
            return false;
        }

        try {
            auto xSet = _tracker.at(coord.y());
            return xSet.contains(coord.x());
        } catch (const std::out_of_range & ex) {
            return false;
        }
    }
};


//    +--+--+
//    |AA|BB|
//    |AA|BB|
//    +--+--+
//    |CC|
//    |CC|
// +--+--+
// |DD|EE|
// |DD|EE|
// +--+--+
// |FF|
// |FF|
// +--+
enum class CubeFace { A, B, C, D, E, F };

class BoardMap
{
private:
    CoordinatePair _start;
    CoordinateCollection _tiles;
    CoordinateCollection _walls;
    Int _width;

public:
    explicit BoardMap(std::istream & is)
        : _width(50)
    {
        bool foundStart = false;

        String line;

        for (Int y = 0; std::getline(is, line); ++y) {
            if (line.empty()) {
                break;
            }

            // Handle conversion between String;:size_type and Int.
            auto lineSize = line.size();
            Int xSize;

            if (lineSize < std::numeric_limits<Int>::max()) {
                xSize = static_cast<Int>(lineSize);
            }
            else {
                std::cerr << "line size too large to store: " << line.size() << std::endl;
                std::terminate();
            }

            for (Int x = 0; x < xSize; ++x) {
                char c = line[x];

                if (c == ' ') {
                    continue;
                }
                else if (c == '.') {
                    _tiles.insert({x, y});

                    if (!foundStart) {
                        _start = {x, y};
                        foundStart = true;
                    }
                }
                else if (c == '#') {
                    _walls.insert({x, y});
                }
                else {
                    std::cerr << "unexpected character in board map: " << c << std::endl;
                    std::terminate();
                }
            }
        }
    }

    auto start() const { return _start; }
    auto width() const { return _width; }

    bool isTile(const CoordinatePair & coord)
        const
    {
        return _tiles.contains(coord);
    }

    bool isWall(const CoordinatePair & coord)
        const
    {
        return _walls.contains(coord);
    }

    CubeFace getCubeFace(const CoordinatePair & coord)
        const
    {
        if ((coord.y() >= 0) &&
            (coord.y() <= (_width * 1) - 1)) {
            if ((coord.x() >= (_width * 1)) &&
                (coord.x() <= (_width * 2) - 1)) {
                return CubeFace::A;
            }
            else if ((coord.x() >= (_width * 2)) &&
                     (coord.x() <= (_width * 3) - 1)) {
                return CubeFace::B;
            }
            else {
                std::cerr << "unable to calculate cube face" << std::endl;
                std::terminate();
            }
        }
        else if ((coord.y() >= (_width * 1)) &&
                 (coord.y() <= (_width * 2) - 1)) {
            if ((coord.x() >= (_width * 1)) &&
                (coord.x() <= (_width * 2) - 1)) {
                return CubeFace::C;
            }
            else {
                std::cerr << "unable to calculate cube face" << std::endl;
                std::terminate();
            }
        }
        else if ((coord.y() >= (_width * 2)) &&
                 (coord.y() <= (_width * 3) - 1)) {
            if ((coord.x() >= 0) &&
                (coord.x() <= (_width * 1) - 1)) {
                return CubeFace::D;
            }
            else if ((coord.x() >= (_width * 1)) &&
                     (coord.x() <= (_width * 2) - 1)) {
                return CubeFace::E;
            }
            else {
                std::cerr << "unable to calculate cube face" << std::endl;
                std::terminate();
            }
        }
        else if ((coord.y() >= (_width * 3)) &&
                 (coord.y() <= (_width * 4) - 1)) {
            if ((coord.x() >= 0) &&
                (coord.x() <= (_width * 1) - 1)) {
                return CubeFace::F;
            }
            else {
                std::cerr << "unable to calculate cube face" << std::endl;
                std::terminate();
            }
        }
        else {
            std::cerr << "unable to calculate cube face" << std::endl;
            std::terminate();
        }
    }
};

using Step = std::variant<Int, Turn>;

class PathDirections
{
private:
    std::vector<Step> _steps;

public:
    explicit PathDirections(std::istream & is)
    {
        static const std::regex noSpaceDirection{"([^\\s])([uUdDlLrR])([^\\s])"};

        String line;
        std::getline(is, line);

        while (std::regex_search(line, noSpaceDirection)) {
            line = std::regex_replace(line, noSpaceDirection, "$1 $2 $3");
        }

        static const std::regex direction{"^[uUdDlLrR]$"};
        static const std::regex numSteps{"^\\d+$"};

        String stepStr;
        std::stringstream ss{line};

        while (ss >> stepStr) {
            if (std::regex_match(stepStr, direction)) {
                _steps.push_back(charToTurn(stepStr[0]));
            }
            else if (std::regex_match(stepStr, numSteps)) {
                _steps.push_back(std::stoll(stepStr));
            }
            else {
                std::cerr << "malformed step: " << stepStr << std::endl;
                std::terminate();
            }
        }
    }

    auto begin() const { return _steps.begin(); }
    auto end() const { return _steps.end(); }
    auto cbegin() const { return _steps.cbegin(); }
    auto cend() const { return _steps.cend(); }
};

using Position = std::pair<CoordinatePair, Direction>;

Position edgeWalk(const Position & playerPosition, const BoardMap & boardMap)
{
    CoordinatePair playerCoord{playerPosition.first};
    Direction playerDir{playerPosition.second};
    CubeFace cubeFace{boardMap.getCubeFace(playerCoord)};
    Int width{boardMap.width()};

    switch (cubeFace) {
        case CubeFace::A: {
            switch (playerDir) {
                case Direction::Up: {
                    // Left F
                    playerCoord = {
                        0,
                        (playerCoord.x() % width) + (width * 3),
                    };
                    playerDir = Direction::Right;
                    break;
                }
                case Direction::Left: {
                    // Left D
                    playerCoord = {
                        0,
                        (-1 * (playerCoord.y() % width) + (width - 1)) + (width * 2),
                    };
                    playerDir = Direction::Right;
                    break;
                }
                default:
                    std::cerr << "invalid direction for edge walk" << std::endl;
                    std::terminate();
                    break;
            }
            break;
        }
        case CubeFace::B: {
            switch (playerDir) {
                case Direction::Up: {
                    // Bottom F
                    playerCoord = {
                        (playerCoord.x() % width) + (width * 0),
                        (width * 4) - 1,
                    };
                    // No direction change.
                    break;
                }
                case Direction::Down: {
                    // Right C
                    playerCoord = {
                        (width * 2) - 1,
                        (playerCoord.x() % width) + (width * 1),
                    };
                    playerDir = Direction::Left;
                    break;
                }
                case Direction::Right: {
                    // Right E
                    playerCoord = {
                        (width * 2) - 1,
                        (-1 * (playerCoord.y() % width) + (width - 1)) + (width * 2)
                    };
                    playerDir = Direction::Left;
                    break;
                }
                default:
                    std::cerr << "invalid direction for edge walk" << std::endl;
                    std::terminate();
                    break;
            }
            break;
        }
        case CubeFace::C: {
            switch (playerDir) {
                case Direction::Left: {
                    // Top D
                    playerCoord = {
                        (playerCoord.y() % width) + (width * 0),
                        (width * 2),
                    };
                    playerDir = Direction::Down;
                    break;
                }
                case Direction::Right: {
                    // Bottom B
                    playerCoord = {
                        (playerCoord.y() % width) + (width * 2),
                        (width * 1) - 1,
                    };
                    playerDir = Direction::Up;
                    break;
                }
                default:
                    std::cerr << "invalid direction for edge walk" << std::endl;
                    std::terminate();
                    break;
            }
            break;
        }
        case CubeFace::D: {
            switch (playerDir) {
                case Direction::Up: {
                    // Left C
                    playerCoord = {
                        (width * 1),
                        (playerCoord.x() % width) + (width * 1),
                    };
                    playerDir = Direction::Right;
                    break;
                }
                case Direction::Left: {
                    // Left A
                    playerCoord = {
                        (width * 1),
                        (-1 * (playerCoord.y() % width) + (width - 1)) + (width * 0),
                    };
                    playerDir = Direction::Right;
                    break;
                }
                default:
                    std::cerr << "invalid direction for edge walk" << std::endl;
                    std::terminate();
                    break;
            }
            break;
        }
        case CubeFace::E: {
            switch (playerDir) {
                case Direction::Down: {
                    // Right F
                    playerCoord = {
                        (width * 1) - 1,
                        (playerCoord.x() % width) + (width * 3),
                    };
                    playerDir = Direction::Left;
                    break;
                }
                case Direction::Right: {
                    // Right B
                    playerCoord = {
                        (width * 3) - 1,
                        (-1 * (playerCoord.y() % width) + (width - 1)) + (width * 0),
                    };
                    playerDir = Direction::Left;
                    break;
                }
                default:
                    std::cerr << "invalid direction for edge walk" << std::endl;
                    std::terminate();
                    break;
            }
            break;
        }
        case CubeFace::F: {
            switch (playerDir) {
                case Direction::Down: {
                    // Top B
                    playerCoord = {
                        (playerCoord.x() % width) + (width * 2),
                        0,
                    };
                    // No direction change.
                    break;
                }
                case Direction::Left: {
                    // Top A
                    playerCoord = {
                        (playerCoord.y() % width) + (width * 1),
                        0,
                    };
                    playerDir = Direction::Down;
                    break;
                }
                case Direction::Right: {
                    // Bottom E
                    playerCoord = {
                        (playerCoord.y() % width) + (width * 1),
                        (width * 3) - 1,
                    };
                    playerDir = Direction::Up;
                    break;
                }
                default:
                    std::cerr << "invalid direction for edge walk" << std::endl;
                    std::terminate();
                    break;
            }
            break;
        }
        default:
            std::cerr << "invalid cube face for edge walk" << std::endl;
            std::terminate();
            break;
    }

    return {playerCoord, playerDir};
}

Position playGame(const BoardMap & boardMap, const PathDirections & pathDirections)
{
    CoordinatePair playerCoord{boardMap.start()};
    Direction playerDirection{Direction::Right};

    for (const Step & step : pathDirections) {
        if (std::holds_alternative<Int>(step)) {
            Int numMoves{std::get<Int>(step)};

            for (int moveIndex = 0; moveIndex < numMoves; ++moveIndex) {
                CoordinatePair nextCoord{playerCoord + directionToForceVector(playerDirection)};

                if (boardMap.isWall(nextCoord)) {
                    break;
                }
                else if (boardMap.isTile(nextCoord)) {
                    playerCoord = nextCoord;
                }
                else {
                    Direction nextDirection;
                    std::tie(nextCoord, nextDirection) = 
                        edgeWalk({playerCoord, playerDirection}, boardMap);

                    if (boardMap.isWall(nextCoord)) {
                        break;
                    }

                    playerCoord = nextCoord;
                    playerDirection = nextDirection;
                }
            }
        }
        else if (std::holds_alternative<Turn>(step)) {
            Turn turn{std::get<Turn>(step)};
            playerDirection = turnDirection(playerDirection, turn);
        }
        else {
            std::cerr << "invalid step" << std::endl;
            std::terminate();
        }
    }

    return {playerCoord, playerDirection};
}

Int score(const Position & position)
{
    CoordinatePair coords{position.first};
    Direction direction{position.second};

    return ((1000 * (coords.y() + 1)) +
            (4 * (coords.x() + 1)) +
            directionToScore(direction));
}

int main()
{
    BoardMap boardMap{std::cin};
    PathDirections pathDirections{std::cin};
    std::cout << score(playGame(boardMap, pathDirections)) << std::endl;
    return 0;
}

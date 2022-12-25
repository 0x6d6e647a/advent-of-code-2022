#include <algorithm>
#include <array>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using Int = std::int64_t;
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

std::ostream & operator<<(std::ostream & os, const CoordinatePair & coordinatePair)
{
    os << coordinatePair.toString();
    return os;
}

template<>
struct std::hash<CoordinatePair>
{
    std::size_t operator()(const CoordinatePair & coordinatePair)
        const
    {
        return std::hash<String>{}(coordinatePair.toString());
    }
};

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

enum class MovementDirection { Left, Right, Down };

std::vector<MovementDirection> parseJetDirections(std::istream & is)
{
    String line;

    if (!std::getline(is, line)) {
        std::cerr << "Unable to read line from input stream" << std::endl;
    }

    std::vector<MovementDirection> directions;

    for (const auto & c : line) {
        if (c == '<') {
            directions.push_back(MovementDirection::Left);
            continue;
        }

        if (c == '>') {
            directions.push_back(MovementDirection::Right);
            continue;
        }

        std::cerr << "Unable to form jet direction from character: " << c << std::endl;
        std::terminate();
    }

    return directions;
}

class ForceVector
    : public CoordinatePair
{
public:
    explicit ForceVector(MovementDirection direction)
        : CoordinatePair(0, 0)
    {
        switch (direction) {
        case MovementDirection::Left: {
            first = -1;
            second = 0;
            break;
        }
        case MovementDirection::Right: {
            first = 1;
            second = 0;
            break;
        }
        case MovementDirection::Down: {
            first = 0;
            second = -1;
            break;
        }
        }
    }
};

enum class RockType { Horizontal, Plus, BackwardsL, Vertical, Square };

std::int8_t numNodes(RockType rockType)
{
    switch (rockType) {
    case RockType::Horizontal:
    case RockType::Square:
    case RockType::Vertical:
        return 4;
    case RockType::Plus:
    case RockType::BackwardsL:
        return 5;
    }

    std::cerr << "Unable to determine number of nodes in rock of unknown type" << std::endl;
    std::terminate();
}

class Rock
{
private:
    using ArraySize = std::int8_t;

    static constexpr ArraySize arraySize = 5;

    RockType _rockType;
    ArraySize _numCoords;
    std::array<CoordinatePair, arraySize> _coords;

    static constexpr Int vertDisplacment = 3;

public:
    Rock()
        : _rockType{std::numeric_limits<ArraySize>::max()}
        , _numCoords(0)
    {}

    explicit Rock(RockType rockType, Int yOffset)
        : _rockType(rockType)
        , _numCoords(numNodes(_rockType))
    {
        switch(rockType) {
        case RockType::Horizontal: {
            // 0123
            _coords[0] = {2, yOffset + vertDisplacment};
            _coords[1] = {3, yOffset + vertDisplacment};
            _coords[2] = {4, yOffset + vertDisplacment};
            _coords[3] = {5, yOffset + vertDisplacment};
            break;
        }
        case RockType::Plus: {
            //  4
            // 123
            //  0
            _coords[0] = {3, yOffset + vertDisplacment + 0};
            _coords[1] = {2, yOffset + vertDisplacment + 1};
            _coords[2] = {3, yOffset + vertDisplacment + 1};
            _coords[3] = {4, yOffset + vertDisplacment + 1};
            _coords[4] = {3, yOffset + vertDisplacment + 2};
            break;
        }
        case RockType::BackwardsL: {
            //   4
            //   3
            // 012
            _coords[0] = {2, yOffset + vertDisplacment + 0};
            _coords[1] = {3, yOffset + vertDisplacment + 0};
            _coords[2] = {4, yOffset + vertDisplacment + 0};
            _coords[3] = {4, yOffset + vertDisplacment + 1};
            _coords[4] = {4, yOffset + vertDisplacment + 2};
            break;
        }
        case RockType::Vertical: {
            // 3
            // 2
            // 1
            // 0
            _coords[0] = {2, yOffset + vertDisplacment + 0};
            _coords[1] = {2, yOffset + vertDisplacment + 1};
            _coords[2] = {2, yOffset + vertDisplacment + 2};
            _coords[3] = {2, yOffset + vertDisplacment + 3};
            break;
        }
        case RockType::Square: {
            // 23
            // 01
            _coords[0] = {2, yOffset + vertDisplacment + 0};
            _coords[1] = {3, yOffset + vertDisplacment + 0};
            _coords[2] = {2, yOffset + vertDisplacment + 1};
            _coords[3] = {3, yOffset + vertDisplacment + 1};
            break;
        }
        }
    }

    auto begin() { return _coords.begin(); }
    auto end() { return std::prev(_coords.end(), arraySize - _numCoords); }
    auto cbegin() const { return _coords.cbegin(); }
    auto cend() const { return std::prev(_coords.cend(), arraySize - _numCoords); }

    Rock apply(const ForceVector & forceVector)
        const
    {
        Rock newRock{*this};

        for (int i = 0; i < newRock._numCoords; ++i) {
            newRock._coords[i] = newRock._coords[i] + forceVector;
        }

        return newRock;
    }
};

class Chamber
{
private:
    CoordinateCollection _rocksAtRest;
    std::vector<MovementDirection> _jetDirections;
    decltype(_jetDirections)::iterator _jetDirectionIter;
    std::vector<RockType> _rockTypes;
    decltype(_rockTypes)::iterator _rockTypesIter;
    Int _highestRockY;

    static constexpr Int numRocksToDrop = 1'000'000'000'000;
    static constexpr Int leftWallX = -1;
    static constexpr Int rightWallX = 7;
    static constexpr Int floorY = -1;

public:
    explicit Chamber(std::istream & is)
        : _jetDirections(parseJetDirections(is))
        , _jetDirectionIter(_jetDirections.begin())
        , _rockTypes({
                RockType::Horizontal,
                RockType::Plus,
                RockType::BackwardsL,
                RockType::Vertical,
                RockType::Square})
        , _rockTypesIter(_rockTypes.begin())
        , _highestRockY(0)
    {
        playGame();
    }

    auto highestRockY() const { return _highestRockY; }

    Int getJetDirectionIndex()
    {
        return std::distance(_jetDirections.begin(), _jetDirectionIter);
    }

    MovementDirection getNextJetDirection()
    {
        auto ret = *_jetDirectionIter;
        ++_jetDirectionIter;

        if (_jetDirectionIter == _jetDirections.end()) {
            _jetDirectionIter = _jetDirections.begin();
        }

        return ret;
    }

    Int getRockTypeIndex()
    {
        return std::distance(_rockTypes.begin(), _rockTypesIter);
    }

    RockType getNextRockType()
    {
        auto ret = *_rockTypesIter;
        ++_rockTypesIter;

        if (_rockTypesIter == _rockTypes.end()) {
            _rockTypesIter = _rockTypes.begin();
        }

        return ret;
    }

    Rock spawnRock()
    {
        return Rock(getNextRockType(), _highestRockY);
    }

    bool moveRock(Rock & rock, const MovementDirection & direction)
    {
        Rock newRock = rock.apply(ForceVector(direction));

        for (const auto & rockPiece : newRock) {
            // Check for collision with walls.
            if ((rockPiece.x() <= leftWallX) ||
                (rockPiece.x() >= rightWallX)) {
                return false;
            }

            // Check for collision with floor.
            if (rockPiece.y() <= floorY) {
                return false;
            }

            // Check for collision with resting rocks.
            if (_rocksAtRest.contains(rockPiece)) {
                return false;
            }
        }

        rock = newRock;
        return true;
    }

    void playGame()
    {
        static constexpr Int checkModulo = 500;

        std::unordered_map<String, Int> keys;
        String repeatKey;
        Int rockIndex;
        std::unordered_map<Int, Int> heights;

        for (rockIndex = 0; rockIndex < numRocksToDrop; ++rockIndex) {
            // Print status.
            if (rockIndex % checkModulo == 0) {
#ifdef DEBUG
                std::cout << "@ round " << rockIndex << std::endl;
#endif // DEBUG
            }

            bool moved = true;
            Rock rock{spawnRock()};

            while (moved) {
                // Move rock with jet of hot gas.
                moveRock(rock, getNextJetDirection());

                // Fall down.
                moved = moveRock(rock, MovementDirection::Down);

                if (!moved) {
                    // Find new highest rock Y.
                    for (const auto & coord : rock) {
                        if (coord.y() + 1 > _highestRockY) {
                            // Update highest rock.
                            _highestRockY = coord.y() + 1;
                        }

                        // Put rock to rest.
                        _rocksAtRest.insert(coord);
                    }
                }
            }

#ifdef DEBUG
            // Print display of the game state.
            if (rockIndex % checkModulo == 0) {
                printChamber(rock);
            }
#endif // DEBUG

            heights[rockIndex] = _highestRockY;

            // Check for pattern.
            static constexpr Int lookback = 10;

            if (highestRockY() > lookback) {
                // Create a "hash" of the current state and top of the chamber.
                std::stringstream ss;
                ss << getRockTypeIndex() << "~"
                   << getJetDirectionIndex() << "~";

                for (Int y = _highestRockY; y > _highestRockY - lookback; --y) {
                    for (Int x = leftWallX + 1; x < rightWallX; ++x) {
                        CoordinatePair coord{x, y};
                        if (_rocksAtRest.contains(coord)) {
                            ss << "1";
                        }
                        else {
                            ss << "0";
                        }
                    }
                }

                // See if "hash" repeats, confirming pattern found.
                String key{ss.str()};

                if (keys.find(key) == keys.end()) {
                    keys[key] = rockIndex;
                }
                else {
                    repeatKey = key;
                    std::cout << "Sequence repeats after " << rockIndex << " rocks!" << std::endl;
                    break;
                }
            }
        }

        // Use discovered to calculate the final value.
        Int start = keys[repeatKey];
        Int diffRock = rockIndex - start;
        Int diffHeigth = _highestRockY - heights[start];
        Int repeats = (numRocksToDrop - start) / diffRock;
        Int total = heights[numRocksToDrop - diffRock * repeats - 1] + repeats * diffHeigth;
        _highestRockY = total;
    }

#ifdef DEBUG
    void printChamber(Rock rock)
    {
        CoordinateCollection rockPieces;
        std::for_each(
            rock.begin(),
            rock.end(),
            [&rockPieces](const auto & rockPiece) {
                rockPieces.insert(rockPiece);
            });

        Int highestRockYPos = std::max_element(
            rock.begin(),
            rock.end(),
            [](const auto & a, const auto & b) {
                return a.y() < b.y();
            })->y();

        Int lowestRockY = std::min_element(
            rock.begin(),
            rock.end(),
            [](const auto & a, const auto & b) {
                return a.y() < b.y();
            })->y();

        Int bottomY = (lowestRockY - 3 < floorY) ? floorY : lowestRockY - 3;

        for (Int y = highestRockYPos + 1; y >= bottomY; --y) {
            std::ios::fmtflags oldFmtFlags = std::cout.flags();

            std::cout
                << std::setw(8) << y << " "
                << std::resetiosflags(oldFmtFlags);

            for (Int x = leftWallX; x <= rightWallX; ++x) {
                if ((x == leftWallX) || (x == rightWallX)) {
                    if (y == floorY) {
                        std::cout << '+';
                    }
                    else {
                        std::cout << '|';
                    }
                }
                else if (y == floorY) {
                    std::cout << '-';
                }
                else if (rockPieces.contains({x, y})) {
                    std::cout << '@';
                }
                else if (_rocksAtRest.contains({x, y})) {
                    std::cout << '#';
                }
                else {
                    std::cout << '.';
                }
            }

            std::cout << std::endl;
        }
    }
#endif // DEBUG
};

int main()
{
    Chamber chamber(std::cin);
    std::cout << chamber.highestRockY() << std::endl;
    return 0;
}

#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

enum class Direction { up, down, left, right };

#ifdef DEBUG
std::ostream & operator<<(std::ostream & os, const Direction & direction)
{
    switch(direction) {
    case Direction::up:
        os << "Up";
        break;
    case Direction::down:
        os << "Down";
        break;
    case Direction::left:
        os << "Left";
        break;
    case Direction::right:
        os << "Right";
        break;
    }

    return os;
}
#endif

using Distance = long long;
using Motion = std::pair<Direction, Distance>;
using CoordinatePair = std::pair<Distance, Distance>;

class Position
{
private:
    Distance _x, _y;

public:
    Position()
        : _x(0)
        , _y(0)
    {}

    ~Position() = default;

    auto x() const { return _x; }
    auto y() const { return _y; }

    bool operator==(const Position & other)
        const
    {
        return ((_x == other._x) && (_y == other._y));
    }

    CoordinatePair getCoordinatePair()
        const
    {
        return std::make_pair(_x, _y);
    }

    bool touching(const Position & other)
        const
    {
        if ((_x == other._x - 1) ||
            (_x == other._x    ) ||
            (_x == other._x + 1)) {
            if ((_y == other._y - 1) ||
                (_y == other._y    ) ||
                (_y == other._y + 1)) {
                return true;
            }
        }

        return false;
    }

    bool sameRowOrCol(const Position & other)
    {
        return ((_x == other._x) || (_y == other._y));
    }

    void applyDirection(const Direction & direction)
    {
        switch(direction) {
        case Direction::up:
            ++_y;
            break;
        case Direction::down:
            --_y;
            break;
        case Direction::left:
            --_x;
            break;
        case Direction::right:
            ++_x;
            break;
        }
    }

    void reconnect(const Position & other)
    {
        for (const auto & direction : {
                Direction:: up,
                Direction::down,
                Direction::left,
                Direction::right,
            }) {
            Position candidate = *this;
            candidate.applyDirection(direction);

            if (candidate.touching(other)) {
                applyDirection(direction);
                return;
            }
        }

        std::cerr << "unable to reconnect" << std::endl;
        std::terminate();
    }

    void reconnectDiag(const Position & other)
    {
        for (const auto & directionA : { Direction::up, Direction::down }) {
            for (const auto & directionB : { Direction::left, Direction::right}) {
                Position candidate = *this;
                candidate.applyDirection(directionA);
                candidate.applyDirection(directionB);

                if (candidate.touching(other)) {
                    applyDirection(directionA);
                    applyDirection(directionB);
                    return;
                }
            }
        }

        std::cerr << "unable to reconnect diagonally" << std::endl;
        std::terminate();
    }
};

class Rope
{
private:
    std::vector<Position> _knots;
    std::map<CoordinatePair, Distance> _tailPositions;
public:
    explicit Rope(int size)
        : _knots(size)
    {
        recordTail();
    }

    ~Rope() = default;

    Position & head() { return _knots.front(); }
    Position & tail() { return _knots.back();  }

    void recordTail()
    {
        ++_tailPositions[tail().getCoordinatePair()];
    }

    void applyMotion(Motion motion)
    {
        auto direction = motion.first;
        auto steps = motion.second;

        for (decltype(steps) i = 0; i < steps; ++i) {
            head().applyDirection(direction);

            for (auto curr = std::next(_knots.begin());
                 curr != _knots.end();
                 ++curr) {
                auto prev = std::prev(curr);

                if (!curr->touching(*prev)) {
                    if (curr->sameRowOrCol(*prev)) {
                        curr->reconnect(*prev);
                    }
                    else {
                        curr->reconnectDiag(*prev);
                    }
                }
            }

            recordTail();
        }
    }

    std::set<CoordinatePair> tailPositions()
        const
    {
        std::set<CoordinatePair> keys;

        for (const auto & [key, _] : _tailPositions) {
            keys.insert(key);
        }

        return keys;
    }

#ifdef DEBUG
    void printRope()
        const
    {
        // Determine display boundaries.
        Distance highestX = std::numeric_limits<Distance>::min();
        Distance highestY = std::numeric_limits<Distance>::min();
        Distance lowestX = std::numeric_limits<Distance>::max();
        Distance lowestY = std::numeric_limits<Distance>::max();

        for (const auto & position : _knots) {
            auto x = position.x();
            auto y = position.y();

            if (x > highestX) {
                highestX = x;
            }

            if (x < lowestX) {
                lowestX = x;
            }

            if (y > highestY) {
                highestY = y;
            }

            if (y < lowestY) {
                lowestY = y;
            }
        }

        for (const auto & [coordPair, distance] : _tailPositions) {
            auto x = coordPair.first;
            auto y = coordPair.second;

            if (x > highestX) {
                highestX = x;
            }

            if (x < lowestX) {
                lowestX = x;
            }

            if (y > highestY) {
                highestY = y;
            }

            if (y < lowestY) {
                lowestY = y;
            }
        }

        // Print display.
        for (auto y = highestY + 1; y >= lowestY - 1; --y) {
            for (auto x = lowestX - 1; x < highestX + 1; ++x) {
                // Display index on rope
                bool knotFound = false;

                for (decltype(_knots)::size_type i = 0; i < _knots.size(); ++i) {
                    auto knot = _knots[i];

                    if ((knot.x() == x) && (knot.y() == y)) {
                        if (i == 0) {
                            std::cout << 'H';
                        }
                        else {
                            std::cout << i;
                        }

                        knotFound = true;
                        break;
                    }
                }

                if (knotFound) {
                    continue;
                }

                // Display starting position.
                if ((x == 0) && (y == 0)) {
                    std::cout << 's';
                    continue;
                }
                
                // Display previous tail positions.
                bool tailFound = false;

                for (const auto & [coordPair, _] : _tailPositions) {
                    if ((coordPair.first == x) && (coordPair.second == y)) {
                        tailFound = true;
                        std::cout << '#';
                        break;
                    }
                }

                if (tailFound) {
                    continue;
                }

                // Generic position.
                std::cout << '.';
            }

            std::cout << std::endl;
        }
    }
#endif
};

Direction parseDirection(const std::string & line)
{
    if ((line.length() != 1) || (std::islower(line[0]))) {
        std::cerr << "badly formatted direction: " << line << std::endl;
        std::terminate();
    }

    switch(line[0]) {
    case 'U':
        return Direction::up;
    case 'D':
        return Direction::down;
    case 'L':
        return Direction::left;
    case 'R':
        return Direction::right;
    }

    std::cerr << "badly formatted direction: " << line << std::endl;
    std::terminate();
}

std::vector<Motion> parseInput(std::istream & is)
{
    std::vector<Motion> motions;

    for (std::string line; std::getline(is, line);) {
        std::istringstream ss{line};
        std::string direction;
        std::string steps;
        ss >> direction >> steps;

        motions.push_back(std::make_pair(parseDirection(direction),
                                         std::stoi(steps)));
    }

    return motions;
}

int main()
{
    Rope rope{10};
    auto motions{parseInput(std::cin)};

    std::for_each(
        motions.begin(),
        motions.end(),
        [&rope](auto motion){ rope.applyMotion(motion); });

#ifdef DEBUG
    rope.printRope();
#endif

    std::cout << rope.tailPositions().size() << std::endl;

    return 0;
}

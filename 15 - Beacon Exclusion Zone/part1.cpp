#include <cmath>
#include <iostream>
#include <limits>
#include <optional>
#include <regex>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

using Int = std::int64_t;

class CoordinatePair
    : public std::pair<Int, Int>
{
private:
    using Parent = std::pair<Int, Int>;

public:
    CoordinatePair(Int x, Int y)
        : Parent(x, y)
    {}

    auto x() const { return first; }
    auto y() const { return second; }

    auto manhattanDistance(const CoordinatePair & other)
        const
    {
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

class HasCoordinates
{
private:
    CoordinatePair _coordinates;

public:
    HasCoordinates(Int x, Int y)
        : _coordinates(x, y)
    {}

    explicit HasCoordinates(const CoordinatePair & coordinates)
        : _coordinates(coordinates.x(), coordinates.y())
    {}

    auto coordinates() const { return _coordinates; }
    auto x() const { return _coordinates.x(); }
    auto y() const { return _coordinates.y(); }
};

class Beacon
    : public HasCoordinates
{
public:
    explicit Beacon(const CoordinatePair & coordinates)
        : HasCoordinates(coordinates)
    {}
};

using BeaconPtr = std::shared_ptr<Beacon>;

class NumericRange
    : public std::pair<Int, Int>
{
private:
    using Parent = std::pair<Int, Int>;

public:
    NumericRange(Int low, Int high)
        : Parent(low, high)
    {
        if (low > high) {
            std::swap(first, second);
        }
    }

    auto low() const { return first; }
    auto high() const { return second; }

    bool inRange(Int i)
        const
    {
        return ((i >= low()) &&
                (i <= high()));
    }

    class NumericRangeGenerator
    {
    private:
        Int _next;
        Int _high;

    public:
        NumericRangeGenerator(Int low, Int high)
            : _next(low)
            , _high(high)
        {}

        std::optional<Int> operator()()
        {
            if (_next > _high) {
                return {};
            }

            auto ret = _next;

            ++_next;

            return {ret};
        }
    };

    auto generator() const { return NumericRangeGenerator(low(), high()); }
};

class Sensor
    : public HasCoordinates
{
private:
    BeaconPtr _beaconPtr;

public:
    Sensor()
        : HasCoordinates(std::numeric_limits<Int>::max(),
                         std::numeric_limits<Int>::max())
        , _beaconPtr(nullptr)
    {}
    
    Sensor(const CoordinatePair & sensorCoordinate,
           BeaconPtr beaconPtr)
        : HasCoordinates(sensorCoordinate)
        , _beaconPtr(beaconPtr)
    {}

    auto beaconPtr() const { return _beaconPtr; };

    auto manhattanDistance()
        const
    {
        return coordinates()
            .manhattanDistance(_beaconPtr->coordinates());
    }

    std::optional<NumericRange> getSensorRange(Int targetY)
        const
    {
        auto distance{manhattanDistance()};
        auto yVal{y()};

        // Check if target y is even in range.
        NumericRange yRange{yVal - distance, yVal + distance};

        if (!yRange.inRange(targetY)) {
            return {};
        }

        // Determine range for target y.
        auto xVal{x()};
        auto delta = distance - std::abs(targetY - yVal);

        return {NumericRange(xVal - delta, xVal + delta)};
    }
};

class CaveMap
{
private:
    std::unordered_map<CoordinatePair, Sensor> _sensorMap;
    std::unordered_map<CoordinatePair, BeaconPtr> _beaconMap;

public:
    explicit CaveMap(std::istream & is)
    {
        static const std::regex regex{"Sensor at x=(-?\\d+), y=(-?\\d+): closest beacon is at x=(-?\\d+), y=(-?\\d+)$"};
        std::smatch matches;

        for (std::string line; std::getline(is, line);) {
            if (!std::regex_search(line, matches, regex)) {
                std::cerr << "Parsing regex failed on line: " << line << std::endl;
                std::terminate();
            }

            CoordinatePair sensorCoords{std::stoi(matches[1]), std::stoi(matches[2])};
            CoordinatePair beaconCoords{std::stoi(matches[3]), std::stoi(matches[4])};

            auto beaconPtr{std::make_shared<Beacon>(beaconCoords)};

            _sensorMap[sensorCoords] = Sensor{sensorCoords, beaconPtr};
            _beaconMap[beaconCoords] = beaconPtr;
        }
    }

    Int score(Int targetY)
    {
        // Find beacons on line.
        std::unordered_set<Int> beaconX;

        for (const auto & [coord, beaconPtr] : _beaconMap) {
            if (coord.y() == targetY) {
                beaconX.insert(coord.x());
            }
        }

        // Find non-beacon positions.
        std::unordered_set<Int> nonBeaconX;

        for (const auto & [coord, sensor] : _sensorMap) {
            auto sensorRange = sensor.getSensorRange(targetY);

            if (!sensorRange) {
                continue;
            }

            auto generator = sensorRange->generator();
            std::optional<Int> ret;

            while ((ret = generator())) {
                if (beaconX.contains(*ret)) {
                    continue;
                }

                nonBeaconX.insert(*ret);
            }
        }

        return nonBeaconX.size();
    }

#ifdef DEBUG
    void printCaveMap()
    {
        // Determine drawing boundaries and gather sensor and beacon coordinates.
        std::unordered_set<CoordinatePair> sensorCoords;
        std::unordered_set<CoordinatePair> beaconCoords;

        Int mostUp    = std::numeric_limits<Int>::max();
        Int mostDown  = std::numeric_limits<Int>::min();
        Int mostLeft  = std::numeric_limits<Int>::max();
        Int mostRight = std::numeric_limits<Int>::min();

        for (const auto & [sensorCoord, sensor] : _sensorMap) {
            auto beaconCoord{sensor.beaconPtr()->coordinates()};

            std::cout
                << "S(" << sensor.coordinates().x() << "," << sensor.coordinates().y() << ")"
                << " -> "
                << "B(" << sensor.beaconPtr()->coordinates().x() << "," << sensor.beaconPtr()->coordinates().y() << ")"
                << " = "
                << sensor.manhattanDistance()
                << std::endl;

            sensorCoords.insert(sensorCoord);
            beaconCoords.insert(beaconCoord);

            auto sensorX = sensorCoord.x();
            auto sensorY = sensorCoord.y();
            auto beaconX = beaconCoord.x();
            auto beaconY = beaconCoord.y();

            // Up
            if (sensorY < mostUp) {
                mostUp = sensorY;
            }

            if (beaconY < mostUp) {
                mostUp = beaconY;
            }

            // Down
            if (sensorY > mostDown) {
                mostDown = sensorY;
            }

            if (beaconY > mostDown) {
                mostDown = beaconY;
            }

            // Left
            if (sensorX < mostLeft) {
                mostLeft = sensorX;
            }

            if (beaconX < mostLeft) {
                mostLeft = beaconX;
            }

            // Right
            if (sensorX > mostRight) {
                mostRight = sensorX;
            }

            if (beaconX > mostRight) {
                mostRight = beaconX;
            }
        }

        // Print cave in boundaries.
        std::cout
            << std::string(80, '~') << std::endl
            << "(4th quadrant)" << std::endl
            << "x: " << mostLeft << " -> " << mostRight << std::endl
            << "y: " << mostUp   << " -> " << mostDown  << std::endl
            << "# sensors: " << sensorCoords.size() << std::endl
            << "# beacons: " << beaconCoords.size() << std::endl
            << std::string(80, '~') << std::endl;

        for (Int y = mostUp - 5; y <= mostDown + 5; ++y) {
            for (Int x = mostLeft - 5; x <= mostRight + 5; ++x) {
                CoordinatePair coords{x, y};

                if (sensorCoords.contains(coords)) {
                    std::cout << 'S';
                }
                else if (beaconCoords.contains(coords)) {
                    std::cout << 'B';
                }
                else {
                    std::cout << '.';
                }
            }
            std::cout << std::endl;
        }

        std::cout << std::string(80, '~') << std::endl;
    }
#endif // DEBUG
};

int main()
{
    CaveMap caveMap{std::cin};

#ifdef DEBUG
    caveMap.printCaveMap();
#endif // DEBUG

    for (Int testY : {10, 2000000}) {
        std::cout
            << testY << ": " << caveMap.score(testY)
            << std::endl;
    }

    return 0;
}

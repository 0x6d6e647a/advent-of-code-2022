#include <any>
#include <cmath>
#include <iostream>
#include <iterator>
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

    auto tuningFreq()
        const
    {
        return (x() * 4000000) + y();
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

template<typename T>
class BoundingBox
{
private:
    Int _up, _down, _left, _right;

public:
    BoundingBox(Int up, Int down, Int left, Int right)
        : _up(up)
        , _down(down)
        , _left(left)
        , _right(right)
    {}

    BoundingBox(typename std::unordered_map<CoordinatePair, T>::const_iterator begin,
                typename std::unordered_map<CoordinatePair, T>::const_iterator end)
        : _up   (std::numeric_limits<Int>::max())
        , _down (std::numeric_limits<Int>::min())
        , _left (std::numeric_limits<Int>::max())
        , _right(std::numeric_limits<Int>::min())
    {
        for (auto iter = begin; iter != end; ++iter) {
            auto x = iter->first.x();
            auto y = iter->first.y();

            if (y < _up) {
                _up = y;
            }

            if (y > _down) {
                _down = y;
            }

            if (x < _left) {
                _left = x;
            }

            if (x > _right) {
                _right = x;
            }
        }
    }

    auto    up() const { return _up;    }
    auto  down() const { return _down;  }
    auto  left() const { return _left;  }
    auto right() const { return _right; }

    void expand(Int i)
    {
        _up    -= i;
        _down  += i;
        _left  -= i;
        _right += i;
    }

    bool intersect(const CoordinatePair & coord)
        const
    {
        auto x{coord.x()};
        auto y{coord.y()};

        return ((y <= _up) &&
                (y >= _down) &&
                (x >= _left) &&
                (x <= _right));
    }
};

template<typename A, typename B>
BoundingBox<std::any> superSet(const BoundingBox<A> & a, const BoundingBox<B> & b)
{
    Int up    = (a.up()    < b.up())    ? a.up()    : b.up();
    Int down  = (a.down()  > b.down())  ? a.down()  : b.down();
    Int left  = (a.left()  < b.left())  ? a.left()  : b.left();
    Int right = (a.right() > b.right()) ? a.right() : b.right();

    return {up, down, left, right};
}

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

    std::size_t size()
        const
    {
        return std::abs(low() - high()) + 1;
    }

    bool contains(const NumericRange & other)
        const
    {
        return (other.low() >= low()) && (other.high() <= high());
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

class NumericRangeMerger
{
private:
    std::vector<NumericRange> _ranges;

    void compact()
    {
        std::sort(
            _ranges.begin(),
            _ranges.end(),
            [](auto a, auto b) {
                return a.low() < b.low();
            });

        bool changed = false;

        for (auto curr = _ranges.begin();
             curr != _ranges.end();
             ++curr)
        {
            auto next = std::next(curr);

            if (next == _ranges.end()) {
                break;
            }

            // Current completely contains next.
            if (curr->contains(*next)) {
                _ranges.erase(next, std::next(next));
                changed = true;
            }
            // Next completely contains current.
            else if (next->contains(*curr)) {
                _ranges.erase(curr, next);
                changed = true;
            }
            // Current and next overlap
            else if (next->inRange(curr->high())) {
                NumericRange newRange{curr->low(), next->high()};

                _ranges.erase(curr, std::next(next));
                _ranges.insert(curr, newRange);
                changed = true;
            }
        }

        if (changed) {
            compact();
        }
    }

public:
    void push_back(const NumericRange & nr)
    {
        _ranges.push_back(nr);
        compact();
    }
    
    std::vector<Int> findEmpty(NumericRange testRange)
    {
        // Check if we can skip.
        if (std::all_of(
                _ranges.begin(),
                _ranges.end(),
                [&testRange](auto nr){ return nr.contains(testRange); })) {
            return {};
        }
                                                                                           
        std::vector<Int> results;

        for (std::optional<Int> n = {testRange.low()}; *n < testRange.high(); ++*n) {
            bool found = false;

            for (const auto & numRange : _ranges) {
                if (numRange.inRange(*n)) {
                    found = true;
                    break;
                }
            }

            if (found) {
                continue;
            }

            results.push_back(*n);
        }

        return results;
    }
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

    auto getSearchBounds()
        const
    {
        return BoundingBox<Sensor>{_sensorMap.begin(), _sensorMap.end()};
    }

    Int score()
    {
        auto searchBounds{getSearchBounds()};
        NumericRange xRange{searchBounds.left(), searchBounds.right()};

        // Build y to beacon x's map.
        std::map<Int, std::unordered_set<Int>> beaconYtoX;

        for (int y = searchBounds.up(); y < searchBounds.down(); ++y) {
            for (const auto & [coord, beaconPtr] : _beaconMap) {
                if (searchBounds.intersect(coord)) {
                    beaconYtoX[coord.y()].insert(coord.x());
                }
            }
        }

        // Search every line in search bounds.
        for (Int y = searchBounds.up(); y < searchBounds.down(); ++y) {
            NumericRangeMerger nrm;

            for (const auto & [coord, sensor] : _sensorMap) {
                auto sensorRange = sensor.getSensorRange(y);

                if (!sensorRange) {
                    continue;
                }

                nrm.push_back(*sensor.getSensorRange(y));
            }

            auto nonBeaconX = nrm.findEmpty(xRange);

            if (!nonBeaconX.empty()) {
                if (nonBeaconX.size() == 1) {
                    return CoordinatePair(nonBeaconX[0], y).tuningFreq();
                }

                std::cout << " @ y = " << y << std::endl;

                for (const auto & x : nonBeaconX) {
                    std::cout << " -> x = " << x << std::endl;
                }
            }
        }

        return -1;
    }

#ifdef DEBUG
    void printCaveMap()
    {
        // Determine drawing boundaries and gather sensor and beacon coordinates.
        std::unordered_set<CoordinatePair> sensorCoords;
        std::unordered_set<CoordinatePair> beaconCoords;

        std::transform(
            _sensorMap.begin(),
            _sensorMap.end(),
            std::inserter(sensorCoords, sensorCoords.begin()),
            [](const auto & pair){ return pair.first; });

        std::transform(
            _beaconMap.begin(),
            _beaconMap.end(),
            std::inserter(beaconCoords, beaconCoords.begin()),
            [](const auto & pair){ return pair.first; });

        BoundingBox<Sensor> sensorBounds{_sensorMap.begin(), _sensorMap.end()};
        BoundingBox<BeaconPtr> beaconBounds{_beaconMap.begin(), _beaconMap.end()};
        auto graphBounds = superSet(sensorBounds, beaconBounds);

            
        // Print cave in boundaries.
        std::cout
            << std::string(80, '~') << std::endl
            << "(4th quadrant)" << std::endl
            << "- Sensor Bounds" << std::endl
            << "x: " << sensorBounds.left() << " -> " << sensorBounds.right() << std::endl
            << "y: " << sensorBounds.up()   << " -> " << sensorBounds.down()  << std::endl
            << "- Beacon Bounds" << std::endl
            << "x: " << beaconBounds.left() << " -> " << beaconBounds.right() << std::endl
            << "y: " << beaconBounds.up()   << " -> " << beaconBounds.down()  << std::endl
            << "- Graph Bounds" << std::endl
            << "x: " << graphBounds.left() << " -> " << graphBounds.right() << std::endl
            << "y: " << graphBounds.up()   << " -> " << graphBounds.down()  << std::endl
            << "# sensors: " << sensorCoords.size() << std::endl
            << "# beacons: " << beaconCoords.size() << std::endl
            << std::string(80, '~') << std::endl;

        // Return if graph is huge.
        if ((graphBounds.up()    <= -100) ||
            (graphBounds.down()  >= 100)  ||
            (graphBounds.left()  <= -100) ||
            (graphBounds.right() >= 100)) {
            return;
        }

        // Expand graph for drawing.
        graphBounds.expand(2);

        // Draw beacon range around sensors.
        std::unordered_set<CoordinatePair> beaconRange;

        for (const auto & [coord, sensor] : _sensorMap) {
            for (Int y = graphBounds.up(); y < graphBounds.down(); ++y) {
                auto sensorRange = sensor.getSensorRange(y);

                if (!sensorRange) {
                    continue;
                }

                std::optional<Int> x;
                auto generatorX = sensorRange->generator();

                while ((x = generatorX())) {
                    beaconRange.insert({*x, y});
                }
            }
        }

        // Draw map.
        for (Int y = graphBounds.up(); y <= graphBounds.down(); ++y) {
            for (Int x = graphBounds.left(); x <= graphBounds.right(); ++x) {
                CoordinatePair coords{x, y};

                if (sensorCoords.contains(coords)) {
                    std::cout << 'S';
                }
                else if (beaconCoords.contains(coords)) {
                    std::cout << 'B';
                }
                else if (beaconRange.contains(coords)) {
                    std::cout << '#';
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

    std::cout << caveMap.score() << std::endl;

    return 0;
}

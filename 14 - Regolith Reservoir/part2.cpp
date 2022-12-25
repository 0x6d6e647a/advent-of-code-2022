#include <iostream>
#include <limits>
#include <map>
#include <regex>
#include <vector>

class CoordinatePair
    : public std::pair<int, int>
{
public:
    constexpr CoordinatePair(int x, int y)
        : std::pair<int, int>(x, y)
    {}

    auto x() const { return first; }
    auto y() const { return second; }

    CoordinatePair operator+(const CoordinatePair & other)
    {
        return {first + other.first, second + other.second};
    }
};

class Cave
    : public std::map<CoordinatePair, char>
{
private:
    using Parent = std::map<CoordinatePair, char>;

    int _sandCount;

    static std::vector<CoordinatePair> parseRockLine(std::string line)
    {
        std::vector<CoordinatePair> coordinates;

        // Replace arrow gaps with spaces.
        static const std::regex arrow{" -> "};
        line = std::regex_replace(line, arrow, " ");

        // For each coordinate pair string.
        std::string coordPairStr;
        std::istringstream coordPairSS{line};

        while (std::getline(coordPairSS, coordPairStr, ' ')) {
            std::vector<std::string> values;
            std::string valueStr;
            std::istringstream valueSS{coordPairStr};

            while (std::getline(valueSS, valueStr, ',')) {
                values.push_back(valueStr);
            }

            if (values.size() != 2) {
                std::cerr << "encountered non-2D coordinates" << std::endl;
                std::terminate();
            }

            coordinates.push_back({std::stoi(values[0]), std::stoi(values[1])});
        }

        return coordinates;
    }

    static CoordinatePair getDirectionCoordinates(const CoordinatePair & start,
                                                  const CoordinatePair & finish)
    {
        // Determine direction.
        if (start.x() == finish.x()) {
            // Vertical
            auto ordering = start.y() <=> finish.y();

            if (ordering < 0) {
                // Down
                return {0, 1};
            }
            else if (ordering > 0) {
                // Up
                return {0, -1};
            }
            else {
                std::cerr << "Unable to determine rock line up vs down" << std::endl;
                std::terminate();
            }
        }
        else if (start.y() == finish.y()) {
            // Vertical
            auto ordering = start.x() <=> finish.x();

            if (ordering < 0) {
                // Right
                return {1, 0};
            }
            else if (ordering > 0) {
                // Left
                return {-1, 0};
            }
            else {
                std::cerr << "Unable to determine rock line up vs down" << std::endl;
                std::terminate();
            }
        }
        else {
            std::cerr << "Unable to determine rock line direction" << std::endl;
            std::terminate();
        }
                 
    }

    void drawRockLine(const CoordinatePair & first,
                      const CoordinatePair & last)
    {
        auto directionCoordinates{getDirectionCoordinates(first, last)};
        CoordinatePair curr = first;

        do {
            (*this)[curr] = '#';
            curr = curr + directionCoordinates;
        } while (curr != last);

        (*this)[curr] = '#';
    }

    void drawRockLine(std::vector<CoordinatePair> rockLinePoints)
    {
        for(auto iter = std::next(rockLinePoints.begin());
            iter != rockLinePoints.end();
            ++iter) {
            drawRockLine(*std::prev(iter), *iter);
        }
    }

    void drawRockLine(const std::string & line)
    {
        drawRockLine(parseRockLine(line));
    }

    int bottomY()
        const
    {
        return std::max_element(
            this->begin(),
            this->end(),
            [](const auto & a, const auto & b) {
                return a.first.y() < b.first.y();
            })->first.y();
    }

    bool canMove(CoordinatePair maybe, int floorY)
    {
        if (maybe.y() == floorY) {
            return false;
        }

        return !(this->contains(maybe));
    }

    bool dropSand(int floorY)
    {
        static constexpr CoordinatePair sandSource{500, 0};
        static constexpr CoordinatePair down{0, 1};
        static constexpr CoordinatePair downLeft{-1, 1};
        static constexpr CoordinatePair downRight{1, 1};

        CoordinatePair sand = sandSource;

        while (true) {
            // Attempt move down.
            auto maybeDown = sand + down;

            if (canMove(maybeDown, floorY)) {
                sand = maybeDown;
                continue;
            }

            // Attempt move down left.
            auto maybeDownLeft = sand + downLeft;

            if (canMove(maybeDownLeft, floorY)) {
                sand = maybeDownLeft;
                continue;
            }

            // Attempt move down right.
            auto maybeDownRight = sand + downRight;

            if (canMove(maybeDownRight, floorY)) {
                sand = maybeDownRight;
                continue;
            }

            // Sand has settled.
            (*this)[sand] = 'o';
            return sand != sandSource;
        }
    }

    void pourSand()
    {
        int floorY{bottomY() + 2};

        while (dropSand(floorY)) {
            ++_sandCount;
        }

        ++_sandCount;
    }

public:
    explicit Cave(std::istream & is)
        : _sandCount(0)
    {
        for (std::string line; std::getline(is, line);) {
            drawRockLine(line);
        }

#ifdef DEBUG
        printCave();
#endif // DEBUG

        pourSand();
    }

    auto sandCount() const { return _sandCount; }

#ifdef DEBUG
    void printCave()
    {
        // Determine drawing boundaries.
        int mostUp    = std::numeric_limits<int>::max();
        int mostDown  = std::numeric_limits<int>::min();
        int mostLeft  = std::numeric_limits<int>::max();
        int mostRight = std::numeric_limits<int>::min();

        for (const auto & [coords, c] : *this) {
            if (coords.y() < mostUp) {
                mostUp = coords.y();
            }

            if (coords.y() > mostDown) {
                mostDown = coords.y();
            }

            if (coords.x() < mostLeft) {
                mostLeft = coords.x();
            }

            if (coords.x() > mostRight) {
                mostRight = coords.x();
            }
        }

        // Print cave in boundaries.
        std::cout
            << std::string(80, '~') << std::endl
            << "(4th quadrant)" << std::endl
            << "x: " << mostLeft << " -> " << mostRight << std::endl
            << "y: " << mostUp   << " -> " << mostDown  << std::endl
            << std::string(80, '~') << std::endl;

        for (int y = mostUp - 1; y <= mostDown; ++y) {
            for (int x = mostLeft - 1; x <= mostRight; ++x) {
                if (this->contains({x, y})) {
                    std::cout << (*this)[{x, y}];
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
    Cave cave{std::cin};
    std::cout << cave.sandCount() << std::endl;
#ifdef DEBUG
    cave.printCave();
#endif // DEBUG
    return 0;
}

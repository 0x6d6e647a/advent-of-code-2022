#include <iostream>
#include <regex>
#include <string>

class Range
{
private:
    int _first, _last;

public:
    Range(int first, int last)
        : _first(first)
        , _last(last)
    {}

    ~Range() = default;

    bool inRange(int num) const
    {
        return (num >= _first) && (num <= _last);
    }

    bool fullyContains(const Range & other) const
    {
        return ((inRange(other._first)) && (inRange(other._last)));
    }

#ifdef DEBUG
private:
    friend std::ostream & operator<<(std::ostream & os, const Range & range);
#endif
};

#ifdef DEBUG
std::ostream & operator<<(std::ostream & os, const Range & range) {
    return os << "Range(" << range._first << "," << range._last << ")";
}
#endif

int main()
{
    int numFullyContain = 0;

    for (std::string line; std::getline(std::cin, line);) {
        const std::regex regex("([0-9]+)-([0-9]+),([0-9]+)-([0-9]+)");
        std::smatch matches;

        if (std::regex_search(line, matches, regex)) {
            const Range rangeA(std::stoi(matches[1]), std::stoi(matches[2]));
            const Range rangeB(std::stoi(matches[3]), std::stoi(matches[4]));

#ifdef DEBUG
            std::cout
                << rangeA << " & " << rangeB << std::endl
                << ((rangeA.fullyContains(rangeB)) ? "YES" : "NO") << ", "
                << ((rangeB.fullyContains(rangeA)) ? "YES" : "NO") << std::endl
                << std::string(80, '-') << std::endl;
#endif

            if ((rangeA.fullyContains(rangeB)) ||
                (rangeB.fullyContains(rangeA))) {
                ++numFullyContain;
            }
        }
        else {
            std::cerr << "error parsing line: " << line << std::endl;
            return 1;
        }
    }

    std::cout << numFullyContain << std::endl;
    
    return 0;
}

#include <algorithm>
#include <compare>
#include <iostream>
#include <numeric>
#include <regex>
#include <string>
#include <vector>

class Elf
{
public:
    Elf() = default;
    ~Elf() = default;

    int get_calories() const {
        return std::accumulate(_snacks.begin(), _snacks.end(), 0);
    }

    void add_snack(const int snack) {
        _snacks.push_back(snack);
    }

    static bool cmp(const Elf & a, const Elf & b) {
        auto cmp =  a.get_calories() <=> b.get_calories();
        return cmp < 0;
    }
private:
    std::vector<int> _snacks;
};

std::vector<Elf> parse_elves()
{
    std::vector<Elf> elves;

    Elf curr;
    
    for (std::string line; std::getline(std::cin, line);) {
        if ((line.empty()) || (std::regex_match(line, std::regex("^\\s+$")))) {
            elves.push_back(curr);
            curr = Elf();
            continue;
        }

        curr.add_snack(std::stoi(line));
    }

    elves.push_back(curr);

    return elves;
}

int main()
{
    auto elves = parse_elves();

    std::sort(elves.begin(), elves.end(), Elf::cmp);

    std::cout
        << std::accumulate(
            elves.rbegin(),
            std::next(elves.rbegin(), 3),
            0,
            [](int t, const Elf & b){ return t + b.get_calories(); })
        << std::endl;

    return 0;
}

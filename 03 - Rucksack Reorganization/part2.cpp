#include <array>
#include <cctype>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>

int itemValue(char item) {
    if (std::isupper(item)) {
        return item - 'A' + 27;
    } else if (std::islower(item)) {
        return item - 'a' + 1;
    } else {
        return std::numeric_limits<int>::lowest();
    }
}

using Histogram = std::unordered_map<char, int>;

class Rucksack
{
private:
    std::string _items;
    std::shared_ptr<Histogram> _histogram;
public:
    explicit Rucksack(const std::string & items)
        : _items{items}
        , _histogram{std::make_shared<Histogram>()}
    {
        std::for_each(
            _items.begin(),
            _items.end(),
            [this](char c){ ++((*_histogram)[c]); });
    }
    ~Rucksack() = default;
    auto get_items() const { return _items; }
    auto get_histogram() const { return _histogram; }
};

constexpr int groupSize = 3;
using RucksackGroupArray = std::array<std::shared_ptr<Rucksack>, groupSize>;

class Group
{
public:
    static const int groupSize = 3; 
private:
    Histogram _histogram;
    RucksackGroupArray _rucksacks;
    char _commonItem;
public:
    explicit Group(RucksackGroupArray rucksacks)
        : _rucksacks{rucksacks}
    {
        for (const auto & rucksack : _rucksacks) {
            for (const auto & [item, occurances] : *rucksack->get_histogram()) {
                ++_histogram[item];
            }
        }

        for (const auto & [item, occurances] : _histogram) {
            if (occurances == groupSize) {
                _commonItem = item;
                break;
            }
        }

#ifdef DEBUG
            std::cout
                << "rucksack[0] = " << _rucksacks[0]->get_items() << std::endl
                << "rucksack[1] = " << _rucksacks[1]->get_items() << std::endl
                << "rucksack[2] = " << _rucksacks[2]->get_items() << std::endl;

            /*
            std::cout << "~histogram~" << std::endl;
            for (const auto & [item, occurances] : _histogram) {
                std::cout << " - " << item << " = " << occurances << std::endl;
            }
            */

            std::cout
                << "~commonItem~ = " << _commonItem << " (" << itemValue(_commonItem) << ")" << std::endl
                << std::string(80, '-') << std::endl;
#endif

    }
    ~Group() = default;
    auto getScore() const { return itemValue(_commonItem); }
};

int main()
{
    int score = 0, index = 0;
    RucksackGroupArray rucksacks;

    for (std::string line; std::getline(std::cin, line); ++index) {
        rucksacks[index % 3] = std::make_shared<Rucksack>(line);

        if (index % 3 == 2) {
            score += Group(rucksacks).getScore();
        }
    }

    std::cout << score << std::endl;

    return 0;
}

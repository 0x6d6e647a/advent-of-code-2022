#include <cctype>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <unordered_set>

int itemValue(char item) {
    if (std::isupper(item)) {
        return item - 'A' + 27;
    } else if (std::islower(item)) {
        return item - 'a' + 1;
    } else {
        return std::numeric_limits<int>::lowest();
    }
}

int processRucksack(std::string rucksack) {
    std::string left = rucksack.substr(0, rucksack.length() / 2);
    std::string right = rucksack.substr(rucksack.length() / 2, std::string::npos);
    
#ifdef DEBUG
    std::cout
        << "begin = " << rucksack << ", " << rucksack.length() << std::endl
        << "left = " << left << ", " << left.length() << std::endl
        << "right = " << right << ", " << right.length() << std::endl;
#endif

    std::unordered_set<char> itemTracker;
    std::for_each(left.begin(), left.end(), [&itemTracker](char c){ itemTracker.insert(c); });

    std::unordered_set<char> duplicates;
    std::for_each(
        right.begin(),
        right.end(),
        [&itemTracker, &duplicates](char c) {
            if (itemTracker.count(c) > 0) {
                duplicates.insert(c);
            }
        });

#ifdef DEBUG
    for ( const auto & duplicate : duplicates) {
        std::cout << duplicate << " = " << itemValue(duplicate) << std::endl;
    }
    std::cout << std::string(80, '-') << std::endl;
#endif

    return std::accumulate(
        duplicates.begin(),
        duplicates.end(),
        0,
        [](int value, char current) {
            return value + itemValue(current);
        });
}

int main()
{
    int score = 0;

    for (std::string line; std::getline(std::cin, line);) {
        score += processRucksack(line);
    }

    std::cout << score << std::endl;

    return 0;
}

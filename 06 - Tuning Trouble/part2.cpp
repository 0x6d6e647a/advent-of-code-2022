#include <iostream>
#include <optional>
#include <set>
#include <string>
#include <utility>

constexpr int markerSize(14);

std::optional<std::pair<std::string, std::string::size_type>>
findStartOfPacketMarker(std::string signal)
{
    if (signal.size() < markerSize) {
        return {};
    }

    for (auto iter = std::next(signal.begin(), markerSize);
         iter < signal.end();
         ++iter) {
        auto begin = std::prev(iter, markerSize);
        std::set unique(begin, iter);

        if (unique.size() == markerSize) {
            return {{{begin, iter}, std::distance(signal.begin(), iter)}};
        }
    }

    return {};
}

int main()
{
    for (std::string line; std::getline(std::cin, line);) {
        auto location = findStartOfPacketMarker(line)
            .value_or(std::make_pair("", -1));
        
        std::cout
#ifdef DEBUG
            << line
#endif
            << " => " << location.first
            << " @ " << location.second
            << std::endl;
    }

    return 0;
}

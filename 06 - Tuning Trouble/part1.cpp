#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <utility>

constexpr int markerSize(4);

std::optional<std::pair<std::string, std::string::size_type>>
findStartOfPacketMarker(std::string signal)
{
    for (decltype(signal)::size_type i = markerSize - 1;
         i < signal.size();
         ++i) {
        std::string maybeMarker(signal, i - (markerSize - 1), markerSize);
        std::map<char, int> charCounter;

        bool foundMarker = true;

        for (const auto & c : maybeMarker) {
            ++charCounter[c];

            if (charCounter[c] > 1) {
                foundMarker = false;
                break;
            }
        }

        if (!foundMarker) {
            continue;
        }

        return {{maybeMarker, i}};
    }

    return {};
}

int main()
{
    for (std::string line; std::getline(std::cin, line);) {
        auto location = findStartOfPacketMarker(line)
            .value_or(std::make_pair("", -1));
        
        std::cout
            << line
            << " => " << location.first
            << " @ " << location.second + 1
            << std::endl;
    }

    return 0;
}

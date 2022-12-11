#include <array>
#include <iostream>
#include <numeric>
#include <sstream>
#include <map>

enum class InstructionType { noop, addx };

InstructionType instructionTypeFromString(const std::string & s) {
    if (s == "noop") {
        return InstructionType::noop;
    }

    if (s == "addx") {
        return InstructionType::addx;
    }

    std::cerr << "unable to determine instruction type from string: " << s << std::endl;
    std::terminate();
}


constexpr int crtWidth = 40;
constexpr int crtHeight = 6;

std::array<std::array<char, crtWidth>, crtHeight> display;

constexpr std::array<std::pair<std::pair<int, int>, int>, crtHeight> cycleToDisplayRow{{
    {{  1,  40}, 0},
    {{ 41,  80}, 1},
    {{ 81, 120}, 2},
    {{121, 160}, 3},
    {{161, 200}, 4},
    {{201, 240}, 5},
}};

int getCycleToDisplayRow(const int & cycle)
{
    for (const auto & [range, row] : cycleToDisplayRow) {
        if ((cycle >= range.first) && (cycle <= range.second)) {
            return row;
        }
    } 

    std::cerr << "unable to determine row for cycle: " << cycle << std::endl;
    std::terminate();
}

int getCycleToDisplayCol(const int & cycle)
{
    return (cycle % crtWidth) - 1;
}
    
void drawPixel(const int & cycle, const int & registerX)
{
    auto x = getCycleToDisplayCol(cycle);

    if ((x >= registerX - 1) && (x <= registerX + 1)) {
        auto y = getCycleToDisplayRow(cycle);
        display[y][x] = '#';
    }
}

void executeProgram(std::istream & is)
{
    int cycle = 1;
    int registerX = 1;

    for(std::string line; std::getline(is, line);) {
        std::stringstream ss{line};
        std::string instrTypeStr, valueStr;
        ss >> instrTypeStr >> valueStr;

        auto instrType = instructionTypeFromString(instrTypeStr);
        auto value = valueStr.length() > 0 ? std::stoi(valueStr) : 0;

        drawPixel(cycle, registerX);

        switch (instrType) {
        case InstructionType::noop: {
            ++cycle;
            break;
        }
        case InstructionType::addx: {
            ++cycle;
            drawPixel(cycle, registerX);
            ++cycle;
            registerX += value;
            break;
        }
        }
    }
}

void printDisplay()
{
    for (int y = 0; y < crtHeight; ++y) {
        for (int x = 0; x < crtWidth; ++x) {
            std::cout << display[y][x];
        }
        std::cout << std::endl;
    }
}

int main()
{
    // Initialize display.
    for (int y = 0; y < crtHeight; ++y) {
        for (int x = 0; x < crtWidth; ++x) {
            display[y][x] = '.';
        }
    }

    executeProgram(std::cin);

    printDisplay();
    
    return 0;
}

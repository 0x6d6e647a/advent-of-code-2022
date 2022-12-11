#include <array>
#include <iostream>
#include <numeric>
#include <sstream>
#include <vector>

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

constexpr auto cyclesToEvaluate = std::to_array({20, 60, 100, 140, 180, 220});
int total = 0;

void evaluateSignalStrength(const int & cycle, const int & registerX)
{
    if (std::any_of(
            cyclesToEvaluate.begin(),
            cyclesToEvaluate.end(),
            [&cycle](auto cycleIndex){ return cycle == cycleIndex; })) {
#ifdef DEBUG
        std::cout
            << "cycle(" << cycle << ") = {" << std::endl
            << "  registerX     = " << registerX << std::endl
            << "  signalStrenth = " << (cycle * registerX) << std::endl
            << "};" << std::endl;
#endif
        total += cycle * registerX;
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

        evaluateSignalStrength(cycle , registerX);

        switch (instrType) {
        case InstructionType::noop: {
            ++cycle;
            break;
        }
        case InstructionType::addx: {
            ++cycle;
            evaluateSignalStrength(cycle , registerX);
            ++cycle;
            registerX += value;
            break;
        }
        }
    }
}

int main()
{
    executeProgram(std::cin);

    std::cout << total << std::endl;
        
    return 0;
}

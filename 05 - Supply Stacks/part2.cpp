#include <algorithm>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>


class CraneInstruction
{
private:
    int _quantity;
    int _sourceIndex;
    int _destinationIndex;

public:
    explicit CraneInstruction(std::string s)
    {
        const std::regex regex("move (\\d+) from (\\d+) to (\\d+)");
        std::smatch matches;

        if (std::regex_search(s, matches, regex)) {
            _quantity = std::stoi(matches[1]);
            _sourceIndex = std::stoi(matches[2]);
            _destinationIndex = std::stoi(matches[3]);
        }
        else {
            std::cerr << "error parsing crane instruction: " << s << std::endl;
            _quantity = -1;
            _sourceIndex = -1;
            _destinationIndex = -1;
        }
    }

    auto quantity()         const { return _quantity;         }
    auto sourceIndex()      const { return _sourceIndex;      }
    auto destinationIndex() const { return _destinationIndex; }

    ~CraneInstruction() = default;
};

class CraneAndCrateStacks
{
private:
    std::vector<std::deque<char>> _crateStacks;

public:
    explicit CraneAndCrateStacks(std::vector<std::deque<char>> && crateStacks)
        : _crateStacks(crateStacks)
    {}

    void executeInstructions(const std::vector<CraneInstruction> & craneInstructions)
    {
        for (const auto & instruction : craneInstructions) {
            std::deque<char> cratesToMove;

            for (int i = 0; i < instruction.quantity(); ++i) {
                auto crate = _crateStacks[instruction.sourceIndex() - 1].back();
                _crateStacks[instruction.sourceIndex() - 1].pop_back();
                cratesToMove.push_front(crate);
            }

            for (int i = 0; i < instruction.quantity(); ++i) {
                auto crate = cratesToMove.front();
                cratesToMove.pop_front();
                _crateStacks[instruction.destinationIndex() - 1].push_back(crate);
            }
        }
    }

    std::string getTopCrates() const
    {
        std::stringstream ss;

        for (const auto & stack : _crateStacks) {
            ss << stack.back();
        }

        return ss.str();
    }

#ifdef DEBUG
private:
    friend std::ostream & operator<<(std::ostream & os, const CraneAndCrateStacks & cacs);
#endif
};

#ifdef DEBUG
std::ostream & operator<<(std::ostream & os, const CraneAndCrateStacks & cacs)
{
    for (decltype(cacs._crateStacks)::size_type i = 0; i < cacs._crateStacks.size(); ++i) {
        os << i + 1 << ": ";

        for (const auto & crate: cacs._crateStacks[i]) {
            os << crate << ", ";
        }

        os << std::endl;
    }

    return os;
}
#endif

class CrateStackDrawing
{
private:
    std::vector<std::string> _drawing;

public:
    CrateStackDrawing() = default;
    ~CrateStackDrawing() = default;

    void addLine(std::string line) { _drawing.push_back(line); }

    CraneAndCrateStacks parseDrawing()
    {
        const std::regex uppercase("[[:upper:]]");
        const std::regex whitespaceOnly("^\\s+$");

        // Determine the number of columns.
        int numColumns = 0;
        std::istringstream labelSS(_drawing.back());

        for (std::string colLabel; std::getline(labelSS, colLabel, ' ');) {
            if (colLabel.empty()) {
                continue;
            }

            numColumns = std::stoi(colLabel);
        }

        // Parse crates into stacks.
        constexpr int columnCharWidth = 5;

        std::vector<std::deque<char>> crateStacks(numColumns);

        for (auto lineIterator = _drawing.begin();
             lineIterator != std::prev(_drawing.end());
             ++lineIterator) {
            std::istringstream stackLineSS(*lineIterator);

            for (int columnIndex = 0; columnIndex < numColumns; ++columnIndex) {
                char columnCharArr[columnCharWidth];
                stackLineSS.get(columnCharArr, columnCharWidth);
                columnCharArr[columnCharWidth - 1] = '\0';

                std::string columnString(columnCharArr);
                std::smatch matches;

                if (std::regex_search(columnString, matches, uppercase)) {
                    std::string columnCharString(matches[0]);
                    crateStacks[columnIndex].push_front(columnCharString[0]);
                }
                else {
                    continue;
                }
            }
        }

        return CraneAndCrateStacks(std::move(crateStacks));
    }
};

int main()
{
    CrateStackDrawing crateStackDrawing;
    std::vector<CraneInstruction> craneInstructions;

    enum class ParsingState { drawing, instructions }; 
    ParsingState parsingState = ParsingState::drawing;

    for (std::string line; std::getline(std::cin, line);) {
        if (line.empty()) {
            parsingState = ParsingState::instructions;
            continue;
        }

        if (parsingState == ParsingState::drawing) {
            crateStackDrawing.addLine(line);
        }
        else if (parsingState == ParsingState::instructions) {
            craneInstructions.push_back(CraneInstruction(line));
        }
        else {
            std::cerr << "bad parsing state" << std::endl;
            return 1;
        }
    }

    auto cargo = crateStackDrawing.parseDrawing();
    cargo.executeInstructions(craneInstructions);

    std::cout << cargo.getTopCrates() << std::endl;

    return 0;
}

#include <iostream>
#include <regex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using Int = std::int32_t;
using Label = std::string;

class Valve
{
private:
    Label _label;
    Int _flowRate;
    std::vector<Label> _tunnelsTo;

public:
    explicit Valve(const std::string & line)
    {
        std::regex regex{"^Valve ([[:upper:]]{2}) has flow rate=(-?\\d+); tunnels? leads? to valves? ([[:upper:]\\s,]+)$"};
        std::smatch matches;

        if (!std::regex_match(line, matches, regex)) {
            std::cerr << "Unable to parse Valve line: " << line << std::endl;
            std::terminate();
        }

        _label = matches[1];
        _flowRate = std::stoi(matches[2]);

        std::string tunnelsToStr{matches[3]};
        tunnelsToStr.erase(std::remove(tunnelsToStr.begin(), tunnelsToStr.end(), ','), tunnelsToStr.end());
        std::stringstream ss{tunnelsToStr};
        std::string tunnelLabel;
        
        while (ss >> tunnelLabel) {
            _tunnelsTo.push_back(tunnelLabel);
        }
    }

    auto label() const { return _label; }
    auto flowRate() const { return _flowRate; }
    auto tunnelsTo() const { return _tunnelsTo; }
};

using ValvePtr = std::shared_ptr<Valve>;

class Volcano
{
private:
    std::unordered_map<Label, ValvePtr> _valves;

public:
    explicit Volcano(std::istream & is)
    {
        for (std::string line; std::getline(is, line);) {
            auto valvePtr{std::make_shared<Valve>(line)};
            _valves.insert({valvePtr->label(), valvePtr});
        }
    }

    class State
    {
    private:
        Label _hereLabel;
        Int _releasing;
        Int _released;
        std::unordered_set<Label> _opened;

    public:
        State()
            : _hereLabel("AA")
            , _releasing(0)
            , _released(0)
        {}

        State(const Label & hereLabel,
              Int releasing,
              Int released,
              const std::unordered_set<Label> & opened)
            : _hereLabel(hereLabel)
            , _releasing(releasing)
            , _released(released)
            , _opened(opened)
        {}

        auto hereLabel() const { return _hereLabel; }
        auto releasing() const { return _releasing; }
        auto released()  const { return _released;  }
        auto opened()    const { return _opened;    }
    };

    Int play()
    {
        std::vector<State> states{{State()}};

        for (int minute = 0; minute < 30; ++minute) {
            std::vector<State> next;

            for (const auto & state : states) {
                Int released = state.released() + state.releasing();
                std::vector<Label> children{_valves[state.hereLabel()]->tunnelsTo()};

                for (const auto & child : children) {
                    State nextState{child, state.releasing(), released, state.opened()};
                    next.push_back(nextState);
                }

                if (!state.opened().contains(state.hereLabel())) {
                    std::unordered_set<Label> nextOpened{state.opened()};
                    nextOpened.insert(state.hereLabel());
                    Int releasing = state.releasing() + _valves[state.hereLabel()]->flowRate();
                    State nextState{state.hereLabel(), releasing, released, nextOpened};
                    next.push_back(nextState);
                }
            }

            std::sort(
                next.begin(),
                next.end(),
                [](const State & a, const State & b) {
                    return a.released() > b.released();
                });

            constexpr std::size_t sizeLimit = 2000;

            if (next.size() > sizeLimit) {
                next.erase(std::next(next.begin(), sizeLimit), next.end());
            }

            states = next;
        }

        return std::max_element(
            states.begin(),
            states.end(),
            [](const auto & a, const auto & b) {
                return a.released() < b.released();
            })->released();
                
    }
};

int main()
{
    Volcano volcano{std::cin};

    std::cout << volcano.play() << std::endl;
    
    return 0;
}

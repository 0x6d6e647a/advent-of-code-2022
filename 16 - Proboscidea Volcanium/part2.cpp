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
        Label _hereHumanLabel;
        Label _hereElephantLabel;
        Int _releasing;
        Int _released;
        std::unordered_set<Label> _opened;
        std::unordered_set<Label> _visited;

    public:
        State()
            : _hereHumanLabel("AA")
            , _hereElephantLabel("AA")
            , _releasing(0)
            , _released(0)
            , _visited({"AA"})
        {}

        State(const Label & hereHumanLabel,
              const Label & hereElephantLabel,
              Int releasing,
              Int released,
              const std::unordered_set<Label> & opened,
              const std::unordered_set<Label> & visted)
            : _hereHumanLabel(hereHumanLabel)
            , _hereElephantLabel(hereElephantLabel)
            , _releasing(releasing)
            , _released(released)
            , _opened(opened)
            , _visited(visted)
        {}

        auto hereHumanLabel()    const { return _hereHumanLabel;    }
        auto hereElephantLabel() const { return _hereElephantLabel; }
        auto releasing() const { return _releasing; }
        auto released()  const { return _released;  }
        auto opened()    const { return _opened;    }
        auto visited()   const { return _visited;   }
    };

    Int play()
    {
        std::vector<State> states{{State()}};

        for (int minute = 0; minute < 26; ++minute) {
            std::vector<State> next;

            for (const auto & state : states) {
                if (state.opened().size() == _valves.size()) {
                    continue;
                }

                auto visited{state.visited()};
                visited.insert(state.hereHumanLabel());
                visited.insert(state.hereElephantLabel());

                Int released = state.released() + state.releasing();

                std::vector<Label> childrenHuman{_valves[state.hereHumanLabel()]->tunnelsTo()};
                std::vector<Label> childrenElephant{_valves[state.hereElephantLabel()]->tunnelsTo()};

                for (const auto & childHuman : childrenHuman) {
                    for (const auto & childElephant : childrenElephant) {
                        State nextState{
                            childHuman,
                            childElephant,
                            state.releasing(),
                            released,
                            state.opened(),
                            visited};
                        next.push_back(nextState);
                    }
                }

                if (!state.opened().contains(state.hereHumanLabel())) {
                    for (const auto & childElephant : childrenElephant) {
                        std::unordered_set<Label> nextOpened{state.opened()};
                        nextOpened.insert(state.hereHumanLabel());

                        Int releasing = state.releasing() + _valves[state.hereHumanLabel()]->flowRate();

                        State nextState{
                            state.hereHumanLabel(),
                            childElephant,
                            releasing,
                            released,
                            nextOpened,
                            visited};
                        next.push_back(nextState);
                    }
                }

                if (!state.opened().contains(state.hereElephantLabel())) {
                    for (const auto & childHuman : childrenHuman) {
                        std::unordered_set<Label> nextOpened{state.opened()};
                        nextOpened.insert(state.hereElephantLabel());

                        Int releasing = state.releasing() + _valves[state.hereElephantLabel()]->flowRate();

                        State nextState{
                            childHuman,
                            state.hereElephantLabel(),
                            releasing,
                            released,
                            nextOpened,
                            visited};
                        next.push_back(nextState);
                   }
                }

                if ((!state.opened().contains(state.hereHumanLabel())) &&
                    (!state.opened().contains(state.hereElephantLabel())) &&
                    (state.hereHumanLabel() != state.hereElephantLabel())) {
                    std::unordered_set<Label> nextOpened{state.opened()};
                    nextOpened.insert(state.hereHumanLabel());
                    nextOpened.insert(state.hereElephantLabel());

                    Int releasing = state.releasing()
                        + _valves[state.hereHumanLabel()]->flowRate()
                        + _valves[state.hereElephantLabel()]->flowRate();

                    State nextState{
                        state.hereHumanLabel(),
                        state.hereElephantLabel(),
                        releasing,
                        released,
                        nextOpened,
                        visited};
                    next.push_back(nextState);
                }
            }

            std::sort(
                next.begin(),
                next.end(),
                [&minute](const State & a, const State & b) {
                    if (minute < 1) {
                        if (a.releasing() != b.releasing()) {
                            return a.releasing() > b.releasing();
                        }
                        else {
                            return a.released() > b.released();
                        }
                    }
                    else {
                        if (a.released() != b.released()) {
                            return a.released() > b.released();
                        }
                        else {
                            return a.releasing() > b.releasing();
                        }
                    }
                });

            constexpr std::size_t sizeLimit = 100'000;

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

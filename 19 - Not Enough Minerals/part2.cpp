#include <cmath>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <regex>

using Int = std::int64_t;
using Size = std::size_t;
using String = std::string;

enum class Resource { Ore, Clay, Obsidian, Geode };

Resource resourceFromString(const String & str)
{
    static const std::regex ore{"ore", std::regex_constants::icase};
    static const std::regex clay{"clay", std::regex_constants::icase};
    static const std::regex obsidian{"obsidian", std::regex_constants::icase};
    static const std::regex geode{"geode", std::regex_constants::icase};

    if (std::regex_match(str, ore)) {
        return Resource::Ore;
    }
    else if (std::regex_match(str, clay)) {
        return Resource::Clay;
    }
    else if (std::regex_match(str, obsidian)) {
        return Resource::Obsidian;
    }
    else if (std::regex_match(str, geode)) {
        return Resource::Geode;
    }

    std::cerr
        << "unable to parse resource from string: \""
        << str
        << "\"" << std::endl;
    std::terminate();
}

String resourceToString(Resource resource)
{
    switch (resource) {
    case Resource::Ore: {
        return "Ore"; 
    }
    case Resource::Clay: {
        return "Clay"; 
    }
    case Resource::Obsidian: {
        return "Obsidian"; 
    }
    case Resource::Geode: {
        return "Geode"; 
    }
    }

    return "ERROR";
}

class Robot
{
private:
    Resource _type;
    std::unordered_map<Resource, Size> _costs;

public:
    explicit Robot(const String & str)
    {
        static const std::regex typeRegex{"Each (\\w+) robot costs"};
        std::smatch matches;

        if (!std::regex_search(str, matches, typeRegex)) {
            std::cerr << "unable to parse robot type: " << str << std::endl;
            std::terminate();
        }

        _type = resourceFromString(matches[1]);

        // Find the beginning of the costs.
        String costs_str{"costs "};
        Size found = str.find(costs_str);

        if (found == String::npos) {
            std::cerr << "unable to find \"" << costs_str << "\": " << str << std::endl;
            std::terminate();
        }

        String costsStr{str.substr(found + costs_str.size())};

        // Parse resource costs.
        for (auto iter = costsStr.begin(); iter != costsStr.end(); ++iter) {
            String curr{iter, costsStr.end()};

            // Parse current resource.
            static const std::regex num_resource{"(\\d+) (\\w+)"};

            if (!std::regex_search(curr, matches, num_resource)) {
                std::cerr << "unable to parse resource amount: " << curr << std::endl;
            }

            Size amount;
            Resource resource{resourceFromString(matches[2])};

            std::stringstream ss{matches[1]};
            ss >> amount;

            _costs.insert({resource, amount});

            // Try to find next entry.
            String and_str{"and "};
            Size foundStr = curr.find(and_str);

            // No further entries, seek ending.
            if (foundStr == String::npos) {
                auto period = std::find(iter, costsStr.end(), '.');

                if (period == costsStr.end()) {
                    std::cerr << "unable to find end of costs: " << curr << std::endl;
                    std::terminate();
                }

                iter = period;
                continue;
            }

            // Setup parsing for next entry.
            iter = std::next(iter, foundStr + and_str.size() - 1);
        }
    }

    auto type() const { return _type; }

    auto costs_cbegin() const { return _costs.cbegin(); }
    auto costs_cend() const { return _costs.cend(); }

    bool canAfford(const std::unordered_map<Resource, Size> & resources)
        const
    {
        for (const auto & cost : _costs) {
            auto resource = cost.first;
            auto amount = cost.second;

            try {
                if (resources.at(resource) < amount) {
                    return false;
                }
            }
            catch (const std::out_of_range & ex) {
                return false;
            }
        }

        return true;
    }

    std::unordered_map<Resource, Size> pay(const std::unordered_map<Resource, Size> & resources)
        const
    {
        // Check that we can afford it.
        if (!canAfford(resources)) {
            std::cerr << "attempt to construct robot you cannot afford" << std::endl;
            std::terminate();
        }

        // Pay for robot.
        std::unordered_map<Resource, Size> newResources{resources};

        for (const auto & cost : _costs) {
            auto resource = cost.first;
            auto amount = cost.second;

            newResources[resource] -= amount;
        }

        return newResources;
    }
};

class Blueprint
{
private:
    Size _index;
    std::unordered_map<Resource, Robot> _robots;

public:
    explicit Blueprint(const String & str)
    {
        // Parse blueprint index.
        static const std::regex indexRegex{"Blueprint (\\d+):"};
        std::smatch matches;

        if (!std::regex_search(str, matches, indexRegex)) {
            std::cerr << "unable to parse blueprint index: " << str << std::endl;
            std::terminate();
        }

        std::stringstream ss{matches[1]};
        ss >> _index;

        // Parse robots.
        for (auto iter = str.begin(); iter != str.end();) {
            // Move past index section.
            if (iter == str.begin()) {
                auto next = std::find(iter, str.end(), ':');

                if (next == str.end()) {
                    std::cerr
                        << "unable to find end of blueprint index section: "
                        << String(iter, str.end())
                        << std::endl;
                    std::terminate();
                }

                iter = std::next(next); // Move past space.
                continue;
            }

            // Move forward over space.
            if (*iter == ' ') {
                ++iter;
            }

            // Each robot entry starts with 'E'.
            if (*iter != 'E') {
                std::cerr
                    << "not positioned at the beginning of a robot entry: \""
                    << String(iter, str.end())
                    << "\"" << std::endl;
                std::terminate();
            }

            // Parse robot entry.
            auto endOfEntry = std::find(iter, str.end(), '.');

            if (endOfEntry == str.end()) {
                std::cerr
                    << "unable to find end of robot entry: "
                    << String(iter, str.end())
                    << std::endl;
                std::terminate();
            }

            endOfEntry = std::next(endOfEntry); // Move past period.

            Robot robot{String(iter, endOfEntry)};
            _robots.insert({robot.type(), robot});

            iter = endOfEntry;
        }
    }

    auto index() const { return _index; }

    auto robots_cbegin() const { return _robots.cbegin(); }
    auto robots_cend() const { return _robots.cend(); }

    auto getRobot(const Resource & resource)
        const
    {
        try {
            return _robots.at(resource);
        }
        catch (const std::out_of_range & ex) {
            std::cerr << "unable to get robot: " << resourceToString(resource) << std::endl;
            std::terminate();
        }
    }
};

constexpr Int minutesLimit = 32;

class State
{
private:
    std::unordered_map<Resource, Size> _resources;
    std::unordered_map<Resource, Size> _robots;

public:
    State()
    {
        ++_robots[Resource::Ore];
    }

    State(decltype(_resources) resources,
          decltype(_robots) robots)
        : _resources(resources)
        , _robots(robots)
    {}

    auto resources() const { return _resources; }
    auto robots() const { return _robots; }

    auto resources_cbegin() const { return _resources.cbegin(); }
    auto resources_cend() const { return _resources.cend(); }

    auto robots_cbegin() const { return _robots.cbegin(); }
    auto robots_cend() const { return _robots.cend(); }
};

class StateScoringEngine
{
private:
    std::unordered_map<Resource, Size> _resourceScores;

public:
    explicit StateScoringEngine(const Blueprint & blueprint)
    {
        _resourceScores[Resource::Ore] = 1;

        for (const auto & robotResource : {
                Resource::Clay,
                Resource::Obsidian,
                Resource::Geode}) {
            auto robot{blueprint.getRobot(robotResource)};

            Size totalCost = 0;
                                      
            for (auto iterCosts = robot.costs_cbegin();
                 iterCosts != robot.costs_cend();
                 ++iterCosts) {
                auto resource{iterCosts->first};
                auto cost{iterCosts->second};

                totalCost += _resourceScores[resource] * cost;
            }

            _resourceScores[robotResource] = totalCost;
        }
    }

    Int robotBonus(Resource robotType)
    {
        return std::floor(_resourceScores[robotType] / 2);
    }

    Int scoreState(const State & state)
    {
        Int scoreTotal = 0;

        for (auto iterResource = state.resources_cbegin();
             iterResource != state.resources_cend();
             ++iterResource) {
            auto resourceType = iterResource->first;
            auto quantity = iterResource->second;

            scoreTotal += _resourceScores[resourceType] * quantity;
        }

        for (auto iterRobot = state.robots_cbegin();
             iterRobot != state.robots_cend();
             ++iterRobot) {
            auto resourceType = iterRobot->first;
            auto quantity = iterRobot->second;

            scoreTotal += _resourceScores[resourceType] * quantity + robotBonus(resourceType);
        }

        return scoreTotal;
    }
};

static const String hrBig{String(80, '=')};
static const String hrSmall{String(80, '~')};

Int score(Blueprint blueprint)
{
    std::cout
        << hrBig << std::endl
        << "Blueprint #" << blueprint.index() << std::endl;

    StateScoringEngine sse(blueprint);
    std::vector<State> states{{State()}};

    for (Int minute = 0; minute < minutesLimit; ++minute) {
        std::cout
            << hrSmall << std::endl
            << ">>> Minute #" << minute + 1 << std::endl;

        std::vector<State> next;

        for (const auto & state : states) {
            // Build all robots you can afford.
            for (auto iter = blueprint.robots_cbegin();
                 iter != blueprint.robots_cend();
                 ++iter) {
                auto blueprintType = iter->first;
                auto blueprintRobot = iter->second;

                if (blueprintRobot.canAfford(state.resources())) {
                    // Pay for robot building.
                    std::unordered_map<Resource, Size> newResources{blueprintRobot.pay(state.resources())};

                    // Do resource gathering.
                    for (const auto & robot : state.robots()) {
                        auto type = robot.first;
                        auto num = robot.second;

                        newResources[type] += num;
                    }

                    // Register newly built robot.
                    std::unordered_map<Resource, Size> newRobots{state.robots()};
                    ++newRobots[blueprintType];

                    next.push_back({newResources, newRobots});
                }
            }

            // Don't build any robots.
            std::unordered_map<Resource, Size> newResources{state.resources()};
            std::unordered_map<Resource, Size> newRobots{state.robots()};

            // Do resource gathering.
            for (const auto & robot : state.robots()) {
                auto type = robot.first;
                auto num = robot.second;

                newResources[type] += num;
            }

            next.push_back({newResources, newRobots});
        }

        std::sort(
            next.begin(),
            next.end(),
            [&sse](const auto & a, const auto & b) {
                return sse.scoreState(a) > sse.scoreState(b);
            });

#ifdef DEBUG
        std::cout << "Generated " << next.size() << " new states." << std::endl;
#endif // DEBUG

        constexpr Size sizeLimit = 50'000;

        if (next.size() > sizeLimit) {
            next.erase(std::next(next.begin(), sizeLimit), next.end());
        }

        states = next;

#ifdef DEBUG
        std::cout
            << "* Best State" << std::endl
            << "  - Score :: " << sse.scoreState(states[0]) << std::endl
            << "** Resources" << std::endl;

        for (const auto & resourcePair : states[0].resources()) {
            auto resource{resourcePair.first};
            auto amount{resourcePair.second};

            std::cout << "   - " << resourceToString(resource) << " :: " << amount << std::endl;
        }

        std::cout << "** Robots" << std::endl;

        for (const auto & robotPair : states[0].robots()) {
            auto resource{robotPair.first};
            auto amount{robotPair.second};

            std::cout << "   - " << resourceToString(resource) << " :: " << amount << std::endl;
        }
#endif // DEBUG
    }

    auto mostGeodesIter = std::max_element(
        states.begin(),
        states.end(),
        [](const auto & a, const auto & b) {
            return a.resources()[Resource::Geode] < b.resources()[Resource::Geode];
        });

#ifdef DEBUG
        std::cout
            << "* FINAL Best State" << std::endl
            << "  - Score :: " << sse.scoreState(*mostGeodesIter) << std::endl
            << "** Resources" << std::endl;

        for (const auto & resourcePair : mostGeodesIter->resources()) {
            auto resource{resourcePair.first};
            auto amount{resourcePair.second};

            std::cout << "   - " << resourceToString(resource) << " :: " << amount << std::endl;
        }

        std::cout << "** Robots" << std::endl;

        for (const auto & robotPair : mostGeodesIter->robots()) {
            auto resource{robotPair.first};
            auto amount{robotPair.second};

            std::cout << "   - " << resourceToString(resource) << " :: " << amount << std::endl;
        }
#endif // DEBUG

    Int mostGeodes = mostGeodesIter->resources()[Resource::Geode];

#ifdef DEBUG
    std::cout
        << "Blueprint #" << blueprint.index()
        << ": " << mostGeodes
        << " = " << mostGeodes * blueprint.index()
        << std::endl;
#endif // DEBUG

    return mostGeodes;
}

Int result(std::istream & is)
{
    std::vector<Int> scores;

    for (String line; std::getline(is, line);) {
        Blueprint blueprint{line};
        scores.push_back(score(blueprint));

        if (blueprint.index() == 3) {
            break;
        }
    }

#ifdef DEBUG
    for (Size i = 0; i < scores.size(); ++i) {
        std::cout << "Blueprint #" << i << ": " << scores[i] << std::endl;
    }
#endif // DEBUG

    return std::accumulate(
        scores.begin(),
        scores.end(),
        Int(1),
        std::multiplies<Int>());
}
        
int main()
{
    std::cout << result(std::cin) << std::endl;
    return 0;
}

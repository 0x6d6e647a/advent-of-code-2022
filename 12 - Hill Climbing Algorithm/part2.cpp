#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <vector>

using CoordinatePair = std::pair<int, int>;

class Hill;
using HillPtr = std::shared_ptr<Hill>;
using HillMap = std::map<CoordinatePair, HillPtr>;

class Hill
{
private:
    int _x;
    int _y;
    int _elevation;
    bool _isStart;
    bool _isEnd;
    HillPtr _parent;

public:
    static int elevationCharToInt(const char & elevation)
    {
        if (elevation == 'S') {
            return 0;
        }
        else if (elevation == 'E') {
            return 'z' - 'a';
        }

        return elevation - 'a';
    }

    static bool isStartHill(const char & elevation)
    {
        return elevation == 'S';
    }

    static bool isEndHill(const char & elevation)
    {
        return elevation == 'E';
    }

    Hill() = default;

    Hill(int x, int y, char elevation)
        : _x(x)
        , _y(y)
        , _elevation(elevationCharToInt(elevation))
        , _isStart(isStartHill(elevation))
        , _isEnd(isEndHill(elevation))
        , _parent(nullptr)
    {}

    auto x() const { return _x; }
    auto y() const { return _y; }
    CoordinatePair coordinatePair() const { return {_x, _y}; }
    auto elevation() const { return _elevation; }
    auto isStart() const { return _isStart; }
    auto isEnd() const { return _isEnd; }
    auto parent() const { return _parent; }

    void setParent(HillPtr hillPtr)
    {
        _parent = hillPtr;
    }

    bool canReach(const Hill & other)
        const
    {
        return other._elevation <= _elevation + 1;
    }

    std::vector<HillPtr> edges(const HillMap & hillMap,
                               const std::vector<CoordinatePair> & visited)
    {
        std::vector<HillPtr> edges;

        // Up
        auto up = getEdge({_x, _y - 1}, hillMap, visited);

        if (up.has_value()) {
            edges.push_back(*up);
        }

        // Down
        auto down = getEdge({_x, _y + 1}, hillMap, visited);

        if (down.has_value()) {
            edges.push_back(*down);
        }

        // Left
        auto left = getEdge({_x - 1, _y}, hillMap, visited);

        if (left.has_value()) {
            edges.push_back(*left);
        }

        // Right
        auto right = getEdge({_x + 1, _y}, hillMap, visited);

        if (right.has_value()) {
            edges.push_back(*right);
        }


        return edges;
    }

    std::optional<HillPtr> getEdge(CoordinatePair coordinatePair,
                                  const HillMap & hillMap,
                                  const std::vector<CoordinatePair> & visted)
        const
    {
        if (!hillMap.contains(coordinatePair)) {
            return {};
        }

        if (std::count(visted.begin(), visted.end(), coordinatePair)) {
            return {};
        }

        if (!canReach(*hillMap.at(coordinatePair))) {
            return {};
        }

        return {hillMap.at(coordinatePair)};
    }
};

std::pair<HillMap, std::vector<HillPtr>> parseInput(std::istream & is)
{
    HillMap hillMap;
    std::vector<HillPtr> starts;

    int y = 0;

    for (std::string line; std::getline(is, line);) {
        int x = 0;

        for (const auto & c : line) {
            auto hillPtr{std::make_shared<Hill>(x, y, c)};
            hillMap.insert({{x, y}, hillPtr});

            if ((hillPtr->isStart()) || (hillPtr->elevation() == 0)) {
                starts.push_back(hillPtr);
            }

            ++x;
        }

        ++y;
    }

    return {hillMap, starts};
}

void clearParents(const HillMap & hillMap)
{
    for (const auto & [coords, hillPtr] : hillMap) {
        hillPtr->setParent(nullptr);
    }
}

HillPtr breadthFirstSearch(const HillMap & hillMap, HillPtr & start)
{
    clearParents(hillMap);

    std::queue<HillPtr> queue{{start}};
    std::vector<CoordinatePair> visited{{start->coordinatePair()}};

    while (!queue.empty()) {
        HillPtr curr = queue.front();
        queue.pop();

        if (curr->isEnd()) {
            return curr;
        }

        for (const auto & next : curr->edges(hillMap, visited)) {
            visited.push_back(next->coordinatePair());
            next->setParent(curr);
            queue.push(next);
        }
    }

    return nullptr;
}

int scorePath(const HillPtr & end) {
    HillPtr curr = end;
    int score = 0;

    do {
        if (!curr) {
            return score;
        }
        ++score;
        curr = curr->parent();
    } while (curr->elevation() != 0);

    return score;
}

int findBestPath(const HillMap & hillMap, std::vector<HillPtr> & starts)
{
    int lowestScore = std::numeric_limits<int>::max();

    for (auto & start : starts) {
        auto end = breadthFirstSearch(hillMap, start);
        auto score = scorePath(end);

        if ((score > 0) && (score < lowestScore)) {
            lowestScore = score;
        }
    }

    return lowestScore;
}


int main()
{
    auto parsed{parseInput(std::cin)};
    HillMap hillMap = parsed.first;
    std::vector<HillPtr> starts = parsed.second;

    std::cout << findBestPath(hillMap, starts) << std::endl;

    return 0;
}

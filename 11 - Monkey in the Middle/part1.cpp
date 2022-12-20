#include <cmath>
#include <functional>
#include <iostream>
#include <numeric>
#include <regex>
#include <variant>
#include <vector>

enum class NamedOperationElement { Old };

using Value = std::int32_t;

using OperationElement = std::variant<NamedOperationElement, Value>;

OperationElement parseOperationElement(const std::string & s)
{
    const std::regex number{"^\\d+"};

    if (std::regex_match(s, number)) {
        return std::stoi(s);
    }
    else {
        return NamedOperationElement::Old;
    }
}

enum class OperationFunction { Addition, Multiplication };

OperationFunction parseOperationFunction(const std::string & s)
{
    if (s == "+") {
        return OperationFunction::Addition;
    }
    else if (s == "*") {
        return OperationFunction::Multiplication;
    }

    std::cerr << "Unable to parse operation function: " << s << std::endl;
    std::terminate();
}

std::function<Value(const Value &, const Value &)> getFunction(const OperationFunction & op)
{
    switch (op) {
    case OperationFunction::Addition: {
        return [](const Value & a, const Value & b){ return a + b; };
    }
    case OperationFunction::Multiplication: {
        return [](const Value & a, const Value & b){ return a * b; };
    }
    }

    std::cerr << "Unable to generate function" << std::endl;
    std::terminate();
}

class Operation
{
private:
    OperationElement _a;
    OperationFunction _func;
    OperationElement _b;

public:
    Operation()
        : _func(OperationFunction::Addition)
    {}

    Operation(const std::string & a,
              const std::string & func,
              const std::string & b)
        : _a(parseOperationElement(a))
        , _func(parseOperationFunction(func))
        , _b(parseOperationElement(b))
    {}

    ~Operation() = default;

    Value apply(Value old)
    {
        // Get first argument.
        Value a;

        if (std::holds_alternative<Value>(_a)) {
            a = std::get<Value>(_a);
        }
        else {
            a = old;
        }

        // Get function.
        auto func = getFunction(_func);

        // Get second argument.
        Value b;

        if (std::holds_alternative<Value>(_b)) {
            b = std::get<Value>(_b);
        }
        else {
            b = old;
        }

        return func(a, b);
    }
};

class Monkey;
using MonkeyIndexMap = std::map<int, Monkey>;

class Monkey
{
private:
    int _index;
    std::deque<Value> _items;
    Operation _operation;
    int _divisible;
    int _trueIndex;
    int _falseIndex;
    int _totalInspections;

public:
    Monkey()
        : _index(-1)
        , _divisible(-1)
        , _trueIndex(-1)
        , _falseIndex(-1)
        , _totalInspections(-1)
    {}

    explicit Monkey(std::istream & is)
        : _totalInspections(0)
    {
        std::smatch matches;
        std::string line;

        // Parse monkey index.

        std::getline(is, line);
        const std::regex monkey_index{"^Monkey\\s+(\\d+):$"};
        
        if (!std::regex_search(line, matches, monkey_index)) {
            std::cerr << "unable to parse monkey index: " << line << std::endl;
            std::terminate();
        }

        _index = std::stoi(matches[1]);
            
        // Parse starting items.

        std::getline(is, line);
        const std::regex starting_items{"^\\s+Starting items:\\s+([\\d,\\s]+)$"};

        if (!std::regex_search(line, matches, starting_items)) {
            std::cerr << "unable to parse starting items: " << line << std::endl;
            std::terminate();
        }

        std::string items = matches[1];

        items.erase(std::remove(items.begin(), items.end(), ','), items.end());

        std::string itemStr;
        std::istringstream iss{items};

        while (iss >> itemStr) {
            _items.push_back(std::stoi(itemStr));
        }

        // Parse operation.

        std::getline(is, line);
        const std::regex operation{"^\\s+Operation:\\s+new\\s+=\\s+(\\w+)\\s+([\\*\\+])\\s+(\\w+)$"};

        if (!std::regex_search(line, matches, operation)) {
            std::cerr << "unable to parse operation: " << line << std::endl;
            std::terminate();
        }

        _operation = {matches[1], matches[2], matches[3]};

        // Parse divisible test.

        std::getline(is, line);
        const std::regex test_div{"^\\s+Test:\\s+divisible\\s+by\\s+(\\d+)$"};

        if (!std::regex_search(line, matches, test_div)) {
            std::cerr << "unable to divisibility test: " << line << std::endl;
            std::terminate();
        }

        _divisible = std::stoi(matches[1]);

        // Parse true throw to.

        std::getline(is, line);
        const std::regex if_true{"^\\s+If\\s+true:\\s+throw\\s+to\\s+monkey\\s+(\\d+)$"};

        if (!std::regex_search(line, matches, if_true)) {
            std::cerr << "unable to true index: " << line << std::endl;
            std::terminate();
        }

        _trueIndex = std::stoi(matches[1]);

        // Parse false throw to.

        std::getline(is, line);
        const std::regex if_false{"^\\s+If\\s+false:\\s+throw\\s+to\\s+monkey\\s+(\\d+)$"};

        if (!std::regex_search(line, matches, if_false)) {
            std::cerr << "unable to false index: " << line << std::endl;
            std::terminate();
        }

        _falseIndex = std::stoi(matches[1]);
    }

    ~Monkey() = default;

    auto index() const { return _index; }
    auto totalInspections() const { return _totalInspections; }

    void catchItem(Value item)
    {
        _items.push_back(item);
    }

    void inspectItems(MonkeyIndexMap & monkeys)
    {
        while (!_items.empty()) {
            auto item = _items.front();
            _items.pop_front();

            auto worryLevel = item;
            worryLevel = _operation.apply(worryLevel);
            worryLevel = std::floor(worryLevel / 3);

            if (worryLevel % _divisible == 0) {
                monkeys[_trueIndex].catchItem(worryLevel);
            }
            else {
                monkeys[_falseIndex].catchItem(worryLevel);
            }

            ++_totalInspections;
        }
    }
};

MonkeyIndexMap parseMonkeys(std::istream & is)
{
    MonkeyIndexMap monkeyMap;
    int index = 0;
    std::string line;

    while (!is.eof()) {
        Monkey monkey{is};

        if (monkey.index() != index) {
            std::cerr << "Monkey indexes out of order: " << index << ", " << monkey.index() << std::endl;
            std::terminate();
        }

        monkeyMap.insert({index, monkey});

        if (std::getline(is, line).eof()) {
            break;
        }

        if (!line.empty()) {
            std::cerr << "Error while parsing monkeys: " << line << std::endl;
            std::terminate();
        }

        ++index;
    }

    return monkeyMap;
}

constexpr int numRounds = 20;

void playGame(MonkeyIndexMap & monkeys)
{
    for (int i = 0; i < numRounds; ++i) {
        for (auto & [monkeyIndex, monkey] : monkeys) {
            monkey.inspectItems(monkeys);
        }
    }
}

Value scoreGame(const MonkeyIndexMap & monkeys)
{
    std::vector<Value> scores;

    std::transform(monkeys.begin(),
                   monkeys.end(),
                   std::back_inserter(scores),
                   [](const std::pair<int, Monkey> & monkeyPair) {
                       return monkeyPair.second.totalInspections();
                   });
    std::sort(scores.begin(), scores.end());

    return std::accumulate(
        scores.rbegin(),
        std::next(scores.rbegin(), 2),
        Value{1},
        std::multiplies<Value>());
}

int main()
{
    auto monkeys = parseMonkeys(std::cin);
    playGame(monkeys);

    std::cout << scoreGame(monkeys) << std::endl;
        
    return 0;
}

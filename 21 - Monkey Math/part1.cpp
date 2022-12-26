#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <regex>
#include <unordered_map>

using Int = std::int64_t;
using Size = std::size_t;
using String = std::string;

enum class Operation { Addition, Subtraction, Multiplication, Division };

Operation charToOperation(const char & c)
{
    switch (c) {
    case '+': {
        return Operation::Addition;
    }
    case '-': {
        return Operation::Subtraction;
    }
    case '*': {
        return Operation::Multiplication;
    }
    case '/': {
        return Operation::Division;
    }
    }

    std::cerr << "unable to get operation from character: " << c << std::endl;
    std::terminate();
}

String operationToString(const Operation & operation)
{
    switch (operation) {
    case Operation::Addition: {
        return "Addition";
    }
    case Operation::Subtraction: {
        return "Subtraction";
    }
    case Operation::Multiplication: {
        return "Multiplication";
    }
    case Operation::Division: {
        return "Division";
    }
    }

    std::cerr << "unable to get string for operation" << std::endl;
    std::terminate();
}

using OperationFunction = std::function<Int(Int, Int)>;

OperationFunction operationToFunction(const Operation & operation)
{
    switch (operation) {
    case Operation::Addition: {
        return std::plus<Int>();
    }
    case Operation::Subtraction: {
        return std::minus<Int>();
    }
    case Operation::Multiplication: {
        return std::multiplies<Int>();
    }
    case Operation::Division: {
        return std::divides<Int>();
    }
    }

    std::cerr << "unable to get function for operation: " << operationToString(operation) << std::endl;
    std::terminate();
}

class Monkey;
using MonkeyPtr = std::shared_ptr<Monkey>;

class Monkey
{
private:
    String _name;
    std::optional<String> _leftName;
    std::optional<Operation> _operation;
    std::optional<String> _rightName;
    std::optional<Int> _value;
    MonkeyPtr _leftPtr;
    MonkeyPtr _rightPtr;

public:
    Monkey(const String & name,
           const String & leftName,
           Operation operation,
           const String & rightName)
        : _name(name)
        , _leftName(leftName)
        , _operation(operation)
        , _rightName(rightName)
    {}

    Monkey(const String & name, Int value)
        : _name(name)
        , _value(value)
    {}

    auto name() const { return _name; }
    auto leftName() const { return _leftName; }
    auto rightName() const { return _rightName; }

    void setLeftPtr(MonkeyPtr leftPtr) { _leftPtr = leftPtr; }
    void setRightPtr(MonkeyPtr rightPtr) { _rightPtr = rightPtr; }

    Int value()
        const
    {
        // Simply return value if available.
        if (_value.has_value()) {
            return _value.value();
        }

        // Fail if no value and no left or right pointers.
        if (!_leftPtr || !_rightPtr || !_operation.has_value()) {
            std::cerr << "unable to retrieve value, no value or left or right pointers or operation" << std::endl;
            std::terminate();
        }

        // Perform operation and return value.
        auto operationFunc{operationToFunction(_operation.value())};
        return operationFunc(_leftPtr->value(), _rightPtr->value());
    }
};

MonkeyPtr parseMonkeys(std::istream & is)
{
    std::unordered_map<String, MonkeyPtr> monkeyMap;

    // Parse monkeys from lines.
    for (String line; std::getline(is, line);) {
        static std::regex monkeyOperation{"^(\\w{4}): (\\w{4}) ([+-/*]) (\\w{4})$"};
        static std::regex monkeyValue{"^(\\w{4}): (-?\\d+)$"};
        std::smatch matches;

        if (std::regex_match(line, matches, monkeyOperation)) {
            auto monkey = std::make_shared<Monkey>(matches[1],
                                                   matches[2],
                                                   charToOperation(String(matches[3])[0]),
                                                   matches[4]);
            monkeyMap.insert({monkey->name(), monkey});
        }
        else if (std::regex_match(line, matches, monkeyValue)) {
            auto monkey = std::make_shared<Monkey>(matches[1],
                                                   std::stoll(matches[2]));
            monkeyMap.insert({monkey->name(), monkey});
        }
        else {
            std::cerr << "unable to parse monkey string: " << line << std::endl;
            std::terminate();
        }
    }

    // Setup pointers to/fro monkeys.
    for (const auto & [monkeyName, monkeyPtr] : monkeyMap) {
        if (monkeyPtr->leftName().has_value() &&
            monkeyPtr->rightName().has_value()) {
            auto leftName{monkeyPtr->leftName().value()};
            auto rightName{monkeyPtr->rightName().value()};

            monkeyPtr->setLeftPtr(monkeyMap[leftName]);
            monkeyPtr->setRightPtr(monkeyMap[rightName]);
        }
    }

    return monkeyMap["root"];
}

int main()
{
    auto rootMonkey{parseMonkeys(std::cin)};
    std::cout << rootMonkey->value() << std::endl;
    return 0;
}

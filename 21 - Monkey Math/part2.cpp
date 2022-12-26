#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <regex>
#include <unordered_map>
#include <variant>

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

enum class EquationSide { Left, Right };

class IncompleteOperation;
using IncompleteOperationPtr = std::shared_ptr<IncompleteOperation>;
class Human {};

using OperationResult = std::variant<Int, Human, IncompleteOperationPtr>;

class IncompleteOperation
{
private:
    Operation _operation;
    OperationResult _left;
    OperationResult _right;

public:
    IncompleteOperation(Operation operation,
                        OperationResult left,
                        OperationResult right)
        : _operation(operation)
        , _left(left)
        , _right(right)
    {}

    auto operation() const { return _operation; }
    auto left() const { return _left; }
    auto right() const { return _right; }

    EquationSide valueSide()
        const
    {
        if (std::holds_alternative<Int>(_left)) {
            return EquationSide::Left;
        }
        else if (std::holds_alternative<Int>(_right)) {
            return EquationSide::Right;
        }

        std::cerr << "unable to determine value size in incomplete operation" << std::endl;
        std::terminate();
    }

    EquationSide variableSide()
        const
    {
        if (std::holds_alternative<Human>(_left) ||
            std::holds_alternative<IncompleteOperationPtr>(_left)) {
            return EquationSide::Left;
        }
        else if (std::holds_alternative<Human>(_right) ||
                 std::holds_alternative<IncompleteOperationPtr>(_right)) {
            return EquationSide::Right;
        }

        std::cerr << "unable to determine variable size in incomplete operation" << std::endl;
        std::terminate();
    }

    OperationResult getValue()
        const
    {
        switch (valueSide()) {
        case EquationSide::Left: {
            return _left;
        }
        case EquationSide::Right: {
            return _right;
        }
        }

        std::cerr << "unable to get value" << std::endl;
        std::terminate();
    }

    OperationResult getVariable()
        const
    {
        switch (variableSide()) {
        case EquationSide::Left: {
            return _left;
        }
        case EquationSide::Right: {
            return _right;
        }
        }

        std::cerr << "unable to get variable" << std::endl;
        std::terminate();
    }
};

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
    bool _isRoot;
    bool _isHuman;

    void parseName()
    {
        if (_name == "root") {
            _isRoot = true;
        }
        else if (_name == "humn") {
            _isHuman = true;
        }
    }

    OperationResult solve()
        const
    {
        auto leftResult{_leftPtr->value()};
        auto rightResult{_rightPtr->value()};

        // Determine which side has value and which to solve.
        Int valueSide;
        IncompleteOperationPtr solveMe;

        if (std::holds_alternative<Int>(leftResult) &&
            std::holds_alternative<IncompleteOperationPtr>(rightResult)) {
            valueSide = std::get<Int>(leftResult);
            solveMe = std::get<IncompleteOperationPtr>(rightResult);
        }
        else if (std::holds_alternative<IncompleteOperationPtr>(leftResult) &&
                 std::holds_alternative<Int>(rightResult)) {
            valueSide = std::get<Int>(rightResult);
            solveMe = std::get<IncompleteOperationPtr>(leftResult);
        }
        else {
            std::cerr << "unable to determine value and equation sides during solve" << std::endl;
            std::terminate();
        }

        // Solve equations.
        Int solution = valueSide;
        bool last = false;

        while (!last) {
            Operation operation{solveMe->operation()};
            EquationSide equationSide{solveMe->valueSide()};
            Int currValue{std::get<Int>(solveMe->getValue())};
            OperationResult variable{solveMe->getVariable()};

            // If found human mark last.
            if (std::holds_alternative<Human>(variable)) {
                last = true;
            }

            // Reverse operation depending on the size.
            if (equationSide == EquationSide::Left) {
                switch (operation) {
                case Operation::Addition: {
                    solution -= currValue;
                    break;
                }
                case Operation::Subtraction: {
                    solution = currValue - solution;
                    break;
                }
                case Operation::Multiplication: {
                    solution /= currValue;
                    break;
                }
                case Operation::Division: {
                    solution = currValue / solution;
                    break;
                }
                default:
                    std::cerr << "invalid operation" << std::endl;
                    std::terminate();
                }
            }
            else if (equationSide == EquationSide::Right) {
                switch (operation) {
                case Operation::Addition: {
                    solution -= currValue;
                    break;
                }
                case Operation::Subtraction: {
                    solution += currValue;
                    break;
                }
                case Operation::Multiplication: {
                    solution /= currValue;
                    break;
                }
                case Operation::Division: {
                    solution *= currValue;
                    break;
                }
                default:
                    std::cerr << "invalid operation" << std::endl;
                    std::terminate();
                }
            }
            else {
                std::cerr << "invalid equation side during solution" << std::endl;
                std::terminate();
            }

            if (!last) {
                solveMe = std::get<IncompleteOperationPtr>(variable);
            }
        }

        return solution;
    }

    OperationResult compute()
        const
    {
        // If has value simply return value.
        if (_value.has_value()) {
            return *_value;
        }

        OperationResult leftResult{_leftPtr->value()};
        OperationResult rightResult{_rightPtr->value()};

        // If both results are Int then perform computation.
        if (std::holds_alternative<Int>(leftResult) &&
            std::holds_alternative<Int>(rightResult)) {
            OperationFunction opFunc{operationToFunction(*_operation)};
            return opFunc(std::get<Int>(leftResult),
                          std::get<Int>(rightResult));
        }

        // Otherwise, perform partial computation.
        return std::make_shared<IncompleteOperation>(*_operation, leftResult, rightResult);
    }

public:
    Monkey(const String & name,
           const String & leftName,
           Operation operation,
           const String & rightName)
        : _name(name)
        , _leftName(leftName)
        , _operation(operation)
        , _rightName(rightName)
        , _isRoot(false)
        , _isHuman(false)
    {
        parseName();
    }

    Monkey(const String & name, Int value)
        : _name(name)
        , _value(value)
        , _isRoot(false)
        , _isHuman(false)
    {
        parseName();
    }

    auto name() const { return _name; }
    auto leftName() const { return _leftName; }
    auto rightName() const { return _rightName; }

    void setLeftPtr(MonkeyPtr leftPtr) { _leftPtr = leftPtr; }
    void setRightPtr(MonkeyPtr rightPtr) { _rightPtr = rightPtr; }

    OperationResult value()
        const
    {
        if (_isRoot) {
            return solve();
        }

        if (_isHuman) {
            return Human();
        }

        return compute();
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
    std::cout << std::get<Int>(rootMonkey->value()) << std::endl;
    return 0;
}

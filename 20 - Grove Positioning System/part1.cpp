#include <algorithm>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>

using Int = std::int32_t;
using Size = std::size_t;
using String = std::string;

class Node;
using NodePtr = std::shared_ptr<Node>;

class Node
{
private:
    Int _value;
    NodePtr _prev, _next;

public:
    explicit Node(Int value)
        : _value(value)
    {}

    auto value() const { return _value; }
    auto prev() const { return _prev; }
    auto next() const { return _next; }

    void setPrev(const auto & prev) { _prev = prev; }
    void setNext(const auto & next) { _next = next; }
};

std::vector<NodePtr> parseInput(std::istream & is)
{
    std::vector<NodePtr> nodes;

    for (String line; std::getline(is, line);) {
        nodes.emplace_back(std::make_shared<Node>(std::stoi(line)));
    }

    if (nodes.size() > std::numeric_limits<Int>::max()) {
        std::cerr << "too many input numbers" << std::endl;
        std::terminate();
    }

    Int numbersSize{static_cast<Int>(nodes.size())};

    for (Int i = 0; i < numbersSize; ++i) {
        auto currNode = nodes[i];

        if (i > 0) {
            currNode->setPrev(nodes[(i - 1) % numbersSize]);
        }
        currNode->setNext(nodes[(i + 1) % numbersSize]);
    }

    nodes[0]->setPrev(nodes[numbersSize - 1]);

    return nodes;
}

Int pyModulo(const Int a, const Int b)
{
    return (b + (a % b)) % b;
}

void decrypt(std::vector<NodePtr> & nodes)
{
    if (nodes.size() > std::numeric_limits<Int>::max()) {
        std::cerr << "too many nodes to decrypt" << std::endl;
        std::terminate();
    }

    Int numNodes{static_cast<Int>(nodes.size())};
    
    for (const auto & node : nodes) {
        if (node->value() == 0) {
            continue;
        }
        
        Int steps = pyModulo(node->value(), (numNodes - 1));

        if (steps == 0) {
            continue;
        }

        // Unlink the current node.
        auto prevNode = node->prev();
        auto nextNode = node->next();
        prevNode->setNext(nextNode);
        nextNode->setPrev(prevNode);

        // Move forward.
        for (Int i = 0; i < steps; ++i) {
            nextNode = nextNode->next();
        }

        prevNode = nextNode->prev();
        node->setPrev(prevNode);
        node->setNext(nextNode);
        prevNode->setNext(node);
        nextNode->setPrev(node);
    }
}

Int score(const std::vector<NodePtr> & nodes)
{
    if (nodes.size() > std::numeric_limits<Int>::max()) {
        std::cerr << "too many nodes to decrypt" << std::endl;
        std::terminate();
    }

    Int numNodes{static_cast<Int>(nodes.size())};

    auto zeroNode = std::find_if(
        nodes.begin(),
        nodes.end(),
        [](const auto & nodePtr) {
            return nodePtr->value() == 0;
        });

    auto node = *zeroNode;
    auto steps = 1000 % numNodes;
    Int score = 0;

    for (Int i = 0; i < 3; ++i) {
        for (Int j = 0; j < steps; ++j) {
            node = node->next();
        }

        score += node->value();
    }

    return score;
}

int main()
{
    auto nodes{parseInput(std::cin)};
    decrypt(nodes);
    std::cout << score(nodes) << std::endl;
    return 0;
}

#include <deque>
#include <iostream>
#include <numeric>
#include <sstream>
#include <variant>
#include <vector>

using PacketInteger = std::int32_t;

class PacketArray;

using PacketData = std::variant<PacketInteger, PacketArray>;

class PacketArray
    : public std::deque<PacketData>
{
private: 
    using std::deque<PacketData>::deque;

    void parsePacketString(std::string packetString)
    {
        auto iter = std::next(packetString.begin());
        parsePacketString(iter, packetString.end());
    }

    void parsePacketString(std::string::iterator & iter,
                           const std::string::iterator & endIter)
    {
        while (iter != endIter) {
            // Check for digits.
            if ((*iter >= '0') && (*iter <= '9')) {
                std::stringstream ss;

                while ((*iter >= '0') && (*iter <= '9')) {
                    ss << *iter;
                    ++iter;
                }

                PacketInteger pi;
                ss >> pi;

                push_back(pi);
                continue;
            }

            // Check for array opening.
            if (*iter == '[') {
                ++iter;
                PacketArray pa;
                pa.parsePacketString(iter, endIter);
                push_back(pa);
                continue;
            }

            // Check for array closing.
            if (*iter == ']') {
                ++iter;
                break;
            }

            // Check for comma.
            if (*iter == ',') {
                ++iter;
                continue;
            }

            std::cerr << "parsing failure: " << std::string(iter, endIter) << std::endl;
            std::terminate();
        }
    }

public:
    explicit PacketArray(const std::string & packetString)
    {
        parsePacketString(packetString);
    }
};

enum class TriBool { False, True, Indeterminate };

TriBool isRightOrder(PacketArray left, PacketArray right)
{
    while (true) {
        if (left.empty() && !right.empty()) {
            return TriBool::True;
        }
        else if (!left.empty() && right.empty()) {
            return TriBool::False;
        }
        else if (left.empty() && right.empty()) {
            return TriBool::Indeterminate;
        }

        auto leftCurr{left.front()};
        auto rightCurr{right.front()};
        left.pop_front();
        right.pop_front();

        // Both values are integers.
        if ((std::holds_alternative<PacketInteger>(leftCurr)) &&
            (std::holds_alternative<PacketInteger>(rightCurr))) {
            const auto & leftInt = std::get<PacketInteger>(leftCurr);
            const auto & rightInt = std::get<PacketInteger>(rightCurr);
            auto ordering = leftInt <=> rightInt;

            if (ordering < 0) {
                return TriBool::True;
            }
            else if (ordering > 0) {
                return TriBool::False;
            }
        }
        // Both values are lists.
        else if ((std::holds_alternative<PacketArray>(leftCurr)) &&
                 (std::holds_alternative<PacketArray>(rightCurr))) {
            const auto & leftArr = std::get<PacketArray>(leftCurr);
            const auto & rightArr = std::get<PacketArray>(rightCurr);
            auto result{isRightOrder(leftArr, rightArr)};

            if (result != TriBool::Indeterminate) {
                return result;
            }
        }
        // Exactly one value is an integer (left integer).
        else if ((std::holds_alternative<PacketInteger>(leftCurr)) &&
                 (!std::holds_alternative<PacketInteger>(rightCurr))) {
            PacketArray leftArr;
            leftArr.push_back(std::get<PacketInteger>(leftCurr));
            const auto & rightArr = std::get<PacketArray>(rightCurr);
            auto result{isRightOrder(leftArr, rightArr)};

            if (result != TriBool::Indeterminate) {
                return result;
            }
        }
        // Exactly one value is an integer (right integer).
        else if ((!std::holds_alternative<PacketInteger>(leftCurr)) &&
                  (std::holds_alternative<PacketInteger>(rightCurr))) {
            const auto & leftArr = std::get<PacketArray>(leftCurr);
            PacketArray rightArr;
            rightArr.push_back(std::get<PacketInteger>(rightCurr));
            auto result{isRightOrder(leftArr, rightArr)};

            if (result != TriBool::Indeterminate) {
                return result;
            }
        }
        else {
            std::cerr << "unknown state while determining order" << std::endl;
            std::terminate();
        }
    }
}

int processPackets(std::istream & is)
{
    std::vector<int> rightOrderIndices;

    int index = 1;

    do {
        std::string leftStr, rightStr, blank;
        std::getline(is, leftStr);
        std::getline(is, rightStr);
        std::getline(is, blank);

        PacketArray left{leftStr};
        PacketArray right{rightStr};
        auto result{isRightOrder(left, right)};

        if (result == TriBool::True) {
            rightOrderIndices.push_back(index);
        }

        ++index;
    } while(!is.eof());

    return std::accumulate(
        rightOrderIndices.begin(),
        rightOrderIndices.end(),
        0,
        std::plus<int>());
}

int main()
{
    std::cout << processPackets(std::cin) << std::endl;

    return 0;
}

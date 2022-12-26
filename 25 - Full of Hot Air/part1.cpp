#include <algorithm>
#include <array>
#include <cmath>
#include <deque>
#include <iostream>
#include <sstream>
#include <tuple>
#include <vector>

using Int = std::int64_t;
using Size = std::size_t;
using String = std::string;

class SnafuDigit
{
private:
    static constexpr std::array<char, 5> snafuDigits{ '0', '1', '2', '=', '-' };

    char _digit;

    static bool isValidSnafuDigit(const char & c)
    {
        auto digitIter = std::find(snafuDigits.begin(), snafuDigits.end(), c);

        if (digitIter == snafuDigits.end()) {
            return false;
        }

        return true;
    }

public:
    SnafuDigit()
        : _digit('0')
    {}

    explicit SnafuDigit(const char c)
        : _digit(c)
    {
        if (!isValidSnafuDigit(_digit)) {
            std::cerr << "not a valid snafu digit: " << _digit << std::endl;
            std::terminate();
        }
    }

    auto digit() const { return _digit; }

    Int decimalValue()
        const
    {
        switch (_digit) {
            case '2': {
                return 2;
            }
            case '1': {
                return 1;
            }
            case '0': {
                return 0;
            }
            case '-': {
                return -1;
            }
            case '=': {
                return -2;
            }
        }

        std::cerr << "unable to determine snafu digit value: " << _digit << std::endl;
        std::terminate();
    }
};

std::ostream & operator<<(std::ostream & os, const SnafuDigit & snafuDigit)
{
    os << snafuDigit.digit();
    return os;
}

class SnafuNumber
{
private:
    std::vector<SnafuDigit> _snafuDigits;

public:
    explicit SnafuNumber(String str)
    {
        std::transform(
            str.begin(),
            str.end(),
            std::back_inserter(_snafuDigits),
            [](const char & c) {
                return SnafuDigit(c);
            });
    }

    SnafuNumber()
        : SnafuNumber("0")
    {}

    explicit SnafuNumber(const std::vector<SnafuDigit> & snafuDigits)
        : _snafuDigits(snafuDigits)
    {}

    String toString()
    {
        std::stringstream ss;

        for (const SnafuDigit & snafuDigit : _snafuDigits) {
            ss << snafuDigit.digit();
        }

        return ss.str();
    }

    Int toDecimal()
        const
    {
        Int value = 0;
        Int i = 0;

        for (auto digitIter = _snafuDigits.rbegin();
             digitIter != _snafuDigits.rend();
             ++digitIter) {
            value += std::pow(5, i) * digitIter->decimalValue();
            ++i;
        }

        return value;
    }

    static SnafuNumber fromDecimal(Int n)
    {
        std::deque<SnafuDigit> digits;
        SnafuDigit newDigit;

        while (n > 0)
        {
            Int x = n % 5;

            if (x == 3) {
                x = -2;
                newDigit = SnafuDigit('=');
            }
            else if (x == 4) {
                x = -1;
                newDigit = SnafuDigit('-');
            }
            else {
                newDigit = SnafuDigit(std::to_string(x)[0]);
            }

            digits.push_front(newDigit);
            n = std::floor((n - x) / 5);
        }

        return SnafuNumber(std::vector<SnafuDigit>(digits.begin(), digits.end()));
    }

};

std::ostream & operator<<(std::ostream & os, SnafuNumber snafuNumber)
{
    os << snafuNumber.toString();
    return os;
}

int main()
{
    Int total = 0;

    for (String line; std::getline(std::cin, line);) {
        total += SnafuNumber(line).toDecimal();
    }

    std::cout << SnafuNumber::fromDecimal(total) << std::endl;

    return 0;
}

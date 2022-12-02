#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

enum class Shape { rock, paper, scissors };

std::map<char, Shape> char2Shape{
    {'A', Shape::rock},
    {'B', Shape::paper},
    {'C', Shape::scissors},
    {'X', Shape::rock},
    {'Y', Shape::paper},
    {'Z', Shape::scissors},
};

std::map<Shape, int> shape2Score{
    {Shape::rock, 1},
    {Shape::paper, 2},
    {Shape::scissors, 3},
};

#ifdef DEBUG
std::map<Shape, std::string> shape2String{
    {Shape::rock, "rock"},
    {Shape::paper, "paper"},
    {Shape::scissors, "scissors"},
};
#endif

std::vector<std::pair<Shape, Shape>> parseGuide()
{
    std::vector<std::pair<Shape, Shape>> rounds;

    for (std::string line; std::getline(std::cin, line);) {
        std::pair<Shape, Shape> round;

        std::istringstream iss(line);
        std::string left, right;
        iss >> left >> right;
        round.first = char2Shape.at(left[0]);
        round.second = char2Shape.at(right[0]);
        
        rounds.push_back(round);
    }

    return rounds;
}

enum class Outcome { win, lose, draw };

std::map<Outcome, int> outcome2Score{
    {Outcome::lose, 0},
    {Outcome::draw, 3},
    {Outcome::win, 6},
};

#ifdef DEBUG
std::map<Outcome, std::string> outcome2String{
    {Outcome::lose, "lose"},
    {Outcome::draw, "draw"},
    {Outcome::win, "win"},
};
#endif

Outcome judgeRound(Shape opponentShape, Shape myShape)
{
    if (myShape == opponentShape) {
        return Outcome::draw;
    }

    if (((myShape == Shape::rock) && (opponentShape == Shape::scissors)) ||
        ((myShape == Shape::paper) && (opponentShape == Shape::rock)) ||
        ((myShape == Shape::scissors) && (opponentShape == Shape::paper))) {
        return Outcome::win;
    }

    return Outcome::lose;
}

int calcScore(const std::vector<std::pair<Shape, Shape>> & rounds)
{
    int totalScore = 0;

    for (const auto & round : rounds) {
        auto opponentShape = round.first;
        auto myShape = round.second;
        auto outcome = judgeRound(opponentShape, myShape);

#ifdef DEBUG
        std::cout
            << shape2String.at(myShape) << " v " << shape2String.at(opponentShape)
            << " => " << outcome2String.at(outcome)
            << " = " << shape2Score.at(myShape) + outcome2Score.at(outcome)
            << std::endl;
#endif

        totalScore += shape2Score.at(myShape) + outcome2Score.at(outcome);
    }

    return totalScore;
}

int main()
{
    auto rounds = parseGuide();
    auto score = calcScore(rounds);

    std::cout << score << std::endl;

    return 0;
}

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

enum class Outcome { win, lose, draw };

std::map<char, Outcome> char2Outcome{
    {'X', Outcome::lose},
    {'Y', Outcome::draw},
    {'Z', Outcome::win},
};

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

std::map<std::pair<Shape, Outcome>, Shape> solutionTable{
    // (opponentShape, outcome) => myShape
    {std::make_pair(Shape::rock, Outcome::win), Shape::paper},
    {std::make_pair(Shape::rock, Outcome::lose), Shape::scissors},
    {std::make_pair(Shape::paper, Outcome::win), Shape::scissors},
    {std::make_pair(Shape::paper, Outcome::lose), Shape::rock},
    {std::make_pair(Shape::scissors, Outcome::win), Shape::rock},
    {std::make_pair(Shape::scissors, Outcome::lose), Shape::paper},
};

Shape solveRound(Shape opponentShape, Outcome outcome)
{
    if (outcome == Outcome::draw) {
        return opponentShape;
    }

    return solutionTable.at(std::make_pair(opponentShape, outcome));
}

std::vector<std::pair<Shape, Outcome>> parseGuide()
{
    std::vector<std::pair<Shape, Outcome>> rounds;

    for (std::string line; std::getline(std::cin, line);) {
        std::pair<Shape, Outcome> round;

        std::istringstream iss(line);
        std::string left, right;
        iss >> left >> right;
        round.first = char2Shape.at(left[0]);
        round.second = char2Outcome.at(right[0]);
        
        rounds.push_back(round);
    }

    return rounds;
}

int calcScore(const std::vector<std::pair<Shape, Outcome>> & rounds)
{
    int totalScore = 0;

    for (const auto & round : rounds) {
        auto opponentShape = round.first;
        auto outcome = round.second;
        auto myShape = solveRound(opponentShape, outcome);

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

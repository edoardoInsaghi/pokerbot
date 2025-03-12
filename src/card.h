#ifndef CARD_H
#define CARD_H

#include <array>
#include <string>
#include <unordered_map>

/*
 *
 * |   | C | D | H | S |
 * |---|---|---|---|---|
 * | 2 | 0 | 1 | 2 | 3 |
 * | 3 | 4 | 5 | 6 | 7 |
 * | 4 | 8 | 9 | 10| 11|
 * | 5 | 12| 13| 14| 15|
 * | 6 | 16| 17| 18| 19|
 * | 7 | 20| 21| 22| 23|
 * | 8 | 24| 25| 26| 27|
 * | 9 | 28| 29| 30| 31|
 * | T | 32| 33| 34| 35|
 * | J | 36| 37| 38| 39|
 * | Q | 40| 41| 42| 43|
 * | K | 44| 45| 46| 47|
 * | A | 48| 49| 50| 51|
 *
 */

const static std::unordered_map<char, int> rankMap = {
    {'2', 0}, {'3', 1}, {'4', 2}, {'5', 3},  {'6', 4},  {'7', 5},  {'8', 6},
    {'9', 7}, {'T', 8}, {'J', 9}, {'Q', 10}, {'K', 11}, {'A', 12},
};

const static std::unordered_map<char, int> suitMap = {
    {'C', 0}, {'D', 1}, {'H', 2}, {'S', 3},
    {'c', 0}, {'d', 1}, {'h', 2}, {'s', 3},
};

const static std::array<char, 13> rankReverseArray = {'2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K', 'A'};
const static std::array<char, 4> suitReverseArray = {'C', 'D', 'H', 'S'};
const static std::array<std::string, 4> suitReverseArrayPrint = {"♣️", "♦️", "❤️", "♠️"};

class Card {
public:

    int id;
    Card() {}
    Card(int id) : id(id) {}
    Card(std::string name) { id = rankMap.at(name[0]) * 4 + suitMap.at(name[1]); }
    Card(const char name[]) : Card(std::string(name)) {}

    char getRank(void) const { return rankReverseArray[id / 4]; }
    char getSuit(void) const { return suitReverseArray[id % 4]; }

    std::string repr(void) const { return std::string{getRank(), getSuit()}; }

    int get_id() const { return id; }
    operator int() const { return id; }
    operator std::string() const { return repr(); }
};

#endif // CARD_H
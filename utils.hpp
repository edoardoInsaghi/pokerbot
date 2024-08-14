#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <random>
#include <iostream>
#include <array>
#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>
#include <random>
#include <cmath>
#include "card.h"
#include "hash.h"
#include <chrono>
#include <omp.h>


#define MAX_RAISES 3


// Cards Utilities
void print_hand(std::array<Card, 2>& hand);
void print_cards(std::vector<Card>& cards);
void sort_hand(std::array<Card, 2>& hand);
Card draw_card(std::vector<Card>& deck);
void burn_card(std::vector<Card>& deck);

// Player Struct
struct Player {
    std::array<Card, 2> hand;
    double stack;
    int id;
};

// History Struct
struct History {
    std::array<bool, 6> has_folded;
    std::array<int, 6> total_bet;
    std::array<int, 6> last_action;
    std::vector<Card> deck;
    std::vector<Card> board;
    int total_pot, betting_round, big_blind, small_blind, small_blind_value, big_blind_value, current_player, last_raise_counter, total_fold, raise_counter;
    std::random_device rd;
};

// Game Utilities
std::vector<Card> new_deck();
void shuffle_deck(History& H);
void bet(int amount, Player& p, History& H);
// bool equal_bets(History& H);
std::array<bool, 7> get_legal_actions(History& H);
int random_policy(std::array<bool, 5>& legal_actions, History& H);
void go_to_showdown(History& H, std::array<Player, 6>& Players);
void play_hand(std::array<Player, 6>& Players);
int get_cluster_hand(int a, int b);
void get_flop_strenghts();
template<typename T>
void print_vector(std::vector<T>& v) {
    for (auto& e : v) {
        std::cout << e << " ";
    }
    std::cout << std::endl;
}
#endif // UTILS_HPP
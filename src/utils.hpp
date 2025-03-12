#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <random>
#include <unordered_map>
#include <unordered_set>
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
#include <bitset>
#include <cstdint>
//#include <omp.h>
//#include <mpi.h>


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
    std::unordered_map<std::string, int> policy;
};

// History Struct
struct History {
    std::array<Player, 6> players;
    std::array<bool, 6> has_folded;
    std::array<int, 6> total_bet;
    std::array<int, 6> last_action;
    std::vector<Card> deck;
    std::vector<Card> board;
    int total_pot, betting_round, big_blind, small_blind, small_blind_value, big_blind_value, current_player, last_raise_counter, total_fold, raise_counter;
    bool needs_card_dealing, needs_chance;
};
History new_history(std::array<Player, 6>& Players, int small_blind, int small_blind_value);
void print_history(History& H);

// Game Utilities
std::vector<Card> new_deck();
void shuffle_deck(std::vector<Card>& deck);
void bet(int amount, Player& p, History& H);
std::array<bool, 7> get_legal_actions(History& H);
int random_policy(std::array<bool, 5>& legal_actions);
void go_to_showdown(History& H);
// void play_hand(std::array<Player, 6>& Players);
void unroll_game_from_history(History& H);
int get_cluster_hand(int a, int b);

#endif // UTILS_HPP
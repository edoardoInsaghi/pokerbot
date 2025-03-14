#ifndef UTILS_HPP
#define UTILS_HPP

#include <array>
#include <vector>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
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

template <int N>
uint64_t get_hand_id(std::array<uint8_t, N>& hand){ // N is the total number of cards in hand 2 + 3/4/5

    /*
        Each hand can be represented with a 64-bit integer. 
        The first 12 bits represent the cards in hand, they are sorted as this representation is not invariant with respect to permutations.h1
        The next 52 bits store information about the community cards, represented as a one hot bit mask.
    */

    uint8_t h1 = hand[0], h2 = hand[1];
    if (h1 > h2) std::swap(h1, h2);

    uint64_t community_mask = 0;
    for (size_t i=2; i<N; ++i) {
        community_mask |= 1ULL << hand[i];
    }

    return static_cast<uint64_t>(h1) | (static_cast<uint64_t>(h2) << 6) | (community_mask << 12);
}

template <int N>
std::array<uint8_t, N> get_hand_from_string(std::string& hand_string) {

    std::array<uint8_t, N> hand;
    size_t len = hand_string.size();
    for (size_t i=0; i<len; i+=2) {
        hand[i/2] = Card(hand_string.substr(i, 2)).get_id();
    }

    return hand;
}

std::string get_string_from_id(uint64_t id);




// Player Struct
struct Player {
    std::array<Card, 2> hand;
    double stack;
    int id;
    std::unordered_map<std::string, int> policy;
};



// History Struct and Utilities
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
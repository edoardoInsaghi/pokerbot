// Six Players Poker Engine

#include <fstream>
#include "src/utils.hpp"

int main(int argc, char* argv[]) {

/*
    int card1 = 48;
    int card2 = 32;
    int cluster = get_cluster_hand(card1, card2);
    std::cout << "Cluster: " << cluster << std::endl;
*/

    std::ofstream logs("logs.csv");
    logs << "0,1,2,3,4,5\n";

    std::array<Player, 6> Players;
    for (int i=0; i<6; i++) {
        Players[i].id = i;
        Players[i].stack = 0;
        if (i == 5) {logs << Players[i].stack; continue;}
        logs << Players[i].stack << ",";
    }
    logs << "\n";

    int small_blind = 1;
    int small_blind_value = 10;

    /*
    History H = new_history(Players, small_blind, small_blind_value);
    unroll_game_from_history(H);
    std::cout << "Game finished" << std::endl;  
    for (auto p : H.players) {
        std::cout << "Player " << p.id << " stack: " << p.stack << std::endl;
    }
    */

    //return 0;

    int num_games = 10000;
    for (int i=0; i<num_games; i++) {

        small_blind = (small_blind++) % 6;
        History H = new_history(Players, small_blind, small_blind_value);
        unroll_game_from_history(H);

        for (int i=0; i<6; i++) {
            if (i == 5) {logs << H.players[i].stack; continue;}
            logs << Players[i].stack << ",";
            Players = H.players;
        }
        logs << "\n";
    }

    return 0;
}

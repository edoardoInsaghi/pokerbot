// Six Players Poker Engine

#include <fstream>
#include "utils.hpp"

int main(int argc, char* argv[]) {
/*
    int card1 = 48;
    int card2 = 32;
    int cluster = get_cluster_hand(card1, card2);
    std::cout << "Cluster: " << cluster << std::endl;
*/

/*
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

    int num_games = 10000;
    for (int i=0; i<num_games; i++) {
        play_hand(Players);
        for(int i=0; i<6; i++) {
            // std::cout << "Player " << Player.id << " stack: " << Player.stack << " | ";
            if(i == 5) {logs << Players[i].stack; continue;}
            logs << Players[i].stack << ",";
        }
        logs << "\n";
    }
*/

    get_flop_strenghts();

    return 0;
}
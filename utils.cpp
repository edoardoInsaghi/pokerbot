#include "utils.hpp"

#define suits {0, 1, 2, 3} // {Clubs, Diamonds, Hearts, Spades}
#define actions {0, 1, 2, 3, 4, 5, 6} // {None, Check, Call, Fold, RaiseSmall, RaiseMedium, RaiseBig}

////////////////////////////////////////////////////////////////
/*
    CARD UTILITIES
*/
////////////////////////////////////////////////////////////////

void print_hand(std::array<Card, 2>& hand) {
    std::cout << hand[0].repr() << "  " << hand[1].repr() << std::endl;
}

void print_cards(std::vector<Card>& cards) {
    for (auto& card : cards) {
        std::cout << card.repr() << "  ";
    }
    std::cout << std::endl;
}

void sort_hand(std::array<Card, 2>& hand) {
    if (hand[0] < hand[1]) {
        std::swap(hand[0], hand[1]);
    }
}

Card draw_card(std::vector<Card>& deck) {
    Card card = deck.back();
    deck.pop_back();
    return card;
}

void burn_card(std::vector<Card>& deck) {
    deck.pop_back();
}





////////////////////////////////////////////////////////////////
/*
    GAME UTILITIES
*/ 
////////////////////////////////////////////////////////////////

std::vector<Card> new_deck() {
    std::vector<Card> deck;
    for (int i=0; i<52; i++) {
        deck.push_back(Card(i));
    }
    return deck;
}

void shuffle_deck(History& H) {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(H.deck.begin(), H.deck.end(), g);
}

void bet(int amount, Player& p, History& H) {
    p.stack -= amount;
    H.total_pot += amount;
    H.total_bet[p.id] += amount;
}

// bool equal_bets(History& H) { // total_bet[0] might not be defined, check later
//     for (int i=1; i<6; i++) {
//         if (!H.has_folded[i]) {
//             if (H.total_bet[i] != H.total_bet[0]) {
//                 return false;
//             }  
//         }
//     }
//     return true;
// }

std::array<bool, 7> get_legal_actions(History& H) {

    std::array<bool, 7> legal_actions = {false, false, false, false, false};
    int player = H.current_player;
    int max_bet = *std::max_element(H.total_bet.begin(), H.total_bet.end());

    if (H.has_folded[player]) { // Player has already folded, takes NONE action
        legal_actions[0] = true;
        return legal_actions;
    }
    if (H.total_bet[player] == max_bet) { // Player already matches the max bet, can check, raise or fold
        legal_actions[1] = true;
        legal_actions[3] = true;
        legal_actions[4] = (H.raise_counter < MAX_RAISES) ? true : false;
        legal_actions[5] = (H.raise_counter < MAX_RAISES) ? true : false;
        legal_actions[6] = (H.raise_counter < MAX_RAISES) ? true : false;
    }
    else {
        legal_actions[2] = true; // Player does not matches the max bet, can call, raise or fold
        legal_actions[3] = true;
        legal_actions[4] = (H.raise_counter < MAX_RAISES) ? true : false;
        legal_actions[5] = (H.raise_counter < MAX_RAISES) ? true : false;
        legal_actions[6] = (H.raise_counter < MAX_RAISES) ? true : false;
    }
    return legal_actions;
}

int random_policy(std::array<bool, 7>& legal_actions, History& H) {

    std::random_device rd;
    std::vector<int> valid_moves;
    for (int i=0; i<7; ++i) {
        if (legal_actions[i]) {
            valid_moves.push_back(i);
        }
    }
    if (valid_moves.empty()) {
        throw std::runtime_error("No legal move found, go back to debugging. \n");
    }
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, valid_moves.size()-1);

    return valid_moves[dis(gen)];
}


void go_to_showdown(History& H, std::array<Player, 6>& Players) {

    std::array<int, 6> hand_values;
    std::array<bool, 6> winners = {false, false, false, false, false, false};
    int num_winners = 0;
    
    for (int i=0; i<6; i++) {
        if (!H.has_folded[i]) {

            int a = Players[i].hand[0];
            int b = Players[i].hand[1];
            int c = H.board[0];
            int d = H.board[1];
            int e = H.board[2];
            int f = H.board[3];
            int g = H.board[4];

            hand_values[i] = evaluate_7cards(a, b, c, d, e, f, g);
            // std::cout << "Player " << i << " hand value: " << hand_values[i] << std::endl;  
        }
        else {
            hand_values[i] = -1;
        }
    }

    int min_rank = std::numeric_limits<int>::max();
    for (int i=0; i<6; i++) {
        if (!H.has_folded[i] && hand_values[i] < min_rank) {
            min_rank = hand_values[i];
        }
    }
    
    for (int i=0; i<6; i++) {
        if (!H.has_folded[i] && hand_values[i] == min_rank) {
            winners[i] = true;
            num_winners++;
        }
    }

    // std::cout << "Winners: ";
    for (int i=0; i<6; i++) {
        if (winners[i]) {
            // std::cout << i << " ";
            Players[i].stack += (float)H.total_pot / num_winners;
        }
    }
    // std::cout << std::endl;
}


void play_hand(std::array<Player, 6>& Players) {

    std::random_device rd;
    std::mt19937 g(rd());
    std::uniform_int_distribution<int> dist(0, 5);
    std::poisson_distribution<> small_raise(1);
    std::poisson_distribution<> medium_raise(4);
    std::poisson_distribution<> big_raise(8);

    History H;
    H.total_pot = 0;
    H.betting_round = 0;
    H.small_blind_value = 10;
    H.big_blind_value = 20;
    H.small_blind = dist(g);
    H.big_blind = (H.small_blind + 1) % 6;
    H.current_player = 2;
    H.last_raise_counter = 0;
    H.total_fold = 0;
    H.total_bet = {0, 0, 0, 0, 0, 0};
    H.has_folded = {false, false, false, false, false, false};
    H.last_action = {1, 1, 1, 1, 1, 1};

    bet(H.small_blind_value, Players[H.small_blind], H);
    bet(H.big_blind_value, Players[H.big_blind], H);

    H.deck = new_deck();
    shuffle_deck(H);
    for (int i = 0; i < 6; i++) {
        Players[i].hand[0] = draw_card(H.deck);
        Players[i].hand[1] = draw_card(H.deck);
        // sort_hand(H.players[i].hand);
        // std::cout << "Player " << i << " hand: "; 
        // print_hand(H.players[i].hand);
    }
    for (int i=0; i<4; ++i) { // 0: Preflop, 1: Flop, 2: Turn, 3: River

        if (i != H.betting_round) {
            throw std::runtime_error("Betting round and game round mismatch. \n");
        }

        switch (i) {
            case 0:
                // std::cout << "Preflop" << std::endl;
                break;

            case 1:
                // std::cout << "Flop" << std::endl;
                for (int j=0; j<3; ++j) {
                    burn_card(H.deck);
                    H.board.push_back(draw_card(H.deck));
                }
                // print_cards(H.board);
                break;

            case 2:
                // std::cout << "Turn" << std::endl;
                burn_card(H.deck);
                H.board.push_back(draw_card(H.deck));
                // print_cards(H.board);
                break;

            case 3:
                // std::cout << "River" << std::endl;
                burn_card(H.deck);
                H.board.push_back(draw_card(H.deck));
                // print_cards(H.board);
                break;
        }

        H.raise_counter = 0;
        H.last_raise_counter = 0;
        H.current_player = (H.big_blind + 1) % 6;

        while (true) { 

            if (H.last_raise_counter == 6) { 
                H.betting_round++; 
                break; 
            }

            if (H.has_folded[H.current_player]) {
                H.current_player = ++H.current_player % 6;
                continue;
            }

            std::array<bool, 7> legal_actions = get_legal_actions(H);
            // take some action, do later right now just pick random from legal actions
            int action = random_policy(legal_actions, H);

            switch (action) {
                case 0:
                    // std::cout << "Player " << H.current_player << " has done nothing." << std::endl;
                    H.last_raise_counter++;
                    H.last_action[H.current_player] = 0;
                    H.current_player = ++H.current_player % 6;
                    break;

                case 1:
                    // std::cout << "Player " << H.current_player << " has checked." << std::endl;
                    H.last_raise_counter++;
                    H.last_action[H.current_player] = 1;
                    H.current_player = ++H.current_player % 6;
                    break;

                case 2: {
                    // std::cout << "Player " << H.current_player << " has called." << std::endl;
                    H.last_raise_counter++;
                    int to_call = *std::max_element(H.total_bet.begin(), H.total_bet.end()) - H.total_bet[H.current_player];
                    bet(to_call, Players[H.current_player], H);
                    H.last_action[H.current_player] = 2;
                    H.current_player = ++H.current_player % 6;
                    break;
                }

                case 3:
                    // std::cout << "Player " << H.current_player << " has folded." << std::endl;
                    H.last_raise_counter++;
                    H.has_folded[H.current_player] = true;
                    H.total_fold++;
                    if (H.total_fold == 5) {
                        for (int i=0; i<6; i++) {
                            if (!H.has_folded[i]) {
                                // std::cout << "Player " << i << " has won the pot. " << H.total_pot << std::endl;
                                Players[i].stack += H.total_pot;
                                break;
                            }
                        }
                        return;
                    }
                    H.last_action[H.current_player] = 3;
                    H.current_player = ++H.current_player % 6;
                    break;

                case 4: {
                    // std::cout << "Player " << H.current_player << " has raised small." << std::endl;
                    H.raise_counter++;
                    H.last_raise_counter = 1;
                    int to_call = *std::max_element(H.total_bet.begin(), H.total_bet.end()) - H.total_bet[H.current_player];
                    int betting = to_call + small_raise(g) * H.big_blind_value;
                    bet(betting, Players[H.current_player], H);
                    H.last_action[H.current_player] = 4;
                    H.current_player = ++H.current_player % 6;
                    break;
                }

                case 5: {
                    // std::cout << "Player " << H.current_player << " has raised medium." << std::endl;
                    H.raise_counter++;
                    H.last_raise_counter = 1;
                    int to_call = *std::max_element(H.total_bet.begin(), H.total_bet.end()) - H.total_bet[H.current_player];
                    int betting = to_call + medium_raise(g) * H.big_blind_value;
                    bet(betting, Players[H.current_player], H);
                    H.last_action[H.current_player] = 4;
                    H.current_player = ++H.current_player % 6;
                    break;
                }

                case 6: {
                    // std::cout << "Player " << H.current_player << " has raised big." << std::endl;
                    H.raise_counter++;
                    H.last_raise_counter = 1;
                    int to_call = *std::max_element(H.total_bet.begin(), H.total_bet.end()) - H.total_bet[H.current_player];
                    int betting = to_call + small_raise(g) * H.big_blind_value;
                    bet(betting, Players[H.current_player], H);
                    H.last_action[H.current_player] = 4;
                    H.current_player = ++H.current_player % 6;
                    break;
                }
            }
        }
    }

    go_to_showdown(H, Players);
}


int get_cluster_hand(int a, int b) {

    int rank1 = a >> 2;
    int rank2 = b >> 2;
    int suit1 = a & 3;
    int suit2 = b & 3;

    // Determine if suited (if suit1 == suit2, suited = 0xFFFFFFFF, else suited = 0)
    int suited = ~(suit1 ^ suit2);

    // Ensure rank1 <= rank2
    int min_rank = rank1 ^ ((rank1 ^ rank2) & (rank1 - rank2) >> 31);
    int max_rank = rank2 ^ ((rank1 ^ rank2) & (rank1 - rank2) >> 31);

    // Calculate the index to use in cluster_map
    int suited_index = suited & cluster_map_1[min_rank][max_rank];
    int unsuited_index = ~suited & cluster_map_1[max_rank][min_rank];

    return (suited_index | unsuited_index);
}



void get_flop_strenghts() {

    std::vector<Card> deck = new_deck();
    std::ofstream flop_distributions("flop_distributions.csv");

    if (!flop_distributions.is_open()) {
        std::cerr << "Failed to open flop_distributions.csv for writing" << std::endl;
        return;
    }

    // generating all possible combination of five cards in the deck
    #pragma omp parallel for
    for (int ii=0; ii<48; ii++) { 
        for(int jj=ii+1; jj<49; jj++) {
            for (int kk=jj+1; kk<50; kk++) {
                for (int ll=kk+1; ll<51; ll++) {
                    for (int mm=ll+1; mm<52; mm++) {

                        int player_card1 = ii;
                        int player_card2 = jj;
                        int board1 = kk;
                        int board2 = ll;
                        int board3 = mm;
                        std::vector<double> data_points;
                        auto start = std::chrono::high_resolution_clock::now();

                        // simulate all two remaining cards for the table
                        for (int board4=0; board4<52; board4++) {
                            if (board4==ii || board4==jj || board4==kk || board4==ll || board4==mm) continue;
                            for (int board5=0; board5<52; board5++) {
                                if (board5==ii || board5==jj || board5==kk || board5==ll || board5==mm || board5==board4) continue;

                                std::array<int, 3> hand_strength = {0, 0, 0};

                                // simulate all possible two cards for the opponent
                                for (int i = 0; i < 52; i++) {
                                    if (i==ii || i==jj || i==kk || i==ll || i==mm || i==board4 || i==board5) continue;
                                    int opponent_card1 = i;

                                    for (int j = i + 1; j < 52; j++) {
                                        if (j==ii || j==jj || j==kk || j==ll || j==mm || j==board4 || j==board5) continue;
                                        int opponent_card2 = j;

                                        int rank1 = evaluate_7cards(player_card1, player_card2, board1, board2, board3, board4, board5);
                                        int rank2 = evaluate_7cards(opponent_card1, opponent_card2, board1, board2, board3, board4, board5);

                                        if (rank1 < rank2) {
                                            hand_strength[0]++;
                                        } else if (rank1 == rank2) {
                                            hand_strength[1]++;
                                        } else {
                                            hand_strength[2]++;
                                        }
                                    }
                                }

                                double strenght = ((double)(hand_strength[0] + hand_strength[1] / 2.0)) / (hand_strength[0] + hand_strength[1] + hand_strength[2]);
                                data_points.push_back(strenght);
                            }
                        }
                        std::ostringstream hand;
                        hand << player_card1 << "-" << player_card2 << "-" << board1 << "-" << board2 << "-" << board3;
                        flop_distributions << hand.str() << ",";
                        for (int qq=0; qq<data_points.size()-1; qq++) {
                            flop_distributions << data_points[qq] << ",";
                        }
                        flop_distributions << data_points[data_points.size()-1] << std::endl;
                        std::cout << hand.str() << " completed." << std::endl;
                        auto end = std::chrono::high_resolution_clock::now();
                        double time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                        std::cout << hand.str() << " completed. Time needed: " << time << std::endl;
                    }
                }
            }
        }
        std::cout << ii << "/48" << std::endl;
    }
}
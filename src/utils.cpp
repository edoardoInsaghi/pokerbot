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

std::string get_string_from_id(uint64_t id){
    std::string hand = "";
    hand += Card(id & 0x3F).repr();
    hand += Card((id >> 6) & 0x3F).repr();

    uint64_t community_mask = id >> 12;
    for (int i = 0; i < 52; ++i) {
        if (community_mask & (1ULL << i)) {
            hand += Card(i).repr();
        }
    }

    return hand;
}

std::unordered_map<uint64_t, int> load_clusters_flop(std::string filename) {

    int SCALE = static_cast<int>(1e5);
    std::ifstream file(filename);

    std::unordered_map<uint64_t, int> data;
    std::string line;
    int line_count = 0;

    while (std::getline(file, line)) {

        std::stringstream ss(line);

        std::string hand;
        std::getline(ss, hand, ',');

        std::array<uint8_t, 5> cards = get_hand_from_string<5>(hand);
        uint64_t key = get_canon_hand_id<5>(cards);
        
        std::string cluster;
        std::getline(ss, cluster, '\n');

        data.emplace(key, std::stoi(cluster));
        line_count++;
        
        if (line_count % SCALE == 0) {
            std::cout << "Processed " << line_count << " rows." << std::endl;
        }
    }

    std::cout << std::endl;
    std::cout << "Loaded " << data.size() << " rows for flop clusters. \n" << std::endl;
    file.close();
    return data;
}


std::unordered_map<uint64_t, int> load_clusters_turn(std::string filename) {

    int SCALE = static_cast<int>(1e6);
    std::ifstream file(filename);

    std::unordered_map<uint64_t, int> data;
    std::string line;
    int line_count = 0;

    while (std::getline(file, line)) {

        std::stringstream ss(line);

        std::string hand;
        std::getline(ss, hand, ',');

        std::array<uint8_t, 6> cards = get_hand_from_string<6>(hand);
        uint64_t key = get_canon_hand_id<6>(cards);
        
        std::string cluster;
        std::getline(ss, cluster, '\n');

        data.emplace(key, std::stoi(cluster));
        line_count++;
        
        if (line_count % SCALE == 0) {
            std::cout << "Processed " << line_count << " rows." << std::endl;
        }
    }

    std::cout << std::endl;
    std::cout << "Loaded " << data.size() << " rows for turn clusters. \n" << std::endl;
    file.close();
    return data;
}


int cluster_river_hand(std::array<int, 7> cards) {

    float strength = 0;
    int rank1 = evaluate_7cards(cards[0], cards[1], cards[2], cards[3], cards[4], cards[5], cards[6]);
    for (int i=0; i<52; i++) {
        for (int j=i+1; j<52; j++) {
            if (i==cards[0] || i==cards[1] || i==cards[2] || i==cards[3] || i==cards[4] || i==cards[5] || i==cards[6]) continue;
            if (j==cards[0] || j==cards[1] || j==cards[2] || j==cards[3] || j==cards[4] || j==cards[5] || j==cards[6]) continue;
            int rank2 = evaluate_7cards(i, j, cards[2], cards[3], cards[4], cards[5], cards[6]);
            if (rank1 < rank2) {
                strength++;
            } else if (rank1 == rank2) {
                strength += 0.5;
            } 
        }
    }

    strength /= 990;
    return static_cast<int>(strength * 100);
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

void shuffle_deck(std::vector<Card>& deck) {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(deck.begin(), deck.end(), g);
}

void bet(int amount, Player& p, History& H) {
    p.stack -= amount;
    H.total_pot += amount;
    H.total_bet[p.id] += amount;
}


History new_history(std::array<Player, 6>& players, int small_blind, int small_blind_value) {
    History H;
    H.players = players;
    H.deck = new_deck();
    H.board = {};
    H.total_pot = 0;
    H.betting_round = 0;
    H.small_blind_value = small_blind_value;
    H.big_blind_value = 2 * small_blind_value;
    H.small_blind = small_blind;
    H.big_blind = (small_blind + 1) % 6;
    H.current_player = (small_blind + 2) % 6;
    H.last_raise_counter = 0;
    H.total_fold = 0;
    H.total_bet = {0, 0, 0, 0, 0, 0};
    H.has_folded = {false, false, false, false, false, false};
    H.last_action = {0, 0, 0, 0, 0, 0};
    H.needs_card_dealing = true;
    H.needs_chance = true;
    return H;
}

void print_history(History& H) {
    std::cout << "Players: " << std::endl;
    for (int i=0; i<6; i++) {
        std::cout << "Player " << i << " stack: " << H.players[i].stack << std::endl;
        std::cout << "Player " << i << " hand: ";
        print_hand(H.players[i].hand);
    }
    std::cout << "Board: " << std::endl;
    print_cards(H.board);
    std::cout << "Total pot: " << H.total_pot << std::endl;
    std::cout << "Betting round: " << H.betting_round << std::endl;
    std::cout << "Small blind: " << H.small_blind << std::endl;
    std::cout << "Big blind: " << H.big_blind << std::endl;
    std::cout << "Current player: " << H.current_player << std::endl;
    std::cout << "Last raise counter: " << H.last_raise_counter << std::endl;
    std::cout << "Total fold: " << H.total_fold << std::endl;
    std::cout << "Total bet: " << std::endl;
    for (int i=0; i<6; i++) {
        std::cout << H.total_bet[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "Has folded: " << std::endl;
    for (int i=0; i<6; i++) {
        std::cout << H.has_folded[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "Last action: " << std::endl;
    for (int i=0; i<6; i++) {
        std::cout << H.last_action[i] << " ";
    }
    std::cout << std::endl;
}

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

int random_policy(std::array<bool, 7>& legal_actions) {

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


void go_to_showdown(History& H) {

    std::array<int, 6> hand_values;
    std::array<bool, 6> winners = {false, false, false, false, false, false};
    int num_winners = 0;
    
    for (int i=0; i<6; i++) {
        if (!H.has_folded[i]) {

            int a = H.players[i].hand[0];
            int b = H.players[i].hand[1];
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

    std::cout << "Winners: ";
    for (int i=0; i<6; i++) {
        if (winners[i]) {
            std::cout << i << " ";
            H.players[i].stack += (float)H.total_pot / num_winners;
        }
    }
    std::cout << std::endl;
}




void unroll_game_from_history(History& H) {

    std::random_device rd;
    std::mt19937 g(rd());
    std::uniform_int_distribution<int> dist(0, 5);
    std::poisson_distribution<> small_raise(1);
    std::poisson_distribution<> medium_raise(4);
    std::poisson_distribution<> big_raise(8);

    // print_history(H);

    // std::cout << "Last raise counter: " << H.last_raise_counter << std::endl;
    if (H.last_raise_counter == 6) {
        H.betting_round++;
        H.last_raise_counter = 0;
        H.needs_chance = true;
    }
    // std::cout << "Check for betting round ok" << std::endl;

    if (H.betting_round == 4) {
        std::cout << "Going to showdown" << std::endl;
        go_to_showdown(H);
        return;
    }
    // std::cout << "Check for showdown ok" << std::endl;

    if (H.needs_card_dealing) {
        std::cout << "Dealing cards..." << std::endl;
        shuffle_deck(H.deck);
        for (int i = 0; i < 6; i++) {
            H.players[i].hand[0] = draw_card(H.deck);
            H.players[i].hand[1] = draw_card(H.deck);
        }
        H.needs_card_dealing = false;
        std::cout << "Players hands: " << std::endl;
        for (int i=0; i<6; i++) {
            std::cout << "Player " << i << ": ";
            print_hand(H.players[i].hand);
        }
    }
    // std::cout << "Check for card dealing ok" << std::endl;

    if (H.needs_chance) {
        switch (H.betting_round) {
            case 0: // preflop
                std::cout << "Preflop" << std::endl;
                std::cout << "Small blind: " << H.small_blind << std::endl;
                std::cout << "Big blind: " << H.big_blind << std::endl;
                bet(H.small_blind_value, H.players[H.small_blind], H);
                bet(H.big_blind_value, H.players[H.big_blind], H);
                H.needs_chance = false;
                break;

            case 1:
                std::cout << "Flop" << std::endl;
                burn_card(H.deck);
                H.board.push_back(draw_card(H.deck));
                H.board.push_back(draw_card(H.deck));
                H.board.push_back(draw_card(H.deck));
                H.needs_chance = false;
                std::cout << "Board: " << H.board[0].repr() << " " << H.board[1].repr() << " " << H.board[2].repr() << std::endl;
                break;

            case 2:
                std::cout << "Turn" << std::endl;
                burn_card(H.deck);
                H.board.push_back(draw_card(H.deck));
                H.needs_chance = false;
                std::cout << "Board: " << H.board[0].repr() << " " << H.board[1].repr() << " " << H.board[2].repr() << " " << H.board[3].repr() << std::endl;
                break;

            case 3:
                std::cout << "River" << std::endl;
                burn_card(H.deck);
                H.board.push_back(draw_card(H.deck));
                H.needs_chance = false;
                std::cout << "Board: " << H.board[0].repr() << " " << H.board[1].repr() << " " << H.board[2].repr() << " " << H.board[3].repr() << " " << H.board[4].repr() << std::endl;
                break;
        }
    }

    std::array<bool, 7> legal_actions = get_legal_actions(H);
    int action = random_policy(legal_actions);

    switch (action) {
        case 0:
            std::cout << "Player " << H.current_player << " has done nothing." << std::endl;
            H.last_raise_counter++;
            H.last_action[H.current_player] = 0;
            H.current_player = ++H.current_player % 6;
            break;

        case 1:
            std::cout << "Player " << H.current_player << " has checked." << std::endl;
            H.last_raise_counter++;
            H.last_action[H.current_player] = 1;
            H.current_player = ++H.current_player % 6;
            break;

        case 2: {
            std::cout << "Player " << H.current_player << " has called." << std::endl;
            H.last_raise_counter++;
            int to_call = *std::max_element(H.total_bet.begin(), H.total_bet.end()) - H.total_bet[H.current_player];
            bet(to_call, H.players[H.current_player], H);
            H.last_action[H.current_player] = 2;
            H.current_player = ++H.current_player % 6;
            break;
        }

        case 3:
            std::cout << "Player " << H.current_player << " has folded." << std::endl;
            H.last_raise_counter++;
            H.has_folded[H.current_player] = true;
            H.total_fold++;
            if (H.total_fold == 5) {
                for (int i=0; i<6; i++) {
                    if (!H.has_folded[i]) {
                        std::cout << "Player " << i << " has won the pot. " << H.total_pot << std::endl;
                        H.players[i].stack += H.total_pot;
                        return;
                    }
                }
                return; // TODO: questo puzza
            }
            H.last_action[H.current_player] = 3;
            H.current_player = ++H.current_player % 6;
            break;

        case 4: {
            std::cout << "Player " << H.current_player << " has raised small." << std::endl;
            H.raise_counter++;
            H.last_raise_counter = 1;
            int to_call = *std::max_element(H.total_bet.begin(), H.total_bet.end()) - H.total_bet[H.current_player];
            int betting = to_call + small_raise(g) * H.big_blind_value;
            bet(betting, H.players[H.current_player], H);
            H.last_action[H.current_player] = 4;
            H.current_player = ++H.current_player % 6;
            break;
        }

        case 5: {
            std::cout << "Player " << H.current_player << " has raised medium." << std::endl;
            H.raise_counter++;
            H.last_raise_counter = 1;
            int to_call = *std::max_element(H.total_bet.begin(), H.total_bet.end()) - H.total_bet[H.current_player];
            int betting = to_call + medium_raise(g) * H.big_blind_value;
            bet(betting, H.players[H.current_player], H);
            H.last_action[H.current_player] = 4;
            H.current_player = ++H.current_player % 6;
            break;
        }

        case 6: {
            std::cout << "Player " << H.current_player << " has raised big." << std::endl;
            H.raise_counter++;
            H.last_raise_counter = 1;
            int to_call = *std::max_element(H.total_bet.begin(), H.total_bet.end()) - H.total_bet[H.current_player];
            int betting = to_call + big_raise(g) * H.big_blind_value;
            bet(betting, H.players[H.current_player], H);
            H.last_action[H.current_player] = 4;
            H.current_player = ++H.current_player % 6;
            break;
        }
    }

    return unroll_game_from_history(H);
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

////////////////////////////////////////////////////////////////










/*
void play_hand(std::array<Player, 6>& Players) {

    std::random_device rd;
    std::mt19937 g(rd());
    std::uniform_int_distribution<int> dist(0, 5);
    std::poisson_distribution<> small_raise(1);
    std::poisson_distribution<> medium_raise(4);
    std::poisson_distribution<> big_raise(8);

    History H;
    H.players = Players;
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
                burn_card(H.deck);
                for (int j=0; j<3; ++j) { 
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
            int action = random_policy(legal_actions);

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
                    bet(to_call, H.players[H.current_player], H);
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
                                H.players[i].stack += H.total_pot;
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
                    bet(betting, H.players[H.current_player], H);
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
                    bet(betting, H.players[H.current_player], H);
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
                    bet(betting, H.players[H.current_player], H);
                    H.last_action[H.current_player] = 4;
                    H.current_player = ++H.current_player % 6;
                    break;
                }
            }
        }
    }

    go_to_showdown(H);
}
*/
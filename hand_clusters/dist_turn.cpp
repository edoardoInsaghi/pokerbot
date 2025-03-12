#include <fstream>
#include <omp.h>
#include "../src/utils.hpp"

void compute_5_hand_distribution_fast(int card1, int card2, std::ofstream& f1) {

    for (int board1 = 0; board1 < 52; board1++) { // Get all four-card combinations left for the board
        if (board1==card1 || board1==card2) continue;

        for (int board2 = board1 + 1; board2 < 52; board2++) {
            if (board2==card1 || board2==card2 || board2==board1) continue;

            for (int board3 = board2 + 1; board3 < 52; board3++) {
                if (board3==card1 || board3==card2 || board3==board1 || board3==board2) continue;

                for (int board4 = board3 + 1; board4 < 52; board4++) { 
                    if (board4==card1 || board4==card2 || board4==board1 || board4==board2 || board4==board3) continue;

                    std::ostringstream hand;
                    hand << card1 << "-" << card2 << "-" << board1 << "-" << board2 << "-" << board3 << "," << board4 << ",";

                    std::array<float, 47> data_points;
                    int next_free_idx = 0;
                    int card_count = 0;
                    auto start = std::chrono::high_resolution_clock::now();

                    for (int board5 = 0; board5 < 52; board5++) {
                        if (board5==card1 || board5==card2 || board5==board1 || board5==board2 || board5==board3 || board5==board4) continue;

                        int hand_count = 0;
                        std::array<int, 3> hand_strength = {0, 0, 0};
                        int rank1 = evaluate_7cards(card1, card2, board1, board2, board3, board4, board5);
                        
                        // Opponent's hand combinations
                        for (int i = 0; i < 52; i++) { // Get all two-card combinations left for the opponent
                            if (i==card1 || i==card2 || i==board1 || i==board2 || i==board3 || i==board4 || i==board5) continue;
                            int opponent_card1 = i;

                            for (int j = i+1; j < 52; j++) {
                                if (j==card1 || j==card2 || j==board1 || j==board2 || j==board3 || j==board4 || j==board5 || j==i) continue;
                                int opponent_card2 = j;
                                
                                card_count++;
                                hand_count++;

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

                        float strength = ((float)(hand_strength[0] + hand_strength[1] / 2.0)) / (hand_strength[0] + hand_strength[1] + hand_strength[2]);
                        data_points[next_free_idx++] = strength;

                        // std::cout << "Evaluated " << (hand_strength[0] + hand_strength[1] + hand_strength[2]) << " combinations for opponent cards" << std::endl;
                    }

                    f1 << hand.str();
                    // f2 << hand.str();

                    float ev = 0;
                    float var = 0;
                    float std = 0;
                    float skew = 0;
                    float kur = 0;
                    float min = 1;
                    float max = 0;

                    //std::array<float, 100> binned_distribution = {};

                    for (const float& point : data_points) {
                        ev += point;
                        var += point * point;
                        if (point < min) min = point;
                        if (point > max) max = point;

                        //int bin = std::min(static_cast<int>(point * 100), 99);
                        //binned_distribution[bin] = binned_distribution[bin] + 1 / 1081.0;
                    }

                    ev /= 47.0;
                    var = var / 47.0 - ev * ev;
                    std = std::sqrt(var);

                    for (const float& point : data_points) {
                        skew += (point - ev) * (point - ev) * (point - ev);
                        kur += (point - ev) * (point - ev) * (point - ev) * (point - ev);
                    }
                    skew /= (47.0 * std * std * std);
                    kur /= (47.0 * var * var);

                    f1 << ev << "," << var << "," << skew << "," << kur << "," << min << "," << max << std::endl;

                    //for (int qq = 0; qq < 99; qq++) {
                    //    f2 << binned_distribution[qq] << ",";
                    //}
                    //f2 << binned_distribution[99] << std::endl;

                    // int id = omp_get_thread_num();
                    // if (id == 0) {
                    //     auto end = std::chrono::high_resolution_clock::now();
                    //     double time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                    //     std::cout << hand.str() << " completed. Time needed: " << time << std::endl;
                    //     std::cout << "Evaluated " << next_free_idx << " combinations for table cards. For a totoal of " << card_count << " opponent cards"<<std::endl;
                    // }

                }
            }
        }
    }
}


int main(int argc, char* argv[]) {

    /*
    std::ifstream f("results1.csv");
    std::string last_line;
    while (std::getline(f, last_line)) {}
    if (!last_line.empty()) {
        std::istringstream ss(last_line);
        std::string hand;
        std::getline(ss, hand, ',');
        std::istringstream hand_stream(hand);
        std::string card1_str, card2_str;
        std::getline(hand_stream, card1_str, '-');
        std::getline(hand_stream, card2_str, '-');
        start_card1 = std::stoi(card1_str);
        start_card2 = std::stoi(card2_str);
        start_card2++; // Start from the next card2
        if (start_card2 == 52) {
            start_card1++;
            start_card2 = start_card1 + 1;
        }
    }
    */

    #pragma omp parallel 
    {
        int id = omp_get_thread_num();
        std::ofstream f1("features_" + std::to_string(id) + ".csv", std::ios::app);
        //std::ofstream f2("binned_distribution_" + std::to_string(id) + ".csv", std::ios::app);

        #pragma omp for collapse(2)
        for (int card1 = 0; card1 < 52; card1++) {
            for (int card2 = card1 + 1; card2 < 52; card2++) {
                compute_5_hand_distribution_fast(card1, card2, f1);
            }
        }

        std::cout << "Thread " << id << " finished." << std::endl;
        f1.close();
    }
    
    return 0;
}
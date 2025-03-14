#include <fstream>
#include <omp.h>
#include "../src/utils.hpp"

const int NBINS = 20;

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
                    Card c1(card1); 
                    Card c2(card2); 
                    Card b1(board1); 
                    Card b2(board2); 
                    Card b3(board3);
                    Card b4(board4);

                    hand << c1.repr() << c2.repr() << b1.repr() << b2.repr() << b3.repr() << b4.repr() << ",";

                    std::array<float, 47> data_points;
                    int next_free_idx = 0;

                    for (int board5 = 0; board5 < 52; board5++) {
                        if (board5==card1 || board5==card2 || board5==board1 || board5==board2 || board5==board3 || board5==board4) continue;

                        std::array<int, 3> hand_strength = {0, 0, 0};
                        int rank1 = evaluate_7cards(card1, card2, board1, board2, board3, board4, board5);
                        
                        // Opponent's hand combinations
                        for (int i = 0; i < 52; i++) { // Get all two-card combinations left for the opponent
                            if (i==card1 || i==card2 || i==board1 || i==board2 || i==board3 || i==board4 || i==board5) continue;
                            int opponent_card1 = i;

                            for (int j = i+1; j < 52; j++) {
                                if (j==card1 || j==card2 || j==board1 || j==board2 || j==board3 || j==board4 || j==board5 || j==i) continue;
                                int opponent_card2 = j;

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
                    }

                    std::array<std::uint16_t, NBINS> binned_distribution = {0};

                    for (const float& point : data_points) {

                        int bin = std::min(static_cast<int>(point * NBINS), NBINS-1);
                        binned_distribution[bin] = binned_distribution[bin] + 1;
                    }

                    f1 << hand.str();
                    for (int qq=0; qq<NBINS-1; qq++) {
                        f1 << binned_distribution[qq] << ",";
                    }
                    f1 << binned_distribution[NBINS-1] << std::endl;
                }
            }
        }
    }
}


int main(int argc, char* argv[]) {

    #pragma omp parallel 
    {
        int id = omp_get_thread_num();
        std::ofstream f1("distributions_turn_" + std::to_string(id) + ".csv", std::ios::app);

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
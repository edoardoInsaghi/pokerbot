#include "../src/utils.hpp"
#include <omp.h>
#include <iostream>

// FLOP PARAMETERS //
// const int NBINS = 50;
// const int NCARDS = 5;
// const float NSAMPLES = 1081.0;
// const int NROWS = 25989600;
// const int SCALE = int(1e5);
// const int CANON_NROWS = 1361802;

// TURN PARAMETERS //
const int NBINS = 20;
const int NCARDS = 6;
const float NSAMPLES = 47.0;
const int NROWS = 305377800;
const int SCALE = int(1e6);
const int CANON_NROWS = 15111642;


bool check_equal_distributions(const std::array<int, NBINS>& a, const std::array<int, NBINS>& b) {
    for (int i = 0; i < NBINS; ++i) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}


int main(int argc, char** argv) {

    std::vector<uint64_t> keys;
    std::vector<std::array<int, NBINS>> cumulatives;
    //std::ifstream file("canon_distributions_flop.csv");
    std::ifstream file("canon_distributions_turn.csv");

    std::string line;
    for (int i=0; i<CANON_NROWS; ++i) {

        std::string line_copy;
        std::getline(file, line_copy);

        std::stringstream ss(line_copy);
        std::string hand;
        std::getline(ss, hand, ',');

        std::array<uint8_t, NCARDS> cards = get_hand_from_string<NCARDS>(hand);
        uint64_t key = get_hand_id<NCARDS>(cards);

        std::array<int, NBINS> cdf{};
        std::string item;
        for (int j=0; j<NBINS; ++j) {
            std::getline(ss, item, ',');
            cdf[j] = std::stoi(item);
        }

        keys.push_back(key);
        cumulatives.push_back(cdf);

        if (i % SCALE == 0) {
            std::cout << "Processed " << i << " rows." << std::endl;
        }
    }

    int size = cumulatives.size();
    int duplicate_count = 0;
    #pragma omp parallel for collapse(2)
    for (int i=0; i<size; ++i) {
        for (int j=i+1; j<size; ++j) {
            if (check_equal_distributions(cumulatives[i], cumulatives[j])) {
                #pragma omp critical
                {
                    ++duplicate_count;
                    std::cout << "Found duplicate distributions: " << get_string_from_id(keys[i]) << " and " << get_string_from_id(keys[j]) << std::endl;
                }
            }
        }
    }

    std::cout << "Total duplicate distributions: " << duplicate_count << std::endl;

    return 0;
}
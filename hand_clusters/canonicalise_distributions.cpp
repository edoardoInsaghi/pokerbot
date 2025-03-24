#include "Eigen/Dense"
#include "../src/utils.hpp"
#include <omp.h>

// FLOP PARAMETERS
// const int NBINS = 50;
// const int NCARDS = 5;
// const float NSAMPLES = 1081.0;
// const int NROWS = 25989600;
// const int SCALE = int(1e6);

// TURN PARAMETERS
const int NBINS = 20;
const int NCARDS = 6;
const float NSAMPLES = 47.0;
const int NROWS = 305377800;
const int SCALE = int(1e7);


bool same_distributions(const std::array<int, NBINS>& a, const std::array<int, NBINS>& b) {
    for (int i=0; i<NBINS; ++i) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}


uint64_t canonicalize_hand(uint64_t key, size_t community_count) {

    uint8_t hole1 = key & 0x3F;        // Bits 0-5
    uint8_t hole2 = (key >> 6) & 0x3F; // Bits 6-11

    // Extract community cards from bitmask (bits 12-63)
    std::vector<uint8_t> community;
    uint64_t comm_mask = key >> 12;
    for (int i = 0; i < 52; ++i) {
        if (comm_mask & (1ULL << i)) {
            community.push_back(i);
        }
    }

    std::vector<uint8_t> all_cards = {hole1, hole2};
    all_cards.insert(all_cards.end(), community.begin(), community.end());


    uint8_t suit_map[4] = {0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t next_suit = 0;
    for (auto& card : all_cards) {
        uint8_t original_suit = card % 4;
        if (suit_map[original_suit] == 0xFF) {
            suit_map[original_suit] = next_suit++;
        }
        card = (card / 4) * 4 + suit_map[original_suit]; // New canonical card
    }

    hole1 = all_cards[0];
    hole2 = all_cards[1];
    community.clear();
    for (size_t i = 2; i < all_cards.size(); ++i) {
        community.push_back(all_cards[i]);
    }

    // Sort hole and community cards
    if (hole1 > hole2) std::swap(hole1, hole2);
    std::sort(community.begin(), community.end());

    // Pack into 64-bit key
    uint64_t canonical = hole1 | (hole2 << 6);
    uint64_t community_mask = 0;
    for (auto& card : community) {
        community_mask |= 1ULL << card;
    }
    canonical |= (community_mask << 12);

    return canonical;
}


int main(int arcg, char** argv) {

    std::unordered_map<uint64_t, std::array<int, NBINS>> cumulatives;
    // std::ifstream file("distributions_flop.csv");
    std::ifstream file("distributions_turn.csv");

    std::string line;
    #pragma omp parallel for schedule(dynamic) shared(cumulatives)
    for (int i=0; i<NROWS; ++i) {
        std::string line_copy;
        {
            #pragma omp critical
            std::getline(file, line_copy);
        }

        std::stringstream ss(line_copy);
        std::string hand;
        std::getline(ss, hand, ',');

        std::array<uint8_t, NCARDS> cards = get_hand_from_string<NCARDS>(hand);
        uint64_t key = get_hand_id<NCARDS>(cards);

        std::array<int, NBINS> hist{};
        int cdf = 0;
        std::string item;
        for (int j=0; j<NBINS; ++j) {
            std::getline(ss, item, ',');
            cdf += std::stoi(item);
            hist[j] = cdf;
        }

        #pragma omp critical
        cumulatives.emplace(key, std::move(hist));

        if (i % SCALE == 0) {
            std::cout << "Processed " << i << " rows." << std::endl;
        }
    }

    std::cout << "Size of histograms: " << cumulatives.size() << std::endl;
    file.close();

    std::vector<uint64_t> keys;
    for (const auto& pair : cumulatives) {
        keys.push_back(pair.first);
    }

    std::unordered_map<uint64_t, std::array<int, NBINS>> canonicals;

    #pragma omp parallel for schedule(dynamic) shared(canonicals, cumulatives)
    for (size_t idx=0; idx<keys.size(); ++idx) {

        uint64_t key = keys[idx];
        const auto& value = cumulatives.at(key);
        uint64_t canonical = canonicalize_hand(key, 3);

        #pragma omp critical
        {
            if (canonicals.find(canonical) == canonicals.end()) {
                canonicals[canonical] = value;
            }
            else { // this should never happen
                if (!same_distributions(canonicals[canonical], value)) {
                    std::cout << "Distributions differ for hand: "
                              << get_string_from_id(canonical) << std::endl;
                }
            }
        }

        if (idx % SCALE == 0) {
            std::cout << "Processed " << idx << " hands." << std::endl;
        }
    }

    std::cout << "Size of canonical map: " << canonicals.size() << std::endl;

    // std::ofstream f1("canon_distributions_flop.csv");
    std::ofstream f1("canon_distributions_turn.csv");
    int counter = 0;
    for (const auto& pair : canonicals) {
        f1 << get_string_from_id(pair.first);
        for (int i=0; i<NBINS; ++i) {
            f1 << "," << pair.second[i];
        }
        f1 << std::endl;
        ++counter;
        if (counter % (SCALE/10) == 0) {
            std::cout << "Written " << counter  << " rows." << std::endl;
        }
    }
    return 0;
}
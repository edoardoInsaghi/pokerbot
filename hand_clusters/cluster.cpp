#include "Eigen/Dense"
#include "../src/utils.hpp"
#include <omp.h>

using namespace Eigen;

// FLOP PARAMETERS
const int NBINS = 50;
const int NCARDS = 5;
const float NSAMPLES = 1081.0;
const int NROWS = 25989600;
const int SCALE = int(1e5);
const int CANON_NROWS = 25989600;

// TURN PARAMETERS
// const int NBINS = 20;
// const int NCARDS = 6;
// const float NSAMPLES = 47.0;
// const int NROWS = 305377800;
// const int SCALE = int(1e6);
// const int CANON_NROWS = 15111642;


float compute_emd(const std::array<float, NBINS>& a, const std::array<float, NBINS>& b) {
    float emd = 0;
    for (size_t i = 0; i < NBINS; ++i) {
        emd += std::abs(a[i] - b[i]);
    }
    return emd;
}

struct KMeansResult {
    std::vector<int> assignments;
    std::vector<std::array<float, NBINS>> centroids;
};

KMeansResult kmeans_emd(const std::vector<std::pair<uint64_t, 
                        std::array<float, NBINS>>>& data, 
                        int k, int max_iter=1000000, float tolerance = 1e-4) {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, data.size() - 1);
    
    KMeansResult result;
    result.centroids.reserve(k);
    for (int i = 0; i < k; ++i) {
        result.centroids.push_back(data[dist(gen)].second);
    }

    std::vector<int> assignments(data.size());
    std::vector<int> prev_assignments(data.size(), -1);
    int iter = 0;

    while (iter < max_iter) {
        
        # pragma omp parallel for schedule(static)
        for (size_t i=0; i<data.size(); ++i) {
            int best_cluster = 0;
            float min_distance = std::numeric_limits<float>::max();
            
            for (int j = 0; j < k; ++j) {
                float d = compute_emd(data[i].second, result.centroids[j]);
                if (d < min_distance) {
                    min_distance = d;
                    best_cluster = j;
                }
            }
            assignments[i] = best_cluster;
        }

        
        if (assignments == prev_assignments)
            break;
        prev_assignments = assignments;

        std::vector<std::array<float, NBINS>> new_centroids(k, std::array<float, NBINS>{});
        std::vector<int> counts(k, 0);

        for (size_t i = 0; i < data.size(); ++i) {
            int cluster = assignments[i];
            for (int j = 0; j < NBINS; ++j) {
                new_centroids[cluster][j] += data[i].second[j];
            }
            counts[cluster]++;
        }

        for (int j=0; j<k; ++j) {
            if (counts[j] == 0) {
                result.centroids[j] = data[dist(gen)].second;
            } else {
                for (int b=0; b<NBINS; ++b) {
                    result.centroids[j][b] = new_centroids[j][b] / counts[j];
                }
            }
        }

        ++iter;
        std::cout << "Iteration " << iter << " completed." << std::endl;
    }

    result.assignments = std::move(assignments);
    return result;
}


float get_kmeans_within_var(KMeansResult& result, std::vector<std::pair<uint64_t, std::array<float, NBINS>>> data) {
    float within_var = 0;
    for (size_t i=0; i<data.size(); ++i) {
        for (size_t j=0; j<result.centroids.size(); ++j) {
            if (result.assignments[i] == j) {
                within_var += compute_emd(data[i].second, result.centroids[j]);
            }
        }
    }
    return within_var;
}


int main(int argc, char** argv) {

    std::ifstream file("canon_distributions_flop.csv");
    // std::ifstream file("canon_distributions_turn.csv");

    std::vector<std::pair<uint64_t, std::array<float, NBINS>>> data;
    std::string line;
    int line_count = 0;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string hand;
        if (!std::getline(ss, hand, ',')) continue;
        
        std::array<uint8_t, NCARDS> cards = get_hand_from_string<NCARDS>(hand);
        uint64_t key = get_canon_hand_id<NCARDS>(cards);
        
        std::array<float, NBINS> cdf{};
        std::string item;
        for (int j=0; j<NBINS; ++j) {
            if (!std::getline(ss, item, ',')) break;
            cdf[j] = std::stof(item) / NSAMPLES;
        }
        
        data.emplace_back(key, cdf);
        line_count++;
        
        if (line_count % SCALE == 0) {
            std::cout << "Processed " << line_count << " rows." << std::endl;
        }
    }
    file.close();

    std::cout << "Total observations read: " << data.size() << std::endl;

    bool elbow = false;

    if (!elbow) {
        int NUM_CLUSTERS = 16;
        KMeansResult result = kmeans_emd(data, NUM_CLUSTERS);

        std::ofstream out_file("flop_clusters.csv");
        // std::ofstream out_file("turn_clusters.csv");

        if (!out_file.is_open()) {
            throw std::runtime_error("Could not open output file flop_clusters.csv");
        }
        for (size_t i = 0; i < data.size(); ++i) {
            std::string hand_str = get_string_from_id(data[i].first);
            out_file << hand_str << "," << result.assignments[i] << "\n";
        }
        out_file.close(); 
    }
    else {
        int attempts = 3;
        std::vector<int> clusters = {2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};

        for (int cluster : clusters) {
            for (int i=0; i<attempts; ++i) {
                std::cout << "Running k-means with " << cluster << " clusters." << std::endl;
                KMeansResult result = kmeans_emd(data, cluster);
                float within_var = get_kmeans_within_var(result, data);
                std::cout << "Within-cluster variance: " << within_var << std::endl;
            }
            std::cout << std::endl;
        }
    }

    return 0;
}

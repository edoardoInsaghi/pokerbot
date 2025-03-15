#include <vector>
#include <array>
#include <cmath>
#include <random>
#include <unordered_map>
#include <fstream>
#include <string>
#include <sstream>
#include <stdexcept>
#include "Eigen/Dense"
#include <omp.h>
#include "../src/utils.hpp"


using namespace Eigen;
const int NBINS = 50; 
const int NCARDS = 5; 
const float NSAMPLES = 1081.0;
const int NROWS = 25989600;

int compute_emd(const std::array<float, NBINS>& a, const std::array<float, NBINS>& b) {
    int emd = 0;
    
    #pragma omp simd reduction(+:emd)
    for (size_t i=0; i<50; ++i) {
        emd += std::abs(a[i] - b[i]);
    }
    // std::cout << "Hello there" << std::endl;
    return emd;
}


std::vector<size_t> select_landmarks(size_t total, size_t k, unsigned seed=42) {
    std::vector<size_t> landmarks(k);
    std::mt19937 gen(seed);
    std::uniform_int_distribution<size_t> dist(0, total-1);
    for (size_t i = 0; i < k; ++i) landmarks[i] = dist(gen);
    return landmarks;
}



int main(int argc, char** argv) {
 


    std::unordered_map<uint64_t, std::array<float, NBINS>> cumulatives;
    std::ifstream file("distributions_flop.csv");

    std::string line;
    #pragma omp parallel for schedule(static) shared(cumulatives)
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
        
        std::array<float, NBINS> hist{};
        float cdf = 0;
        std::string item;
        for (int j = 0; j < NBINS; ++j) {
            std::getline(ss, item, ',');
            cdf += std::stof(item) / NSAMPLES;
            hist[j] = cdf;
        }
        
        #pragma omp critical
        cumulatives.emplace(key, std::move(hist));

        if (i % 1000000 == 0) {
            std::cout << "Processed " << 1000000 << " rows." << std::endl;
        }
    }

    std::cout << "Size of histograms: " << cumulatives.size() << std::endl;
    
    const size_t k = 1000;
    std::vector<size_t> landmarks = select_landmarks(cumulatives.size(), k);

    // Compute landmark distance matrix
    MatrixXf D(k, k);
    #pragma omp parallel for collapse(2)
    for (size_t i=0; i<k; ++i) {
        for (size_t j=0; j<k; ++j) {
            D(i, j) = compute_emd(cumulatives[landmarks[i]], cumulatives[landmarks[j]]);
        }
    }
    std::cout << "Landmark distance matrix computed." << std::endl;

    // Classical MDS for landmarks
    MatrixXf D_sq = D.array().square();
    MatrixXf J = MatrixXf::Identity(k, k) - MatrixXf::Ones(k, k)/k;
    MatrixXf B = -0.5 * J * D_sq * J;

    SelfAdjointEigenSolver<MatrixXf> solver(B);
    const int d = 2; // Output dimensionality
    MatrixXf Y = solver.eigenvectors().rightCols(d) * 
                 solver.eigenvalues().tail(d).cwiseSqrt().asDiagonal();
    std::cout << "Landmarks projected." << std::endl;

    // Project all points using LMDS
    MatrixXf L_pinv = Y.completeOrthogonalDecomposition().pseudoInverse();
    VectorXf D_sq_mean = D_sq.rowwise().mean();


    MatrixXf embeddings(cumulatives.size(), d);
    #pragma omp parallel for schedule(static)
    for (size_t i=0; i<cumulatives.size(); ++i) {
        VectorXf d_sq(k);
        //std::cout << "Processing point " << i << std::endl;
        for (size_t j=0; j<k; ++j) {
            float dist = compute_emd(cumulatives[i], cumulatives[landmarks[j]]);
            //std::cout << "Distance: " << dist << std::endl;
            d_sq[j] = dist * dist;
        }
        
        VectorXf rhs = -0.5 * (d_sq - D_sq_mean);
        embeddings.row(i) = L_pinv * rhs;
        //std::cout << "Embedding: " << embeddings.row(i) << std::endl;
        if (i % 1000000 == 0) {
            std::cout << "Processed " << 1000000 << " points." << std::endl;
        }
    }
    std::cout << "Embeddings computed." << std::endl;

    std::ofstream f("flop_embeddings.csv");
    for (size_t i=0; i<cumulatives.size(); ++i) {
        f << embeddings(i, 0) << "," << embeddings(i, 1) << std::endl;
    }
    f.close();

    return 0;
}
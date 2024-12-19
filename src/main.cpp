#include <random>
#include <map>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include <chrono>
#include <omp.h>
#include <cstring> // for strcmp
#include <cstdlib> // for atoi, strtoull
#include "math_utils.h"

struct AvalancheStatistics {
  uint64_t P;
  uint64_t G;
  double zero_bits_percentage;
  double mean;
  double stddev;
  std::map<int, int> histogram;
};

AvalancheStatistics avalanche_quality(uint64_t P, uint64_t G) {
  std::random_device rd;
  std::mt19937_64 gen(rd());
  std::uniform_int_distribution<uint64_t> dist(1ULL << 63, ((1ULL << 63) - 1) + (1ULL << 63));

  const int iterations = 10000000;
  std::map<int, int> histogram;
  int sumBitChanges = 0;
  int zero_bits_count = 0;

  for (int i = 1; i <= iterations; i++) {
    uint64_t state = dist(gen);
    uint64_t modified_state = state ^ (1ULL << (gen() % 64));

    uint64_t state_prime = (state * G) % P;
    uint64_t modified_state_prime = (modified_state * G) % P;

    int bitsChanged = math_utils::hamming_distance(state_prime, modified_state_prime);
    histogram[bitsChanged]++;

    if (bitsChanged == 0) {
      zero_bits_count++;
    }

    sumBitChanges += bitsChanged;
  }

  double average = static_cast<double>(sumBitChanges) / iterations;
  double zero_bits_percentage = static_cast<double>(zero_bits_count) / iterations * 100;

  double sumOfSquaredDifferences = 0;
  for (auto &entry : histogram) {
    double diff = entry.first - average;
    sumOfSquaredDifferences += diff * diff * entry.second;
  }
  double stdev = std::sqrt(sumOfSquaredDifferences / iterations);

  return {P, G, zero_bits_percentage, average, stdev, histogram};
}

// Ranking function emphasizing stddev more than mean
bool ranking_function(const AvalancheStatistics& a, const AvalancheStatistics& b) {
  double score_a = 10 * std::abs(a.mean - 32) + 15 * a.stddev + 5 * a.zero_bits_percentage;
  double score_b = 10 * std::abs(b.mean - 32) + 15 * b.stddev + 5 * b.zero_bits_percentage;
  return score_a < score_b;
}

std::string generate_histogram_string(const std::map<int, int>& histogram, int iterations) {
  std::stringstream ss;
  for (auto &entry : histogram) {
    ss << std::setw(2) << entry.first << " bits: ";
    int barLength = entry.second / (iterations/500);
    for (int i = 0; i < barLength; i++) ss << "#";
    if (barLength > 0) ss << " ";
    ss << "(" << entry.second << ")\n";
  }
  return ss.str();
}

void print_usage() {
  std::cout << "Usage: cornice [options]\n"
            << "Options:\n"
            << "  -h                 : show this help message\n"
            << "  [num_samples]      : number of (P,G) pairs to test in normal mode (default: 10)\n"
            << "  -p <prime>         : test a specific prime P\n"
            << "  --p-samples <N>    : when using -p, test N (P,G) pairs for that prime instead of just 1\n\n"
            << "Examples:\n"
            << "  ./cornice              # runs with default 10 samples\n"
            << "  ./cornice 100          # runs with 100 samples\n"
            << "  ./cornice -p 1234567   # runs once for prime 1234567\n"
            << "  ./cornice -p 1234567 --p-samples 5  # runs 5 (P,G) tests for given prime\n";
}

int main(int argc, char* argv[]) {
  int num_samples = 10; // default number of samples
  bool single_prime_mode = false;
  uint64_t prime_input = 0;
  int p_samples = 1; // default number of (P,G) tests when -p is provided

  // Parse arguments
  for (int i = 1; i < argc; i++) {
    if (std::strcmp(argv[i], "-h") == 0) {
      print_usage();
      return 0;
    } else if (std::strcmp(argv[i], "-p") == 0) {
      if (i + 1 < argc) {
        prime_input = std::strtoull(argv[i+1], nullptr, 10);
        if (prime_input == 0) {
          std::cerr << "Error: prime must be a positive integer.\n";
          return 1;
        }
        single_prime_mode = true;
        i++; // skip the next arg as it's the prime
      } else {
        std::cerr << "Error: -p requires a prime number argument.\n";
        return 1;
      }
    } else if (std::strcmp(argv[i], "--p-samples") == 0) {
      if (i + 1 < argc) {
        p_samples = std::atoi(argv[i+1]);
        if (p_samples <= 0) {
          std::cerr << "Error: --p-samples must be a positive integer.\n";
          return 1;
        }
        i++;
      } else {
        std::cerr << "Error: --p-samples requires an integer argument.\n";
        return 1;
      }
    }
    else if (argv[i][0] != '-' && !single_prime_mode) {
      // If no -p provided, attempt to parse as num_samples
      num_samples = std::atoi(argv[i]);
      if (num_samples <= 0) {
        std::cerr << "Error: num_samples must be a positive integer.\n";
        return 1;
      }
    }
  }

  if (single_prime_mode) {
    // Run multiple samples for the given prime, similar to normal mode but all on the same prime
    std::vector<AvalancheStatistics> p_results;

    #pragma omp parallel for
    for (int i = 0; i < p_samples; i++) {
      uint64_t P = prime_input;
      std::vector<uint64_t> factors = math_utils::factorize_source(P);
      uint64_t G = math_utils::find_big_generator(factors, P);
      AvalancheStatistics stat = avalanche_quality(P, G);

      #pragma omp critical
      {
        p_results.push_back(stat);
      }
    }

    // Sort the results based on ranking
    std::sort(p_results.begin(), p_results.end(), ranking_function);

    // Print all results
    std::cout << "Results for P: " << prime_input << " (best first):\n";
    for (size_t i = 0; i < p_results.size(); i++) {
      std::cout << "P: " << p_results[i].P << ", G: " << p_results[i].G
                << ", Zero bits %: " << p_results[i].zero_bits_percentage
                << ", Mean: " << p_results[i].mean
                << ", Stddev: " << p_results[i].stddev << "\n";
      std::cout << "Histogram:\n" << generate_histogram_string(p_results[i].histogram, 10000000) << "\n";
    }

  } else {
    // Normal mode: run num_samples times
    std::vector<AvalancheStatistics> results;

    #pragma omp parallel for
    for (int i = 0; i < num_samples; i++) {
      uint64_t P = math_utils::random_large_prime();
      std::vector<uint64_t> factors = math_utils::factorize_source(P);
      uint64_t G = math_utils::find_big_generator(factors, P);
      AvalancheStatistics stat = avalanche_quality(P, G);

      #pragma omp critical
      {
        results.push_back(stat);
      }
    }

    // Sort results based on the ranking function
    std::sort(results.begin(), results.end(), ranking_function);

    // Get the current time as timestamp
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_tm = std::localtime(&now_time_t);

    // Format the timestamp
    std::ostringstream timestamp;
    timestamp << std::put_time(now_tm, "%Y-%m-%d_%H-%M-%S");

    // Concatenate timestamp to filename
    std::string filename = "avalanche_results_" + timestamp.str() + ".txt";

    std::ofstream file(filename);
    for (const auto &res : results) {
      file << "P: " << res.P << ", G: " << res.G << ", Zero bits %: " << res.zero_bits_percentage;
      file << ", Mean: " << res.mean << ", Stddev: " << res.stddev << "\n";
      file << "Histogram:\n" << generate_histogram_string(res.histogram, 10000000) << "\n";
    }

    // Print the top 50 results to stdout
    std::cout << "Top 50 results:\n";
    for (int i = 0; i < 50 && i < (int)results.size(); i++) {
      std::cout << "P: " << results[i].P << ", G: " << results[i].G << ", Zero bits %: " << results[i].zero_bits_percentage;
      std::cout << ", Mean: " << results[i].mean << ", Stddev: " << results[i].stddev << "\n";
    }
  }

  return 0;
}


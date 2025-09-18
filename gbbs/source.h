#pragma once

#include <iostream>
#include <random>
#include <type_traits>

#include "macros.h"

static const int64_t kRandSeed = 27491095;

template <typename NodeID_, typename rng_t_,
          typename uNodeID_ = typename std::make_unsigned<NodeID_>::type>
class UniDist {
public:
  UniDist(NodeID_ max_value, rng_t_ &rng) : rng_(rng) {
    no_mod_ = rng_.max() == static_cast<uNodeID_>(max_value);
    mod_ = max_value + 1;
    uNodeID_ remainder_sub_1 = rng_.max() % mod_;
    if (remainder_sub_1 == mod_ - 1)
      cutoff_ = 0;
    else
      cutoff_ = rng_.max() - remainder_sub_1;
  }

  NodeID_ operator()() {
    uNodeID_ rand_num = rng_();
    if (no_mod_)
      return rand_num;
    if (cutoff_ != 0) {
      while (rand_num >= cutoff_)
        rand_num = rng_();
    }
    return rand_num % mod_;
  }

private:
  rng_t_ &rng_;
  bool no_mod_;
  uNodeID_ mod_;
  uNodeID_ cutoff_;
};

template <typename ValueT_> class VectorReader {
  std::string filename_;

public:
  explicit VectorReader(std::string filename) : filename_(filename) {
    if (filename == "") {
      std::cout << "No sources filename given (Use -h for help)" << std::endl;
      std::exit(-8);
    }
  }

  std::vector<ValueT_> Read() {
    std::ifstream file(filename_, std::ios::binary);
    if (!file.is_open()) {
      std::cout << "Couldn't open file " << filename_ << std::endl;
      std::exit(-2);
    }

    std::vector<ValueT_> sources;
    while (!file.eof()) {
      ValueT_ source;
      file >> source;
      sources.push_back(source);
    }
    file.close();

    return sources;
  }

  std::vector<ValueT_> ReadSerialized() {
    std::ifstream file(filename_, std::ios::binary);
    if (!file.is_open()) {
      std::cout << "Couldn't open file " << filename_ << std::endl;
      std::exit(-2);
    }

    std::vector<ValueT_> values;
    int64_t num_values; // must be 64-bit value
    file.read(reinterpret_cast<char *>(&num_values), sizeof(num_values));

    values.resize(num_values);
    file.read(reinterpret_cast<char *>(values.data()),
              num_values * sizeof(ValueT_));
    file.close();

    return values;
  }
};

template <typename GraphT_> class SourcePicker {
public:
  explicit SourcePicker(const GraphT_ &g, std::string filename = "")
      : rng_(kRandSeed), udist_(g.n - 1, rng_), g_(g) {
    if (filename != "") {
      VectorReader<gbbs::uintE> reader(filename);
      file_sources_ = reader.Read();
    }
  }

  gbbs::uintE PickNext() {
    // File sources
    if (!file_sources_.empty()) {
      static size_t current = 0;
      return file_sources_[current++];
    }

    gbbs::uintE source;
    do {
      source = udist_();
    } while (g_.get_vertex(source).out_degree() == 0);

    return source;
  }

private:
  std::mt19937_64 rng_;
  UniDist<gbbs::uintE, std::mt19937_64> udist_;
  const GraphT_ &g_;
  std::vector<gbbs::uintE> file_sources_;
};

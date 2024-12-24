#pragma once

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

template <typename GraphT_> class SourcePicker {
public:
  explicit SourcePicker(const GraphT_ &g)
      : rng_(kRandSeed), udist_(g.n - 1, rng_), g_(g) {}

  gbbs::uintE PickNext() {
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
};
// This code is part of the project "Theoretically Efficient Parallel Graph
// Algorithms Can Be Fast and Scalable", presented at Symposium on Parallelism
// in Algorithms and Architectures, 2018.
// Copyright (c) 2018 Laxman Dhulipala, Guy Blelloch, and Julian Shun
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all  copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// Usage:
// numactl -i all ./DeltaStepping -src 10012 -s -m -rounds 3 twitter_wgh_SJ
// flags:
//   required:
//     -src: the source to compute shortest path distances from
//     -w: indicate that the graph is weighted
//   optional:
//     -rounds : the number of times to run the algorithm
//     -c : indicate that the graph is compressed
//     -m : indicate that the graph should be mmap'd
//     -s : indicate that the graph is symmetric

#define WEIGHTED 1

#include "DeltaStepping.h"

namespace gbbs {

template <class Graph>
double DeltaStepping_runner(Graph &G, commandLine P, uintE src) {
  size_t num_buckets = P.getOptionLongValue("-nb", 32);
  double delta = P.getOptionDoubleValue("-delta", 1.0);

  std::cout << "\n### Application: DeltaStepping" << std::endl;
  std::cout << "### Graph: " << P.getArgument(0) << std::endl;
  std::cout << "### Threads: " << num_workers() << std::endl;
  std::cout << "### n: " << G.n << std::endl;
  std::cout << "### m: " << G.m << std::endl;
  std::cout << "### Params: -src = " << src << " -delta = " << delta
            << " -nb (num_buckets) = " << num_buckets << std::endl;
  std::cout << "### ------------------------------------" << std::endl;

  if (num_buckets != (((uintE)1) << parlay::log2_up(num_buckets))) {
    std::cout << "Please specify a number of buckets that is a power of two"
              << "\n";
    exit(-1);
  }
  timer t;
  t.start();
  auto dists = DeltaStepping(G, src, delta, num_buckets);
  double tt = t.stop();

  std::cout << "### Running Time: " << tt << std::endl;

  using W = typename Graph::weight_type;
  using Distance =
      typename std::conditional<std::is_same<W, gbbs::empty>::value, uintE,
                                W>::type;
  constexpr Distance kMaxWeight = std::numeric_limits<Distance>::max();

  auto not_max_cmp = [&](Distance a, Distance b) {
    if (b == kMaxWeight)
      return false;
    if (a == kMaxWeight)
      return true;
    return a < b;
  };

  auto not_max = [&](Distance e) { return e != kMaxWeight; };

  auto longest_distance = *parlay::max_element(dists, not_max_cmp);
  auto reached = parlay::count_if(dists, not_max);
  std::cout << "Nodes reached: " << reached << std::endl;
  std::cout << "Longest distance: " << longest_distance << std::endl;

  return tt;
}

} // namespace gbbs

// generate_float_main(gbbs::DeltaStepping_runner, false);
generate_weighted_traversal_main(gbbs::DeltaStepping_runner, false);

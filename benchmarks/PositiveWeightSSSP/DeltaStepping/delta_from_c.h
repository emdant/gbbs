#pragma once

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <type_traits>

#include "gbbs/gbbs.h"

/*
Derive delta from a graph-independent constant C:

    delta = C * mean_edge_weight / average_degree

`average_degree` is the number of arcs stored per vertex, G.m / G.n. A symmetric
graph stores every edge in both directions, so this is 2m/n, which is what the
graph statistics tool reports as "Average degree". `mean_edge_weight` is taken
over the same stored arcs; a symmetric edge contributes twice with the same
weight, so the mean is unaffected.

TIMING
------
DeltaSelector::Get() is called from inside the timed region of the runner, once
per round of every source, immediately before DeltaStepping allocates its
buckets. That is the whole point: the cost of choosing delta is charged to the
algorithm.

To take it back out of the timer, pass -delta-outside-timer. Warmup() then
computes the value once before the timer starts and Get() returns the cached
value. The call site never moves, so switching between the two costs nothing.
*/

namespace gbbs {

template <class Graph> double MeanEdgeWeight(Graph &G) {
  using W = typename Graph::weight_type;
  // Graph::reduceEdges wants an old-style gbbs monoid, so reduce in two levels
  // with parlay monoids instead. The compressed vertex takes both by non-const
  // reference, so they have to be named lvalues.
  auto map_f = [](const uintE &, const uintE &, const W &w) -> double {
    return static_cast<double>(w);
  };
  auto monoid = parlay::plus<double>();
  auto per_vertex = parlay::delayed_seq<double>(G.n, [&](size_t i) {
    return G.get_vertex(i).out_neighbors().reduce(map_f, monoid);
  });
  return parlay::reduce(per_vertex) / static_cast<double>(G.m);
}

class DeltaSelector {
public:
  DeltaSelector() = default;
  DeltaSelector(double fixed_delta, double c, bool use_c, bool outside_timer)
      : fixed_delta_(fixed_delta), c_(c), use_c_(use_c),
        outside_timer_(outside_timer) {}

  // Build from the command line. -C selects the derived mode.
  static DeltaSelector FromCommandLine(commandLine &P) {
    return DeltaSelector(P.getOptionDoubleValue("-delta", 1.0),
                         P.getOptionDoubleValue("-C", 0.0), P.getOption("-C"),
                         P.getOption("-delta-outside-timer"));
  }

  // Call before the timer starts. Only does work under -delta-outside-timer.
  template <class Graph> void Warmup(Graph &G) {
    if (use_c_ && outside_timer_) {
      cached_ = Compute(G);
      have_cached_ = true;
    }
  }

  // Call inside the timed region, just before the buckets are built.
  template <class Graph> double Get(Graph &G) const {
    if (!use_c_) {
      last_ = fixed_delta_;
    } else if (have_cached_) {
      last_ = cached_;
    } else {
      last_ = Compute(G);
    }
    return last_;
  }

  // Report the delta the last Get() handed out. Call outside the timer.
  void PrintLast() const {
    if (use_c_)
      std::cout << "### Derived delta: " << last_ << " (C = " << c_ << ")"
                << std::endl;
  }

  bool use_c() const { return use_c_; }
  double c() const { return c_; }

private:
  template <class Graph> double Compute(Graph &G) const {
    using W = typename Graph::weight_type;
    const double average_degree =
        static_cast<double>(G.m) / static_cast<double>(G.n);
    double delta = c_ * MeanEdgeWeight(G) / average_degree;

    // An integer-weighted graph has no use for a delta below 1.
    if (std::is_integral<W>::value)
      delta = std::max(1.0, std::round(delta));

    return delta;
  }

  double fixed_delta_ = 1.0;
  double c_ = 0.0;
  bool use_c_ = false;
  bool outside_timer_ = false;
  double cached_ = 0.0;
  bool have_cached_ = false;
  mutable double last_ = 0.0;
};

} // namespace gbbs

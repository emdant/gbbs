#include <type_traits>

#include "gbbs/gbbs.h"

namespace gbbs {
template <class Graph> double print_edges(Graph &G, gbbs::commandLine P) {
  timer t;
  t.start();

  std::cout << "n = " << G.n << ", m = " << G.m << std::endl;

  auto n = G.n;
  for (size_t u = 0; u < n; u++) {
    if (G.get_vertex(u).out_degree() > 0) {
      auto neigh = G.get_vertex(u).out_neighbors();
      auto min = neigh.degree < 5 ? neigh.degree : 5;

      for (size_t i = 0; i < min; i++) {
        std::cout << i << "-th neighbor: " << neigh.get_neighbor(i)
                  << ", weight: " << neigh.get_weight(i) << std::endl;
      }
      break;
    }
  }

  double tt = t.stop();

  std::cout << "### Running Time: " << tt << std::endl;
  return tt;
}

} // namespace gbbs

generate_weighted_main(gbbs::print_edges, false)

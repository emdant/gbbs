#include <type_traits>

#include "gbbs/gbbs.h"

namespace gbbs {
template <class Graph> double print_edges(Graph &G, gbbs::commandLine P) {
  timer t;
  t.start();
  if constexpr (std::is_same_v<Graph, gbbs::asymmetric_graph<asymmetric_vertex,
                                                             gbbs::intE>>) {
    auto n = G.n;
    for (size_t u = 0; u < n; u++) {
      if (G.get_vertex(u).out_degree() > 0) {
        auto neigh = G.get_vertex(u).out_neighbors();

        for (size_t i = 0; i < 1; i++) {
          std::cout << neigh.get_neighbor(i) << "," << neigh.get_weight(i)
                    << " ";
        }
        std::cout << std::endl;
        break;
      }
    }
  }
  double tt = t.stop();

  std::cout << "### Running Time: " << tt << std::endl;
  return tt;
}

} // namespace gbbs

generate_weighted_main(gbbs::print_edges, false)

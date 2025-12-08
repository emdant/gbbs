#include <algorithm>
#include <tuple>
#include <vector>

#include "graph.h"
#include "macros.h"
#include "source.h"
#include "vertex.h"

template <typename weight_type>
void replace_weights(
    gbbs::symmetric_graph<gbbs::symmetric_vertex, weight_type> &g,
    std::string weights_file) {
  VectorReader<weight_type> reader(weights_file);
  std::vector<weight_type> weights = reader.ReadSerialized();

  gbbs::parallel_for(0, g.m,
                     [&](size_t i) { std::get<1>(g.e0[i]) = weights[i]; });
}

template <typename weight_type>
void replace_weights(
    gbbs::asymmetric_graph<gbbs::asymmetric_vertex, weight_type> &g,
    std::string weights_file) {
  using neighbor_type =
      typename gbbs::asymmetric_graph<gbbs::asymmetric_vertex,
                                      weight_type>::neighbor_type;

  VectorReader<weight_type> reader(weights_file);
  std::vector<weight_type> weights = reader.ReadSerialized();
  gbbs::parallel_for(
      0, g.m, [&](size_t i) { std::get<1>(g.out_edges[i]) = weights[i]; });

  auto compare_node = [](const neighbor_type &a, const neighbor_type &b) {
    return std::get<0>(a) < std::get<0>(b);
  };

  gbbs::parallel_for(0, g.n, [&](size_t u) {
    gbbs::vertex_data vd = g.v_out_data[u];
    for (gbbs::uintE i = 0; i < vd.degree; i++) {
      neighbor_type nb = g.out_edges[vd.offset + i];
      gbbs::uintE v = std::get<0>(nb);
      weight_type w = std::get<1>(nb);

      gbbs::vertex_data v_data = g.v_in_data[v];
      neighbor_type *v_in_begin = g.in_edges + v_data.offset;
      neighbor_type *v_in_end = g.in_edges + v_data.offset + v_data.degree;

      neighbor_type dummy{u, static_cast<weight_type>(1)};
      auto it = std::lower_bound(v_in_begin, v_in_end, dummy, compare_node);
      std::get<1>(*it) = w;
    }
  });
}
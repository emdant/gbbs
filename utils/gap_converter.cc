#include <exception>
#include <iostream>
#include <vector>

#include "converter.h"
#include "gbbs/graph_io.h"
#include "gbbs/helpers/parse_command_line.h"

namespace gbbs {
namespace {

int RunGAPConverter(int argc, char *argv[]) {
#ifdef USE_FLOAT
  typedef float weight_type;
#else
  typedef uintE weight_type;
#endif

  const std::string kCommandLineHelpString{
      "Usage: ./gap_converter [-s] [-w] -i <input file> -o <output file>\n"
      "\n"
      "Converts a graph in edge-list format to the adjacency graph format "
      "that\n"
      "GBBS uses.\n"
      "Arguments:\n"
      "  -i <filename>: Path to the input GAP file.\n"
      "  -o <filename>: Path to the output GBBS file.\n"
      "Optional arguments:\n"
      "  -w: Use this flag if the edge list is weighted with 32-bit integers.\n"
      "floats.\n"};
  const std::string kInputFlag{"-i"};
  const std::string kOutputFlag{"-o"};

  const commandLine parameters{argc, argv, kCommandLineHelpString};
  const char *const input_file{parameters.getOptionValue(kInputFlag)};
  const char *const output_file{parameters.getOptionValue(kOutputFlag)};
  const bool weighted{parameters.getOption("-w")};

  bool is_symmetric_graph = false;

  if (argc < 2 || std::string(argv[1]) == "-h" ||
      std::string(argv[1]) == "--help") {
    std::cout << kCommandLineHelpString << '\n';
    return 0;
  }

  if (input_file == nullptr || output_file == nullptr) {
    std::cerr << "ERROR: Please specify the input GAP binary file with the '"
              << kInputFlag << "' flag and the output file with the '"
              << kOutputFlag << "' flag.\n";
    std::terminate();
  }
  std::ofstream out(output_file, std::ofstream::out | std::ios::binary);
  std::cout << "opened out file" << std::endl;

  is_symmetric_graph = !gbbs_io::is_gap_graph_directed(input_file);
  std::cout << "GAP graph is "
            << (is_symmetric_graph ? "undirected" : "directed") << std::endl;

  if (weighted) {
    if (is_symmetric_graph) {
      auto sym_weighted =
          gbbs_io::read_gap_weighted_symmetric_graph<weight_type>(input_file,
                                                                  true, true);
      std::cout << "created in memory graph" << std::endl;

      int skipped = 0;
      auto first_ten = std::min(10ul, sym_weighted.n);
      for (uintE i = 0, v = 0; i < first_ten && skipped < 100; i++, v++) {
        if (sym_weighted.get_vertex(i).out_neighbors().degree == 0) {
          std::cout << "Vertex " << v << " has no out-neighbors." << std::endl;
          i--;
          skipped++;
          continue;
        }
        auto neigh = sym_weighted.get_vertex(v).out_neighbors();
        auto first_five_neighs = std::min(5u, neigh.degree);
        std::cout << "Vertex " << v << " has out-degree: " << neigh.degree
                  << std::endl;
        for (uintE j = 0; j < first_five_neighs; j++) {

          std::cout << "neigh(" << j << "): " << neigh.get_neighbor(0);
          std::cout << " - edge weight: " << neigh.get_weight(j) << std::endl;
        }
      }

      binary_format::write_graph_binary_format(sym_weighted, out,
                                               is_symmetric_graph);
      std::cout << "graph outputted" << std::endl;

    } else {
      auto asym_weighted =
          gbbs_io::read_gap_weighted_asymmetric_graph<weight_type>(input_file,
                                                                   true, true);
      std::cout << "created in memory graph" << std::endl;

      auto first_ten = std::min(10ul, asym_weighted.n);
      for (uintE i = 0, v = 0; i < first_ten; i++, v++) {
        if (asym_weighted.get_vertex(i).out_neighbors().degree == 0) {
          std::cout << "Vertex " << v << " has no out-neighbors." << std::endl;
          i--;
          continue;
        }
        auto neigh = asym_weighted.get_vertex(v).out_neighbors();
        auto first_five_neighs = std::min(5u, neigh.degree);
        std::cout << "Vertex " << v << " has out-degree: " << neigh.degree
                  << std::endl;
        for (uintE j = 0; j < first_five_neighs; j++) {

          std::cout << "neigh(" << j << "): " << neigh.get_neighbor(0);
          std::cout << " - edge weight: " << neigh.get_weight(j) << std::endl;
        }
      }
      binary_format::write_graph_binary_format(asym_weighted, out,
                                               is_symmetric_graph);
      std::cout << "graph outputted" << std::endl;
    }

  } else {
    std::cout << "only integer-weighted GAP graphs are supported" << std::endl;
  }
  return 0;
}

} // namespace
} // namespace gbbs

int main(int argc, char *argv[]) { return gbbs::RunGAPConverter(argc, argv); }

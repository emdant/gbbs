#include <exception>
#include <vector>

#include "converter.h"
#include "gbbs/graph_io.h"
#include "gbbs/helpers/parse_command_line.h"

namespace gbbs {
namespace {

int RunGAPConverter(int argc, char *argv[]) {
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
      "  -s: Treat the edges as a list of undirected edges and create a\n"
      "      symmetric graph. (Without this flag, the edges are treated as a\n"
      "      list of directed edges.)\n"
      "  -w: Use this flag if the edge list is weighted with 32-bit integers.\n"
      "floats.\n"};
  const std::string kInputFlag{"-i"};
  const std::string kOutputFlag{"-o"};

  const commandLine parameters{argc, argv, kCommandLineHelpString};
  const char *const input_file{parameters.getOptionValue(kInputFlag)};
  const char *const output_file{parameters.getOptionValue(kOutputFlag)};
  const bool is_symmetric_graph{parameters.getOption("-s")};
  const bool integer_weighted{parameters.getOption("-w")};

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

  if (integer_weighted) {
    if (is_symmetric_graph) {
      auto sym_weighted = gbbs_io::read_gap_weighted_symmetric_graph<uintE>(
          input_file, true, true);
      std::cout << "created in memory graph" << std::endl;
      binary_format::write_graph_binary_format(sym_weighted, out,
                                               is_symmetric_graph);
      std::cout << "graph outputted" << std::endl;

    } else {
      auto asym_weighted = gbbs_io::read_gap_weighted_asymmetric_graph<uintE>(
          input_file, true, true);
      std::cout << "created in memory graph" << std::endl;
      auto neigh = asym_weighted.get_vertex(0).out_neighbors();
      std::cout << neigh.get_neighbor(0) << "," << neigh.get_weight(0)
                << std::endl;

      binary_format::write_graph_binary_format(asym_weighted, out,
                                               is_symmetric_graph);
      std::cout << "graph outputted" << std::endl;
    }

  } else {
    if (is_symmetric_graph) {

    } else {
      // gbbs_io::read_gap_unweighted_asymmetric_graph<int32_t>(input_file,
      // true, true);
    }
  }
  return 0;
}

} // namespace
} // namespace gbbs

int main(int argc, char *argv[]) { return gbbs::RunGAPConverter(argc, argv); }

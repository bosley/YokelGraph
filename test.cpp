#include "YokelGraph/Graph.hpp"

#include <string>
#include <fmt/format.h>

static constexpr bool SHOW_GRAPH_NUMBER = false;
static constexpr std::size_t TEST_ITERATIONS = 20;

namespace {

using test_graph_t = yokel::graph_c<std::string, std::string>;
using test_data_t = test_graph_t::source_s;

struct test_graph_s {
  struct path_s {
    std::string from;
    std::string to;
    std::size_t expected_distance{0};
    std::vector<std::string> expected_path;
    bool possible{true};
  };
  test_data_t data;
  std::vector<path_s> paths;
  bool contains_cycles{true};
};

test_graph_s graph_one() {
/*
           ┌────────┐
           │        │
           │   G─┐  │
           │   ▲ │  ▼
      ┌──► A ──┘ │  B ◄──┐
      │    │     │       │
      E ◄──┘     └─────► F ◄┐
      │                  │  │
      └──► C ◄───── D ◄──┘  │
           │                │
           └─► H ───────────┘
*/
  return test_graph_s{
    {
      {"A", "B", "C", "D", "E", "F", "G", "H"},
      {
        {"A", "G", "A->G"},
        {"A", "E", "A->E"},
        {"A", "B", "A->B"},
        {"E", "A", "E->A"},
        {"E", "C", "E->C"},
        {"C", "H", "C->H"},
        {"G", "F", "G->F"},
        {"H", "F", "H->F"},
        {"D", "C", "D->C"},
        {"F", "B", "F->B"},
        {"F", "D", "F->D"},
      }
    },
    {
      {"E", "C", 1, {"E->C"}},
      {"A", "C", 2, {"A->E", "E->C"}},
      {"G", "H", 4, {"G->F", "F->D", "D->C", "C->H"}},
      {"F", "H", 3, {"F->D", "D->C", "C->H"}},
      {"F", "A", 0, {}, false},
    }
  };
}

test_graph_s graph_two() {
/*
              ┌────┐
              │    │
        ┌────►B◄───┘
        │     │
        │     │
        A     └───►C
                   │
                   │
        E◄────D◄───┘
*/
  return test_graph_s{
    {
      {"A", "B", "C", "D", "E"},
      {
        {"A", "B", "A->B"},
        {"B", "B", "B->B"},
        {"B", "C", "B->C"},
        {"C", "D", "C->D"},
        {"D", "E", "D->E"},
      }
    },
    {
      {"B", "B", 1, {"B->B"}},
      {"B", "A", 0, {}, false},
      {"A", "C", 2, {"A->B", "B->C"}},
      {"A", "E", 4, {"A->B", "B->C", "C->D", "D->E"}},
    }
  };
} 

test_graph_s graph_three() {
/*
            ┌────┐
            │    │
      ┌────►B◄───┘
      │     │
      │     │
      A     └───►C────┐
                 │    │
                 │    │
      E◄────D◄───┘    │
      ▲               │
      │               │
      └───────────────┘
*/
  return test_graph_s{
    {
      {"A", "B", "C", "D", "E"},
      {
        {"A", "B", "A->B"},
        {"B", "B", "B->B"},
        {"B", "C", "B->C"},
        {"C", "D", "C->D"},
        {"D", "E", "D->E"},
        {"C", "E", "C->E"},
      }
    },
    {
      {"B", "B", 1, {"B->B"}},
      {"A", "C", 2, {"A->B", "B->C"}},
      {"A", "E", 3, {"A->B", "B->C", "C->E"}},
      {"X", "Y", 0, {}, false}, // no exist
      {"Y", "Z", 0, {}, false}, // no exist
      {"Z", "Z", 0, {}, false}, // no exist
    }
  };
} 

test_graph_s graph_four() {
/*
            ┌────────────────────────┐
            │                        │
            ▼                        │
            B  ◄──────── C ◄──────── D ◄────────┐
                                                │
    A         ┌───────► E────────────► F ───────┘
    │         │         │
    │         │         │
    └───────► G ◄───────┘
*/
  return test_graph_s{
    {
      {"A", "B", "C", "D", "E", "F", "G"},
      {
        {"A", "G", "A->G"},
        {"G", "E", "G->E"},
        {"E", "G", "E->G"},
        {"E", "F", "E->F"},
        {"F", "D", "F->D"},
        {"D", "C", "D->C"},
        {"D", "B", "D->B"},
        {"C", "B", "C->B"},
      }
    },
    {
      {"A", "B", 5, {"A->G", "G->E", "E->F", "F->D", "D->B"}},
      {"D", "B", 1, {"D->B"}},
      {"E", "G", 1, {"E->G"}},
      {"B", "A", 0, {}, false},
      {"B", "A", 0, {}, false},
      {"X", "Y", 0, {}, false}, // no exist
      {"Y", "Z", 0, {}, false}, // no exist
      {"Z", "Z", 0, {}, false}, // no exist
    }
  };
} 

test_graph_s graph_five() {
/*
    A────►C
    │     │
    └┐    └─►F
     ▼       │
     B─────┐ └──►G
     │     │     │
     ▼     ▼     └──►H
     D     E
*/
  return test_graph_s{
    {
      {"A", "B", "C", "D", "E", "F", "G", "H"},
      {
        {"A", "B", "A->B"},
        {"B", "D", "B->D"},
        {"B", "E", "B->E"},
        {"A", "C", "A->C"},
        {"C", "F", "C->F"},
        {"F", "G", "F->G"},
        {"G", "H", "G->H"},
      }
    },
    {
      {"A", "H", 4, {"A->C", "C->F", "F->G", "G->H"}},
      {"A", "D", 2, {"A->B", "B->D"}},
      {"A", "E", 2, {"A->B", "B->E"}},
      {"F", "H", 2, {"F->G", "G->H"}},
      {"E", "H", 0, {}, false},
      {"H", "A", 0, {}, false},
      {"X", "Y", 0, {}, false}, // no exist
      {"Y", "Z", 0, {}, false}, // no exist
      {"Z", "Z", 0, {}, false}, // no exist
    },
    false // no cycles
  };
}

test_graph_s graph_six() {
/*
                  ┌──────────────────┐
                  │                  ▼
       ┌─────────►B───────┐          I
       │          ▲       ▼
       │          │       E◄───┐
       │          │       │    │
       │          C◄──────┘    │
       │                       └───H◄─┐
       │                              │
   ┌──►A─────────────────►F───────┐   │
   │   │                  ▲       ▼   │
   │   │                  │       G───┘
   │   └─────────►D───────┘
   │              │
   └──────────────┘
*/
  return test_graph_s{
    {
      {"A", "B", "C", "D", "E", "F", "G", "H", "I"},
      {
        {"A", "F", "A->F"},
        {"A", "D", "A->D"},
        {"A", "B", "A->B"},
        {"D", "A", "D->A"},
        {"D", "F", "D->F"},
        {"F", "G", "F->G"},
        {"G", "H", "G->H"},
        {"H", "E", "H->E"},
        {"E", "C", "E->C"},
        {"C", "B", "C->B"},
        {"B", "I", "B->I"},
        {"B", "E", "B->E"},
      }
    },
    {
      {"A", "I", 2, {"A->B", "B->I"}},
      {"D", "I", 3, {"D->A", "A->B", "B->I"}},
      {"F", "I", 6, {"F->G", "G->H", "H->E", "E->C", "C->B", "B->I"}},
      {"C", "E", 2, {"C->B", "B->E"}},
      {"I", "F", 0, {}, false},
    },
  };
}

test_graph_s graph_seven() {
/*
         ┌──────────┐
         │          │
         │          ▼
    ┌──► A─────┐    B◄───┐
    │    │     │    │    │
    │    │     │    │    │
    │    │     │    │    │
    │    ▼     │    ▼    │
    │    C     └───►D────┘
    │               │
    │               │
    └───────────────┘
*/
  return test_graph_s{
    {
      {"A", "B", "C", "D"},
      {
        {"A", "D", "A->D"},
        {"A", "B", "A->B"},
        {"A", "C", "A->C"},
        {"D", "B", "D->B"},
        {"D", "A", "D->A"},
        {"B", "D", "B->D"},
      }
    },
    {
      {"A", "C", 1, {"A->C"}},
      {"B", "C", 3, {"B->D", "D->A", "A->C"}},
    },
  };
}
} // namespace

bool graph_tests() {

  static constexpr bool CHECK_CYCLES = true;

  using nodes_t = yokel::graph_c<std::string, std::string>::node_list_t;

  std::size_t i{1};
  for(auto graph_fn : {
      graph_one,
      graph_two,
      graph_three,
      graph_four,
      graph_five,
      graph_six,
      graph_seven
      }) {

    if (SHOW_GRAPH_NUMBER) {
      fmt::print(stderr, "Graph test # {}\n", i++);
    }

    yokel::graph_c<std::string, std::string> graph;
    auto graph_data = graph_fn();

    if (!graph.build_from(graph_data.data)) {
      fmt::print(stderr, "Failed to build graph\n");
      return false;
    }

    if (CHECK_CYCLES && graph_data.contains_cycles != graph.contains_cycles()) {
      fmt::print(stderr, "Failed to assess cyclic nature of graph. Expected {}, got {}\n",
          graph_data.contains_cycles, graph.contains_cycles());
      return false;
    }

    for(std::size_t i = 0; i < 2; i++) {
      if (i > 0 && (!graph.optimize_trace())) {
        fmt::print(stderr, "Failed to execute trace optimization");
        return false;
      }

      for (auto& path : graph_data.paths) {
        fflush(stderr);
        std::optional<nodes_t> result = graph.trace(path.from, path.to);
        
        if (!path.possible) {
          if (result.has_value()) {
            fmt::print(stderr, "Impossible path determined possible {} to {}", path.from, path.to);
            return false;
          }
          continue;
        }

        if (!result.has_value()) {
          fmt::print(stderr, "Failed to retrieve path for {} to {}\n", path.from, path.to);
          return false;
        }

        if (path.expected_distance > 1 && result.value().size() - 1 != path.expected_distance) {
          fmt::print(stderr, "Incorrect distance for {} to {}. Got {} expected {}\n",
              path.from, path.to, result.value().size()-1, path.expected_distance);
          return false;
        }

        if (result.value().size() <= 1 && (path.from != path.to)) {
          continue;
        }

        auto edges = graph.load_edges(result.value());

        if (!edges.has_value()) {
          fmt::print(stderr, "Unable to retrieve edges for {} to {}\n", path.from, path.to);
          return false;
        }
       
        if (edges.value().size() != path.expected_path.size()) {
          fmt::print(stderr, "Expected path size != retrieved edges : {} to {}\n", path.from, path.to);
          return false;
        }

        for(std::size_t i = 0; i < path.expected_path.size(); i++) {
          if (path.expected_path[i] != *(edges.value()[i])) {
            fmt::print(stderr, "Unexpected edge retrieved for {} to {}, Expected {} got {}\n",
              path.from, path.to, path.expected_path[i], *(edges.value()[i]));
            return false;
          }
        }
      }
    }
  }
  return true;
}

int main(void) {
  for(std::size_t i = 0; i < TEST_ITERATIONS; i++) {
    if (!graph_tests()) {
      fmt::print(stderr, "Failure\n");
      return 1;
    }
  }
  fmt::print(stderr, "Success of {} test iterations\n", TEST_ITERATIONS);
  return 0;
}

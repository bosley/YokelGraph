#include "graph.hpp"
#include <string>
#include <iostream>

using test_graph_t = yokel::graph_c<std::string, std::string>;
using test_data_t = test_graph_t::source_s;

static std::string graph_image = R"#(


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


)#";

int main(void) {

  std::cout << graph_image << std::endl;

  test_graph_t graph;

  test_data_t data = 
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
    };

  if (!graph.build_from(data)) {
    std::cout << "Failed to build graph\n";
    return 1;
  }

  std::cout << "Finding path from 'A' to 'C'" << std::endl;

  // Retrieve the path
  auto path = graph.trace("A", "C");

  std::cout << "Retrieved a path of length " << (*path).size() << " nodes, " 
            << (*path).size() - 1 << " hops" <<  std::endl;

  // The result is an optional, but we live dangerously in readmes. 
  auto edge_data = graph.load_edges(*path);

  for(auto* edge : edge_data.value()) {
    std::cout << *edge << " ";
  }
  std::cout << std::endl;

  return 0;
}


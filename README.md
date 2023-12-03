# YokelGraph

[![CircleCI](https://dl.circleci.com/status-badge/img/gh/bosley/YokelGraph/tree/main.svg?style=svg)](https://dl.circleci.com/status-badge/redirect/gh/bosley/YokelGraph/tree/main)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT) <br>
![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)

An unintelligent header-only directed graph implementation that encodes user-defined data into edges.

Given a graph such as :
```
  ┌─────────Alice────works-with──────► Bob
  │           ┼
knows      is-related-to
  │           │
  │           │
  ▼           ▼
Candice     Jeremy
  ▲           ▲
  │           │
  └─is-wed-to─┘
```

## Purpose

We may need to find the path from any node to any other given node. We may also need
to store variable relations between two nodes. This can be data such as "is-a", "has-a", "knows", etc.

In this graph implementation we find the shortest number of hops between two points (if a path exists),
and can retrieve the user-encoded relations between them.

### Note:

Graph searching is not thread-safe. This was built to handle small -> medium-ish graphs.

## Example usage

The following example is in `example.cpp` and can be constructed with the command `make example`.

```cpp

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

```


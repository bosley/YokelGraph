/*
    MIT License
    
    Copyright (c) 2023 bosley
    
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
    
    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.
    
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#ifndef YOKEL_GRAPH_HPP
#define YOKEL_GRAPH_HPP

#include <cmath>
#include <limits>
#include <map>
#include <optional>
#include <stack>
#include <vector>

/*
  When this is enabled in build it will require fmt/format.h to build
  correctly. 
      https://fmt.dev/latest/index.html
      Most package managers have "libfmt-dev"

  The debug statements help trace the path finding through the current graph.
*/
#ifdef GRAPH_ENABLE_DBG
#include <fmt/format.h>
#define GRAPH_DBG(msg_) \
  fmt::print(stderr, "{}", msg_);
#else
#define GRAPH_DBG(msg_)
#endif

namespace yokel {

//! \brief Graph implementation
//! \param NODE_ID_TYPE Data type to encode node identifiers as (must be default constructable)
//! \param EDGE_DATA Data type to encode into graph edges (must be default constructable)
template<class NODE_ID_TYPE, class EDGE_DATA>
class graph_c {
public:

  //! \brief A helpful structure to quickly load the graph
  struct source_s {
    struct edge_s {
      NODE_ID_TYPE from;
      NODE_ID_TYPE to;
      EDGE_DATA data;
    };
    std::vector<NODE_ID_TYPE> nodes;
    std::vector<edge_s> edges;
  };

  //! \brief Interface handed back to users when a path
  //!        is traced from one node to another
  class node_if {
  public:
    node_if() = default;
    virtual const NODE_ID_TYPE* data() const = 0;
  };

  using node_list_t = std::vector<node_if*>; 
  using edge_list_t = std::vector<EDGE_DATA*>;

  graph_c() = default;
  graph_c(const bool& cache_enabled)
    : _cache_enabled{cache_enabled} {}

  //! \brief Attempt to laod the graph
  bool build_from(const source_s& source) {
    for(auto& node : source.nodes) {
      if (!add_node(node)) {
        GRAPH_DBG("Failed to add node\n")
        return false;
      }
    }
    for(auto& edge : source.edges) {
      if (!add_edge(edge.from, edge.to, edge.data)) {
        GRAPH_DBG("Failed to add edge\n")
        return false;
      }
    }
    return true;
  }

  //! \brief Add a new node by copy (must have unique id)
  bool add_node(const NODE_ID_TYPE& id) {
    if (load_node(id)) { return false; }

    clear_cache();
    _node_storage.emplace(id, node_s(id));
    _contains_cycles = false;
    return true;
  }

  //! \brief Add an edge (must be a unique pair of nodes)
  bool add_edge(
    const NODE_ID_TYPE& from,
    const NODE_ID_TYPE& to,
    const EDGE_DATA& edge_data) {

    auto* to_node = load_node(to);
    if (!to_node) { return false; }

    auto* from_node = load_node(from);
    if (!from_node) { return false; }

    const std::size_t edge_hash = merge_ids(from, to);
    {
      const auto it = _edge_storage.find(edge_hash);
      if (it != _edge_storage.end()) { return false; }
    }
  
    clear_cache();
    from_node->out.push_back(to_node);
    _edge_storage[edge_hash] = edge_data;
    _contains_cycles = false;
    return true;
  }

  //! \brief Manually clear the cache
  inline void clear_cache() {
    _cache.clear();
  }

  //! \brief Enable/ disable the cache (clears when called)
  void toggle_cache(const bool& is_enabled) {
    _cache_enabled = is_enabled;
    return clear_cache();
  }

  //! \brief Attempt to optimize trace by calculating average
  //!        path length in the cache for memory reservation
  //!        on trace calls
  bool optimize_trace() {
    if (!_cache_enabled || _cache.empty()) { return false; }
    std::size_t total_paths_len{0};
    for(auto it = _cache.begin(); it != _cache.end(); ++it) {
      total_paths_len += it->second.size();
    }
    _average_path_len = std::ceil(total_paths_len / _cache.size());
    return true;
  }

  //! \brief Attempt to find a path between two nodes.
  //!        Will return the shortest path found 
  std::optional<node_list_t> trace(const NODE_ID_TYPE& from, const NODE_ID_TYPE& to) {
    if (!load_node(to)) { return std::nullopt; }
    if (!load_node(from)) { return std::nullopt; }
  
    std::size_t merged_ids{0};

    node_list_t result;

    std::size_t reservation{DEFAULT_TRACE_RESERVATION};

    if (_cache_enabled) {
      merged_ids = merge_ids(from, to);
      const auto it = _cache.find(merged_ids);
      if (it != _cache.end()) { return {it->second}; }
      reservation = _average_path_len;
      result.reserve(reservation);
    }

    const bool found = this->find(from, to, result, reservation);

    if (!found) {
      return std::nullopt;
    }

    if (_cache_enabled) {
      _cache[merged_ids] = {result};
    }

    return {result};
  }

  //! \brief Given some result path from trace, load data from
  //!        all edges that were crossed
  std::optional<edge_list_t> load_edges(const node_list_t& path) {
    if (path.size() < 2) { return std::nullopt; }
    edge_list_t result;
    result.reserve(path.size()-1);
    for(std::size_t i = 0; i < path.size()-1; i++) {
      const std::size_t id = merge_ids(*(path[i]->data()), *(path[i+1]->data()));
      const auto it = _edge_storage.find(id);
      if (it == _edge_storage.end()) { return std::nullopt; }
      result.push_back(&it->second);
    }
    return {result};
  }

  //! \brief Check if the graph contains cycles
  bool contains_cycles() {
    if (_contains_cycles) { return true; }
    for(auto&& [id, node] : _node_storage) {
      for(auto* neighbor : node.out) {
        auto result = trace(neighbor->id, id);
        if (result.has_value()){
          _contains_cycles = true;
          return true;
        }
      }
    }
    return _contains_cycles;
  }

private:
  static constexpr std::size_t DEFAULT_TRACE_RESERVATION = 5;

  struct node_s : public node_if {
    node_s() : node_if() {}
    node_s(NODE_ID_TYPE id) : node_if(), id (id) {}
    node_s(const NODE_ID_TYPE&& id) : node_if(), id (id) {}
    node_s(const node_s&& o) : node_if(), id(o.id){}

    const NODE_ID_TYPE* data() const override { return &id; }
    NODE_ID_TYPE id;
    std::vector<node_s*> out;
    bool marked{false};
  };

  bool _contains_cycles{false};
  std::map<NODE_ID_TYPE, node_s> _node_storage;
  std::map<std::size_t, EDGE_DATA> _edge_storage;

  bool _cache_enabled{true};
  std::size_t _average_path_len{0};
  std::map<std::size_t, node_list_t> _cache;

  static constexpr std::size_t merge_ids(
      const NODE_ID_TYPE& from, const NODE_ID_TYPE& to) {
    return std::hash<NODE_ID_TYPE>{}(from) ^ 
           (std::hash<NODE_ID_TYPE>{}(to) << 1);
  }

  inline node_s* load_node(const NODE_ID_TYPE& x) {
    const auto it = _node_storage.find(x);
    if (it == _node_storage.end()) { return nullptr; }
    return &it->second;
  }

  inline bool find(
    const NODE_ID_TYPE& from, 
    const NODE_ID_TYPE& to,
    node_list_t& path,
    const std::size_t& reservation) {

    auto& node = _node_storage[from];
    if (node.marked) {
      return false;
    }
    
    path.push_back(&node);

    if (node.id == to) {
      return true;
    }
    
    std::vector<node_list_t> traversed_path;
    traversed_path.reserve(node.out.size());

    node.marked = true;

    for(auto* neighbor : node.out) {
      GRAPH_DBG(fmt::format("{} scanning {}\n", node.id, neighbor->id))

      node_list_t current_path;
      current_path.reserve(path.capacity());
      if (find(neighbor->id, to, current_path, reservation)) {
        traversed_path.push_back({current_path});
        GRAPH_DBG("\n-FOUND-\n")
      }

      GRAPH_DBG((current_path.size()) ? "\nPATH:" : "")
      for(auto*x : current_path) {
        reinterpret_cast<node_s*>(x)->marked = false;
        GRAPH_DBG(fmt::format(" {}", *x->data()))
      };
      GRAPH_DBG((current_path.size()) ? "\n\n" : "")
    }

    node.marked = false;

    if (traversed_path.empty()){
      path.pop_back();
      return false;
    }

    std::size_t idx{0};
    std::size_t smallest = std::numeric_limits<std::size_t>::max();
    for(std::size_t i = 0; i < traversed_path.size(); i++) {
      if (traversed_path[i].size() < smallest) {
        smallest = traversed_path[i].size();
        idx = i;
      }
    }

    path.insert(
      path.end(),
      traversed_path[idx].begin(),
      traversed_path[idx].end());
    return true;
  }
};

} // namespace

#endif


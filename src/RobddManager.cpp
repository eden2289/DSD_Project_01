#include "RobddManager.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <map>
#include <queue>
#include <unordered_map>
#include <cmath>

// Creates a new node or returns an existing one if an identical node already exists.
// This function is the heart of the "reduction" in ROBDD.
int RobddManager::makeNode(int var_index, int else_id, int then_id) {
    // Reduction Rule 1: Eliminate redundant tests.
    if (else_id == then_id) return else_id;

    // Reduction Rule 2: Merge isomorphic nodes.
    long long key = makeKey(var_index, else_id, then_id);
    auto it = unique_table.find(key);
    if (it != unique_table.end()) return it->second;

    int id = (int)nodes.size();
    nodes.push_back({id, var_index, else_id, then_id, -1});
    unique_table[key] = id;
    return id;
}

// Always create a node without using unique table (used during OBDD construction).
int RobddManager::makeNodeNoReduce(int var_index, int else_id, int then_id, int obdd_index) {
    int id = (int)nodes.size();
    nodes.push_back({id, var_index, else_id, then_id, obdd_index});
    return id;
}

// ---------------- OBDD builder (no on-the-fly reduction) ----------------
int RobddManager::buildObddRec(int var_index, const std::vector<int>& term_indices, const PlaParser& parser, int cur_obdd_index) {
    int n = parser.getNumInputs();

    // Base case: at the leaf level. Check if any term covers this path.
    if (var_index == n) {
        return term_indices.empty() ? 0 : 1;
    }

    // Partition by current variable
    std::vector<int> else_terms; else_terms.reserve(term_indices.size());
    std::vector<int> then_terms; then_terms.reserve(term_indices.size());
    const auto& terms = parser.getProductTerms();
    for (int idx : term_indices) {
        char literal = terms[idx].cube[var_index];
        switch (literal) {
            case '-': else_terms.push_back(idx); then_terms.push_back(idx); break;
            case '0': else_terms.push_back(idx); break;
            case '1': then_terms.push_back(idx); break;
            default: break;
        }
    }

    int else_id = buildObddRec(var_index + 1, else_terms, parser, cur_obdd_index * 2);
    int then_id = buildObddRec(var_index + 1, then_terms, parser, cur_obdd_index * 2 + 1);

    // Always create a node to form a full tree, with its OBDD index assigned.
    return makeNodeNoReduce(var_index, else_id, then_id, cur_obdd_index);
}

// Initializes and starts the OBDD construction process.
bool RobddManager::buildObddFromPla(const PlaParser& parser) {
    nodes.clear();
    unique_table.clear();
    var_names = parser.getInputNames();

    // terminals
    nodes.push_back({0, -1, 0, 0, -1}); // 0
    nodes.push_back({1, -1, 1, 1, -1}); // 1

    int nVars = parser.getNumInputs();
    int expected_nodes = (1 << (nVars + 1)); // rough estimate
    nodes.reserve(expected_nodes);

    std::vector<int> all_terms;
    const auto& pts = parser.getProductTerms();
    all_terms.reserve(pts.size());
    for (size_t i = 0; i < pts.size(); ++i) all_terms.push_back((int)i);

    root_id = buildObddRec(0, all_terms, parser, 1);
    return true;
}

// Reduce an already built OBDD to an ROBDD.
int RobddManager::reduceRec(int old_id,
                            std::unordered_map<int,int>& map_old_to_new,
                            std::unordered_map<long long,int>& uniq,
                            std::vector<Node>& new_nodes) {
    // If this old_id has been processed, return its new mapped id.
    auto it = map_old_to_new.find(old_id);
    if (it != map_old_to_new.end()) return it->second;

    // Terminals map to themselves (fixed new IDs 0 and 1)
    if (old_id == 0) return 0;
    if (old_id == 1) return 1;

    const Node& cur = nodes[old_id];
    int new_else = reduceRec(cur.else_id, map_old_to_new, uniq, new_nodes);
    int new_then = reduceRec(cur.then_id, map_old_to_new, uniq, new_nodes);

    // Rule 1: remove redundant tests
    if (new_else == new_then) {
        map_old_to_new[old_id] = new_else;
        return new_else;
    }

    // Rule 2: merge isomorphic nodes
    long long key = makeKey(cur.var_index, new_else, new_then);
    auto uit = uniq.find(key);
    if (uit != uniq.end()) {
        map_old_to_new[old_id] = uit->second;
        return uit->second;
    }

    // Create reduced node and preserve OBDD index
    int new_id = (int)new_nodes.size();
    new_nodes.push_back({new_id, cur.var_index, new_else, new_then, cur.obdd_index});
    uniq[key] = new_id;
    map_old_to_new[old_id] = new_id;
    return new_id;
}

bool RobddManager::reduceToRobdd() {
    if (nodes.empty()) return false;

    // Prepare new container with terminals copied at fixed ids 0 and 1
    std::vector<Node> new_nodes;
    new_nodes.reserve(nodes.size());
    new_nodes.push_back({0, -1, 0, 0, -1});
    new_nodes.push_back({1, -1, 1, 1, -1});

    std::unordered_map<int,int> map_old_to_new; // old id -> new id
    std::unordered_map<long long,int> uniq; // unique table for reduced graph

    int new_root = reduceRec(root_id, map_old_to_new, uniq, new_nodes);

    nodes.swap(new_nodes);
    unique_table = std::move(uniq);
    root_id = new_root;
    return true;
}

// Convenience wrapper: build OBDD then reduce to ROBDD
bool RobddManager::buildFromPla(const PlaParser& parser) {
    if (!buildObddFromPla(parser)) return false;
    return reduceToRobdd();
}

// Writes the current BDD to a DOT file for visualization with Graphviz (compat: ROBDD style)
void RobddManager::writeDot(const std::string& filename) const {
    writeRobddDot(filename);
}

// Helper to compute level (0-based) from full OBDD index i (1..2^n-1)
static inline int level_from_obdd_index(int i) {
    int lvl = 0;
    while ((1 << (lvl + 1)) <= i) ++lvl;
    return lvl;
}

void RobddManager::writeObddDot(const std::string& filename) const {
    std::ofstream ofs(filename);
    if (!ofs.is_open()) {
        std::cerr << "Cannot write DOT file: " << filename << std::endl; return; }
    ofs << "digraph OBDD {\n";

    int nVars = (int)var_names.size();
    int max_internal = (1 << nVars) - 1;
    int oneId = (1 << nVars);

    // rank lines
    for (int lvl = 0; lvl < nVars; ++lvl) {
        int start = 1 << lvl;
        int end = (1 << (lvl + 1)) - 1;
        ofs << "{rank=same ";
        for (int i = start; i <= end; ++i) {
            ofs << i;
            if (i < end) ofs << ' ';
        }
        ofs << "}\n";
    }

    // Build mapping: obdd_index -> node id
    std::vector<int> obddToNode(max_internal + 1, -1); // 0 unused
    for (const auto& n : nodes) {
        if (n.var_index >= 0 && n.obdd_index >= 1 && n.obdd_index <= max_internal) {
            obddToNode[n.obdd_index] = n.id;
        }
    }

    // Terminals
    ofs << "0 [label=\"0\", shape=box];\n";

    // Variable nodes
    for (int i = 1; i <= max_internal; ++i) {
        int lvl = level_from_obdd_index(i);
        ofs << i << " [label=\"" << var_names[lvl] << "\"]\n"; // no semicolon per sample
    }

    // Terminal 1
    ofs << oneId << " [label=\"1\", shape=box];\n";

    // Edges
    for (int i = 1; i <= max_internal; ++i) {
        int nodeId = obddToNode[i];
        if (nodeId < 0) continue; // safety
        const auto& nd = nodes[nodeId];
        int e = (nd.else_id == 0) ? 0 : (nd.else_id == 1 ? oneId : nodes[nd.else_id].obdd_index);
        int t = (nd.then_id == 0) ? 0 : (nd.then_id == 1 ? oneId : nodes[nd.then_id].obdd_index);
        ofs << i << " -> " << e << " [label=\"0\", style=dotted]\n";
        ofs << i << " -> " << t << " [label=\"1\", style=solid]\n";
    }

    ofs << "}\n";
}

void RobddManager::writeRobddDot(const std::string& filename) const {
    std::ofstream ofs(filename);
    if (!ofs.is_open()) {
        std::cerr << "Cannot write DOT file: " << filename << std::endl; return; }
    ofs << "digraph ROBDD {\n";

    int nVars = (int)var_names.size();
    int oneId = (1 << nVars);

    // Collect surviving nodes per level using their OBDD indices
    std::map<int, std::vector<int>> level_to_indices; // level -> [obdd_index]
    for (const auto& n : nodes) {
        if (n.var_index >= 0 && n.obdd_index > 0) {
            int lvl = n.var_index; // since var order is fixed a,b,c,...
            level_to_indices[lvl].push_back(n.obdd_index);
        }
    }
    for (auto& kv : level_to_indices) {
        std::sort(kv.second.begin(), kv.second.end());
    }

    // Rank lines (only surviving indices)
    for (int lvl = 0; lvl < nVars; ++lvl) {
        auto it = level_to_indices.find(lvl);
        if (it == level_to_indices.end() || it->second.empty()) continue;
        ofs << "{rank=same ";
        for (size_t i = 0; i < it->second.size(); ++i) {
            ofs << it->second[i];
            if (i + 1 < it->second.size()) ofs << ' ';
        }
        ofs << "}\n";
    }

    // Terminals
    ofs << "0 [label=\"0\", shape=box];\n";

    // Variable nodes lines (sorted by index)
    std::vector<std::pair<int,int>> idx_and_level; // (obdd_index, level)
    for (const auto& kv : level_to_indices) {
        for (int idx : kv.second) idx_and_level.push_back({idx, kv.first});
    }
    std::sort(idx_and_level.begin(), idx_and_level.end());
    for (auto& p : idx_and_level) {
        ofs << p.first << " [label=\"" << var_names[p.second] << "\"]\n";
    }

    // Terminal 1
    ofs << oneId << " [label=\"1\", shape=box];\n";

    // Build mapping from obdd_index to current node id index in nodes vector
    std::unordered_map<int,int> obddToNode;
    for (const auto& n : nodes) {
        if (n.var_index >= 0 && n.obdd_index > 0) obddToNode[n.obdd_index] = n.id;
    }

    // Edges using OBDD indices
    for (auto& p : idx_and_level) {
        int idx = p.first;
        int nodeId = obddToNode[idx];
        const auto& nd = nodes[nodeId];
        int e = (nd.else_id == 0) ? 0 : (nd.else_id == 1 ? oneId : nodes[nd.else_id].obdd_index);
        int t = (nd.then_id == 0) ? 0 : (nd.then_id == 1 ? oneId : nodes[nd.then_id].obdd_index);
        ofs << idx << " -> " << e << " [label=\"0\", style=dotted]\n";
        ofs << idx << " -> " << t << " [label=\"1\", style=solid]\n";
    }

    ofs << "}\n";
}

void RobddManager::printTable() const {
    std::cout << "ID\tVar\tElse\tThen" << std::endl;
    for (const auto& n : nodes) {
        if (n.var_index == -1) {
            std::cout << n.id << '\t' << (n.id==0?"0":"1") << "\t-\t-" << std::endl;
        } else {
            std::cout << n.id << '\t' << var_names[n.var_index] << '\t' << n.else_id << '\t' << n.then_id << std::endl;
        }
    }
    std::cout << "Root = " << root_id << std::endl;
}

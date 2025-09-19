#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <string>
#include "PlaParser.h"

// Manages the creation and storage of an Ordered Binary Decision Diagram (OBDD)
// and its reduction to an ROBDD.
class RobddManager {
public:
    // Builds the OBDD from the boolean function defined in the PLA parser
    // (no on-the-fly reduction; it only returns terminals when the sub-function
    // becomes a constant). Returns true on success.
    bool buildObddFromPla(const PlaParser& parser);

    // Reduces the currently built OBDD to an ROBDD by applying the two
    // reduction rules bottom-up (eliminate redundant tests and merge isomorphic nodes).
    // Returns true on success.
    bool reduceToRobdd();

    // Convenience: builds OBDD then reduces to ROBDD.
    bool buildFromPla(const PlaParser& parser);

    // Writes the current BDD structure to a file in DOT format for visualization.
    void writeDot(const std::string& filename) const; // kept for compatibility (ROBDD style)

    // Dedicated writers for OBDD and ROBDD styles
    void writeObddDot(const std::string& filename) const;
    void writeRobddDot(const std::string& filename) const;

    // Prints the internal node table to the console for debugging.
    void printTable() const;

private:
    // Represents a single node in the BDD.
    struct Node {
        int id;          // Unique identifier for this node.
        int var_index;   // Index of the input variable this node tests. -1 for terminal nodes.
        int else_id;     // ID of the node to go to if the variable is 0 (else branch).
        int then_id;     // ID of the node to go to if the variable is 1 (then branch).
        int obdd_index;  // The index in the full OBDD (1..2^n-1), -1 for terminals or unset.
    };

    std::vector<Node> nodes; // Node table. Node ID is its index. 0: FALSE terminal, 1: TRUE terminal.
    int root_id = -1;        // The ID of the current root node.
    std::vector<std::string> var_names; // Input variable names.

    // --- Uniqueness table used only in reduction / ROBDD phase ---
    std::unordered_map<long long, int> unique_table; // (var,e,t) -> node id

    // Creates a 64-bit key from three integers for the unique_table.
    long long makeKey(int var_index, int e, int t) const { return ((long long)var_index << 42) ^ ((long long)e << 21) ^ (long long)t; }

    // Node creation helpers
    int makeNode(int var_index, int else_id, int then_id);        // with reduction (ROBDD)
    int makeNodeNoReduce(int var_index, int else_id, int then_id, int obdd_index); // always creates a new node (OBDD)

    // --- Recursive construction (OBDD only) ---
    int buildObddRec(int var_index, const std::vector<int>& term_indices, const PlaParser& parser, int cur_obdd_index);  // no reduction, full tree with index

    // Reduction of an existing OBDD to ROBDD
    int reduceRec(int old_id,
                  std::unordered_map<int,int>& map_old_to_new,
                  std::unordered_map<long long,int>& uniq,
                  std::vector<Node>& new_nodes);
};
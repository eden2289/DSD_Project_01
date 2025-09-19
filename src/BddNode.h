#pragma once
class BddNode{
    public:
        int id;
        int var_index; // -1 for terminal nodes
        BddNode* then_child; // high / 1-edge
        BddNode* else_child; // low / 0-edge
        BddNode(int id, int var_index, BddNode* then_child, BddNode* else_child)
            : id(id), var_index(var_index), then_child(then_child), else_child(else_child) {}
};
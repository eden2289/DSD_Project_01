#pragma once
#include <string>
#include <vector>

struct ProductTerm {
    std::string cube;
    std::string output;
};

class PlaParser{
public:
    bool parse(const std::string& filename);
    int getNumInputs() const;
    const std::vector<std::string>& getInputNames() const;
    const std::vector<ProductTerm>& getProductTerms() const;
private:
    int num_inputs = 0;
    std::vector<std::string> var_names;
    std::vector<ProductTerm> product_terms;
};

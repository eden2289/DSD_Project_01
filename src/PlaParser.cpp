#include "PlaParser.h"
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

// Parses a PLA file line by line.
bool PlaParser::parse(const std::string& filename){
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: cannot open " << filename << std::endl;
        return false;
    }
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string first_token;
        ss >> first_token; // Read the first word, automatically skipping whitespace

        // Skip empty lines and comments (lines starting with '#').
        if (first_token.empty() || first_token[0] == '#') {
            continue;
        }
        
        // Process lines starting with '.', which are PLA commands.
        if (first_token[0] == '.') {
            if (first_token == ".i") {
                // .i: specifies the number of inputs.
                ss >> this->num_inputs;
                this->var_names.resize(this->num_inputs);
            } else if (first_token == ".ilb") {
                // .ilb: specifies the names of the input variables.
                for (size_t i = 0; i < var_names.size(); ++i) {
                    ss >> var_names[i];
                }
            } else if (first_token == ".p") {
                // .p: specifies the number of product terms. We reserve space for efficiency.
                int num_products;
                ss >> num_products;
                this->product_terms.reserve(num_products);
            } else if (first_token == ".e") {
                // .e: marks the end of the file.
                break;
            }
            // Other commands like .o, .type are ignored.
        } else {
            // If the line does not start with '.', it's a product term.
            // The first token is the input cube.
            std::string output;
            ss >> output; // The second token is the output.
            if (!output.empty()) {
                // We only care about the on-set, i.e., terms where the output is '1'.
                if (output == "1") {
                    product_terms.push_back({first_token, output});
                }
            }
        }
    }
    file.close();
    return true;
}

// Getter for the number of inputs.
int PlaParser::getNumInputs() const {
    return this->num_inputs;
}

// Getter for the input variable names.
const std::vector<std::string>& PlaParser::getInputNames() const {
    return this->var_names;
}

// Getter for the product terms.
const std::vector<ProductTerm>& PlaParser::getProductTerms() const {
    return product_terms;
}
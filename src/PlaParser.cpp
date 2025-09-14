#include "PlaParser.h"
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

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

        if (first_token.empty() || first_token[0] == '#') {
            continue; // Skip empty lines and comments
        }
        
        if (first_token[0] == '.') {
            // This line is a command
            if (first_token == ".i") {
                ss >> this->num_inputs;
                this->var_names.resize(this->num_inputs);
            } else if (first_token == ".ilb") {
                for (size_t i = 0; i < var_names.size(); ++i) {
                    ss >> var_names[i];
                }
            } else if (first_token == ".p") {
                int num_products;
                ss >> num_products;
                this->product_terms.reserve(num_products);
            } else if (first_token == ".e") {
                break; // end of file
            }
            // ignore other commands like .o, .type etc.
        } else {
            // This line is a product term. first_token is the cube.
            std::string output;
            ss >> output;
            if (!output.empty()) {
                // 只處理有 on-set 的 product term (輸出為 1)
                if (output == "1") {
                    product_terms.push_back({first_token, output});
                }
            }
        }
    }
    file.close();
    return true;
}

int PlaParser::getNumInputs() const {
    return this->num_inputs;
}

const std::vector<std::string>& PlaParser::getInputNames() const {
    return this->var_names;
}

const std::vector<ProductTerm>& PlaParser::getProductTerms() const {
    return product_terms;
}
#include <iostream>
#include "PlaParser.h"
#include "RobddManager.h"

// Helper function to print the parsed results for verification.
void printParseResult(const PlaParser& parser) {
    std::cout << "====================================" << std::endl;
    std::cout << "      PLA Parser Result" << std::endl;
    std::cout << "====================================" << std::endl;
    std::cout << "Number of inputs: " << parser.getNumInputs() << std::endl;
    std::cout << "Variable names (" << parser.getInputNames().size() << ") : ";
    for (const auto& name : parser.getInputNames()) std::cout << name << ' ';
    std::cout << std::endl;
    std::cout << "Product terms (" << parser.getProductTerms().size() << ") :" << std::endl;
    int c = 1;
    for (const auto& term : parser.getProductTerms()) {
        std::cout << "  " << c++ << ". cube=" << term.cube << ", output=" << term.output << std::endl;
    }
    std::cout << "====================================" << std::endl;
}

// This is the main entry point of the program.
int main(int argc, char* argv[]) {
    // Check for correct command-line arguments.
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <input.pla> <output.dot>" << std::endl;
        std::cout << "Example: ./robdd pla_files/input.pla output.dot" << std::endl;
        return 1;
    }

    std::string pla_filepath = argv[1];
    std::string dot_filepath = argv[2];
    std::cout << "Input PLA file : " << pla_filepath << '\n';
    std::cout << "Output DOT file: " << dot_filepath << '\n';

    // 1. Parse the PLA file.
    PlaParser parser;
    if (!parser.parse(pla_filepath)) {
        std::cerr << "[Error] Parse failed." << std::endl;
        return 1;
    }
    std::cout << "[OK] Parsing successful." << std::endl;
    printParseResult(parser); // (Optional) Print parsed results for verification.

    // 2. Build OBDD first
    RobddManager mgr;
    std::cout << "[Info] Building OBDD ..." << std::endl;
    if (!mgr.buildObddFromPla(parser)) {
        std::cerr << "[Error] OBDD build failed." << std::endl;
        return 1;
    }

    // Derive OBDD dot path: append _obdd before .dot if present.
    std::string obdd_dot_path = dot_filepath;
    if (obdd_dot_path.size() >= 4 && obdd_dot_path.substr(obdd_dot_path.size()-4) == ".dot") {
        obdd_dot_path = obdd_dot_path.substr(0, obdd_dot_path.size()-4) + "_obdd.dot";
    } else {
        obdd_dot_path += "_obdd.dot";
    }

    std::cout << "[Info] Writing OBDD DOT ..." << std::endl;
    mgr.writeObddDot(obdd_dot_path);
    std::cout << "[OK] OBDD DOT written to: " << obdd_dot_path << std::endl;

    // 3. Reduce to ROBDD and write final DOT to the specified path.
    std::cout << "[Info] Reducing to ROBDD ..." << std::endl;
    if (!mgr.reduceToRobdd()) {
        std::cerr << "[Error] ROBDD reduction failed." << std::endl;
        return 1;
    }
    std::cout << "[OK] ROBDD built. Printing node table:" << std::endl;
    mgr.printTable(); // (Optional) Print the final node table for debugging.

    std::cout << "[Info] Writing ROBDD DOT ..." << std::endl;
    mgr.writeRobddDot(dot_filepath);
    std::cout << "[OK] ROBDD DOT written to: " << dot_filepath << std::endl;
    std::cout << "You can run: dot -Tpng " << obdd_dot_path << " -o obdd_graph.png" << std::endl;
    std::cout << "You can run: dot -Tpng " << dot_filepath << " -o graph.png" << std::endl;

    return 0;
}
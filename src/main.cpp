#include <iostream>
#include "PlaParser.h"
#include "RobddManager.h"

// 函式：用於印出解析結果，方便檢視
void printParseResult(const PlaParser& parser) {
    std::cout << "====================================" << std::endl;
    std::cout << "      PLA Parser Test Result" << std::endl;
    std::cout << "====================================" << std::endl;

    std::cout << "Number of inputs: " << parser.getNumInputs() << std::endl;

    std::cout << "Variable names (" << parser.getInputNames().size() << " found): ";
    for (const auto& name : parser.getInputNames()) {
        std::cout << name << " ";
    }
    std::cout << std::endl;

    std::cout << "Product terms (" << parser.getProductTerms().size() << " found):" << std::endl;
    int count = 1;
    for (const auto& term : parser.getProductTerms()) {
        // 明確印出 ProductTerm 的 cube 和 output 成員
        std::cout << "  " << count++ << ". cube=" << term.cube << ", output=" << term.output << std::endl;
    }
    std::cout << "====================================" << std::endl;
}


int main(int argc, char* argv[]) {
    // 根據專案要求，程式需要能處理命令列參數
    // SYNOPSIS: > PROGRAM PLA_FILE DOT_FILE
    if (argc != 3) {
        // 為了方便測試，如果沒有給參數，我們就讀取預設檔案
        std::cout << "Usage: " << argv[0] << " <input.pla> <output.dot>" << std::endl;
        std::cout << "Running test with default file: pla_files/input.pla" << std::endl;
        
        PlaParser test_parser;
        if (test_parser.parse("pla_files/input.pla")) {
            printParseResult(test_parser);
        } else {
            std::cerr << "Test failed: Could not parse default file." << std::endl;
            return 1;
        }
        return 0;
    }

    // --- 正常執行流程 ---
    std::string pla_filepath = argv[1];
    std::string dot_filepath = argv[2];

    std::cout << "Input PLA file: " << pla_filepath << std::endl;
    std::cout << "Output DOT file: " << dot_filepath << std::endl;

    PlaParser parser;
    if (parser.parse(pla_filepath)) {
        std::cout << "\nParsing successful!" << std::endl;
        printParseResult(parser);
        // TODO: 接下來就可以把 parser 物件交給 RobddManager 處理
    } else {
        std::cerr << "\nParsing failed." << std::endl;
        return 1;
    }

    return 0;
}
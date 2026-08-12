#include "ExpenseSplitterCLI.h"
#include <iostream>

int main() {
    try {
        ExpenseSplitterCLI cli("expense_splitter.db");
        cli.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}

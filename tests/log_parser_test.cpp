// Small test for the parser. Usage: ./test file1.log file2.txt ...
#include "LogParser.h"
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
    vector<string> files;
    for (int i = 1; i < argc; ++i) files.push_back(argv[i]);

    LogParser parser;
    int opened = parser.parseFiles(files);

    cout << "Files opened : " << opened << "/" << files.size() << "\n";
    cout << "Entries      : " << parser.size() << "\n";
    cout << "Skipped lines: " << parser.getSkippedLines() << "\n\n";

    for (const auto& e : parser.getEntries())
        cout << e.getTimestamp() << " | " << e.getLevel() << " | "
                  << e.getMessage() << "  (" << e.getSourceFile() << ":"
                  << e.getLineNumber() << ")\n";
    return 0;
}

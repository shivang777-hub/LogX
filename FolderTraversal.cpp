#include "FolderTraversal.h"
#include <iostream>
using namespace std;

namespace fs = std::filesystem;

bool isLogFile(const fs::path& path)
{
    string extension = path.extension().string();

    return extension == ".log" || extension == ".txt";
}

void traverse(const fs::path& folder)
{
    for (const auto& entry : fs::directory_iterator(folder))
    {
        if (entry.is_directory())
        {
            traverse(entry.path());
        }
        else if (isLogFile(entry.path()))
        {
            cout << "Log file found: "
                 << entry.path() << endl;
        }
    }
}





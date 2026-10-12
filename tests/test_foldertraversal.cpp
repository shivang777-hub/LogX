#include <iostream>
#include <filesystem>
#include <string>
#include "FolderTraversal.h"

using namespace std;
namespace fs = std::filesystem;

int main()
{
    string folderPath;

    cout << "Enter the path of your log folder: ";
    getline(cin, folderPath);

    if (!fs::exists(folderPath) || !fs::is_directory(folderPath))
    {
        cout << "Invalid folder path." << endl;
        return 1;
    }

    traverse(folderPath);

    return 0;
}
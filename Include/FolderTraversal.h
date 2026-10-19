#ifndef FOLDERTRAVERSAL_H
#define FOLDERTRAVERSAL_H

#include <filesystem>
#include <string>
#include <vector>

void traverse(const std::filesystem::path& folder, std::vector<std::string>& files);

#endif

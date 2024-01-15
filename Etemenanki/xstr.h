#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include <regex>

#include "Etemenanki.h"

struct xstrPair {
    int id;
    std::string text;
};

extern int Counter;
extern int Offset;

extern std::ofstream Output_file;

extern std::vector<std::string> Valid_extensions;

extern std::regex Valid_patterns[];

extern std::string Input_path;

int run(QLabel* terminal);
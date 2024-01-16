#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <regex>

#include "Etemenanki.h"

struct xstrPair {
    int id;
    std::string text;
};

struct regexPattern {
    std::regex pattern;
    int string_position;
    int id_position;
    int idx;
    std::string pattern_string;
};

extern QLabel* Terminal;

extern std::string Output_filename;

extern int Offset;
extern bool Replace_existing;

extern std::vector<std::string> Valid_extensions;

extern std::vector<regexPattern> Valid_patterns;

extern std::string Input_path;

int run();
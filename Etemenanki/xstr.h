/*
 * Copyright (C) Mike Nelson
 *
 * All source code herein is subject to the GPL license included.
 *
*/

#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <regex>

#include <QThread>
#include <QTimer>
#include <QObject>
#include <qdebug.h>

namespace fs = std::filesystem;

extern bool continueProcessing;

struct xstrPair {
    int id;
    std::string text;
};

class XstrProcessor : public QObject {
    Q_OBJECT

public:
    XstrProcessor(QObject* parent = nullptr) : QObject(parent) {}

    // Public methods
    void setTerminalText(const std::string& text);
    void setInputPath(std::string path);
    void setOutputFilepath(std::string path);
    void setOutputFilename(std::string file);
    void setReplaceExisting(bool val);
    void setOffset(int val);
    void setLogFilePath(QString path);

    int getNumFileExtensions();
    int getNumRegexPatterns();

    void clearVectors();

    void addFileExtension(std::string ext);
    void addRegexPattern(std::string pattern, int string_pos, int id_pos, int idx);

    bool isRunning();

    void run();

signals:
    void updateTerminalText(const QString& text);

public slots:
    //

private:
    // Top level methods
    void processDirectory(const fs::path& directoryPath);
    bool isExtensionValid(const std::string& extension);
    void processFile(const fs::path& filePath, bool write = true);
    void logEntry(const std::string& text, bool update_terminal = true);

    // Takes the line from the file and replaces the ID with the new one
    std::string replacePattern(const std::string& input, const std::string& somestring, int counter);

    // Checks if a line ID needs to be replaced
    void replaceLineID(std::string& line, const std::string& current_string, int& current_id);

    // Validates an xstr pair by comparing existing strings and IDs in the XSTR_list.
    // Fixes invalid IDs and generates new ones if required
    void validateXSTR(const std::string& line, int& id);

    // Saves a pair to XSTR_list and outputs the pair to the output file (usually tstrings.tbl)
    void savePair(const std::string& line, int id);

    // Replaces the ID of a pair in XSTR_list
    void replacePairID(const std::string& line, int new_id);

    xstrPair* findPair(const std::string& text);
    xstrPair* findPair(const int& id);
    int getNewId();

    std::vector<xstrPair> XSTR_list;
    int Counter = 0;
    std::ofstream Output_file;
    std::string LogFilePath;
    std::ofstream Log_file;

    struct regexPattern {
        std::regex pattern;
        int string_position;
        int id_position;
        int idx;
        std::string pattern_string;
    };

    std::string Output_filepath;
    std::string Output_filename;
    int Offset = 0;
    bool Replace_existing = false;
    std::vector<std::string> Valid_extensions;
    std::vector<regexPattern> Valid_patterns;
    std::string Input_path;
};
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
#include <set>
#include <thread>
#include <chrono>

#include <QThread>
#include <QTimer>
#include <QObject>
#include <qdebug.h>

namespace fs = std::filesystem;

extern bool g_continueProcessing;

struct XstrPair {
    int id;
    std::string text;
    bool printed;
    std::string file;

    XstrPair() : id(-1), text(""), printed(false), file("") {}
};

class XstrProcessor : public QObject {
    Q_OBJECT

public:
    XstrProcessor(QObject* parent = nullptr) : QObject(parent) {}

    // Methods to set runtime options
    void setTerminalText(const std::string& text);
    void setInputPath(std::string path);
    void setOutputFilepath(std::string path);
    void setOutputFilename(std::string file);
    void setReplaceExisting(bool val);
    void setComprehensiveScan(bool val);
    void setFillEmptyIds(bool val);
    void setOffset(int val);
    void setLogFilePath(QString path);

    // Method to set runtime vectors
    void clearVectors();
    void addFileExtension(std::string ext);
    void addRegexPattern(std::string pattern, int string_pos, int id_pos, int idx);
    
    // Methods to get runtime information
    int getNumFileExtensions();
    int getNumRegexPatterns();

    // Thread handling
    bool isRunning();
    void run();

signals:
    void update_terminal_text(const QString& text);

public slots:
    //

private:
    // Top level methods
    void processDirectory(const fs::path& directory_path, bool write);
    bool isExtensionValid(const std::string& extension);
    void processFile(const fs::path& file_path, bool write);
    void logEntry(const std::string& text, bool update_terminal = true);
    void updateIds();

    // Takes the line from the file and replaces the ID with the new one
    std::string replacePattern(const std::string& input, const std::string& current_string, const int& current_id);

    // Checks if a line ID needs to be replaced
    void replaceLineID(std::string& line, const std::string& current_string, int& current_id);

    // Validates an xstr pair by comparing existing strings and IDs in the m_xstrList.
    // Fixes invalid IDs and generates new ones if required
    void validateXSTR(const std::string& line, int& id);

    // Saves a pair to the m_xstrList vector
    void savePair(const std::string& line, int id);

    // Write a pair to the output file, usually tstrings.tbl
    void writePair(const std::string& line, const int& id);

    // Get a new unique xstr id
    int getNewId();
    
    // Overloads to find an xstr pair
    XstrPair* findPair(const std::string& text);
    XstrPair* findPair(const int& id);

    // Internal variable members
    std::vector<XstrPair> m_xstrList;
    int m_counter = 0;
    std::ofstream m_outputFile;
    std::string m_logFilePath;
    std::ofstream m_logFile;
    std::string m_currentFile;

    // Struct for storing regex patterns and associated information
    struct RegexPattern {
        std::regex pattern;
        int string_position;
        int id_position;
        int idx;
        std::string pattern_string;
    };

    // User options members
    std::string m_outputFilepath;
    std::string m_outputFilename;
    std::string m_inputFilepath;
    int m_offset = 0;
    bool m_replaceExisting = false;
    bool m_comprehensiveScan = false;
    bool m_fillEmptyIds = false;
    std::vector<std::string> m_validExtensions;
    std::vector<RegexPattern> m_validPatterns;
};
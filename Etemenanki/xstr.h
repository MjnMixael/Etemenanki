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

class XstrProcessor : public QObject {
    Q_OBJECT

public:
    XstrProcessor(QObject* parent = nullptr) : QObject(parent) {}

    // Public methods
    void setTerminalText(const std::string& text);
    void setInputPath(std::string path);
    void setOutputFilename(std::string file);
    void setReplaceExisting(bool val);
    void setOffset(int val);

    void clearVectors();

    void addFileExtension(std::string ext);
    void addRegexPattern(std::string pattern, int string_pos, int id_pos, int idx);

    bool isRunning();

    void run();

    struct regexPattern {
        std::regex pattern;
        int string_position;
        int id_position;
        int idx;
        std::string pattern_string;
    };

    std::string Output_filename;
    int Offset = 0;
    bool Replace_existing = false;
    std::vector<std::string> Valid_extensions;
    std::vector<regexPattern> Valid_patterns;
    std::string Input_path;

signals:
    void updateTerminalText(const QString& text);

public slots:
    //

private:
    void processDirectory(const fs::path& directoryPath);
    bool isExtensionValid(std::string extension);
    void processFile(const fs::path& filePath);
    int getXSTR(std::string line);
    bool hasInvalidID(std::string line, int id);
    int getExistingXSTR(std::string line);
    std::string replacePattern(const std::string& input, const std::string& somestring, int counter);
    int savePair(std::string line, int id);
    void logEntry(const std::string& text, bool update_terminal = true);

    struct xstrPair {
        int id;
        std::string text;
    };

    std::vector<xstrPair> XSTR_list;
    int Counter = 0;
    std::ofstream Output_file;
    std::ofstream Log_file;
};
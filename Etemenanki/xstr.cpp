/*
 * Copyright (C) Mike Nelson
 *
 * All source code herein is subject to the GPL license included.
 *
*/

#include "xstr.h"

bool XstrProcessor::isRunning() {
    return continueProcessing == true;
}

void XstrProcessor::setLogFilePath(QString path) {
    LogFilePath = path.toStdString();
}

void XstrProcessor::setInputPath(std::string path) {
    Input_path = path;
}

void XstrProcessor::setOutputFilepath(std::string path) {
    if (!path.empty() && path.back() != '\\') {
        path += '\\';
    }
    Output_filepath = path;
}

void XstrProcessor::setOutputFilename(std::string file) {
    Output_filename = file;
}

void XstrProcessor::setReplaceExisting(bool val) {
    Replace_existing = val;
}

void XstrProcessor::setOffset(int val) {
    Offset = val;
}

void XstrProcessor::clearVectors() {
    Valid_extensions.clear();
    Valid_patterns.clear();
}

void XstrProcessor::addFileExtension(std::string ext) {
    Valid_extensions.push_back(ext);
}
void XstrProcessor::addRegexPattern(std::string pattern, int string_pos, int id_pos, int idx) {
    std::regex reg;
    try {
        reg.assign(pattern);
    }
    catch (const std::regex_error& e) {
        //std::cerr << "Regex error: " << e.what() << std::endl;
        return;
    }
    regexPattern thisPattern = { reg, string_pos, id_pos, idx, pattern };
    Valid_patterns.push_back(thisPattern);
}

void XstrProcessor::setTerminalText(const std::string &text) {
    QString msg = QString::fromStdString(text);
    emit updateTerminalText(msg);
}

void XstrProcessor::logEntry(const std::string& text, bool update_terminal) {
    if (Log_file.is_open()) {
        Log_file << text << std::endl;
    }
    if (update_terminal) {
        setTerminalText(text);
    }
}

int XstrProcessor::savePair(std::string line, int id) {
    xstrPair newPair = { id, line };
    XSTR_list.push_back(newPair);
    Output_file << newPair.id << ", \"" << newPair.text << "\"" << std::endl;

    return newPair.id;
}

std::string XstrProcessor::replacePattern(const std::string& input, const std::string& somestring, int counter) {
    std::regex pattern("\"([^\"]+)\",\\s*(-?\\d+)");
    std::smatch match;

    if (std::regex_search(input, match, pattern)) {
        // Replace the original number with the new one
        return std::regex_replace(input, pattern, "\"" + somestring + "\", " + std::to_string(counter));
    }

    return input;
}

int XstrProcessor::getExistingXSTR(std::string line) {
    for (auto& pair : XSTR_list) {
        if (pair.text == line) {
            return pair.id;
        }
    }

    return -1;
}

bool XstrProcessor::hasInvalidID(std::string line, int id) {
    for (auto& pair : XSTR_list) {
        if (pair.id == id) {
            if (pair.text != line) {
                return true;
            } else {
                return false;
            }
        }
    }

    return false;
}

int XstrProcessor::getXSTR(std::string line) {
    int id = getExistingXSTR(line);

    if (id >= 0) {
        return id;
    }

    id = Offset + Counter++;

    return savePair(line, id);
}

void XstrProcessor::processFile(const fs::path& filePath) {
    if (!continueProcessing) {
        return;
    }

    // Skip the file if it is our output file
    std::string thisPath = filePath.string();
    std::string outputPath = Output_filepath + Output_filename;
    if (thisPath == outputPath) {
        return;
    }

    std::ifstream inputFile(filePath);
    if (!inputFile.is_open()) {
        std::string msg = "Error opening file: " + filePath.string();
        setTerminalText(msg);
        return;
    }

    std::string modifiedContent = "";

    bool found = false;

    // Print to the terminal
    std::string msg = "Processing file: " + filePath.filename().string();
    logEntry(msg);

    std::string line;
    while (std::getline(inputFile, line)) {
        for (const auto& set : Valid_patterns) {
            std::smatch match;
            if (continueProcessing && std::regex_search(line, match, set.pattern)) {

                if (!found) {
                    found = true;

                    // List the filename we found a match in
                    std::string file = filePath.filename().string();
                    Output_file << ";;" << filePath.filename().string() << std::endl;

                    // Print to the terminal
                    msg = "Found XSTR matches using pattern \"" + set.pattern_string + "\"";
                    logEntry(msg, false);
                }

                std::string somestring = match[set.string_position].str();
                std::string current_id = match[set.id_position].str();

                int id;
                try {
                    id = std::stoi(current_id);
                }
                catch (const std::invalid_argument& e) {
                    std::string error = e.what();
                    msg = "Xstr conversion invalid argument: " + error + ". Using -1.";
                    logEntry(msg, false);
                    id = -1;
                }
                catch (const std::out_of_range& e) {
                    std::string error = e.what();
                    msg = "Xstr id is out of range: " + error + ". Using -1.";
                    logEntry(msg, false);
                    id = -1;
                }
                catch (...) {
                    logEntry("Unknown error occurred getting xstr id. Using -1.", false);
                    id = -1;
                }

                if (id >= 0) {
                    msg = "Existing Integer in line: " + std::to_string(id);
                    logEntry(msg, false);
                }

                // Check for invalid duplicate IDs
                bool invalid_id = false;
                if (!Replace_existing && (hasInvalidID(somestring, id))) {
                    invalid_id = true;
                }

                // Maybe get a new id
                if ((id == -1) || invalid_id || Replace_existing) {
                    id = getXSTR(somestring);
                }
                else {
                    if (getExistingXSTR(somestring) == -1) {
                        savePair(somestring, id);
                    }
                }
                line = replacePattern(line, somestring, id);
            }
        }

        // Append the modified line to the modified content
        modifiedContent += line + '\n';
    }
    inputFile.close();

    if (found) {
        // Write the modified content back to the file
        std::ofstream outputFile(filePath, std::ofstream::trunc);
        if (!outputFile.is_open()) {
            msg = "Error saving file: " + filePath.string();
            logEntry(msg, false);
            return;
        }

        outputFile << modifiedContent;
        outputFile.close();
    }

    msg = "Processing completed for file: " + filePath.string();
    logEntry(msg, false);
}


bool XstrProcessor::isExtensionValid(std::string extension) {
    for (auto ext : Valid_extensions) {
        if (extension == ext) {
            return true;
        }
    }

    return false;
}

void XstrProcessor::processDirectory(const fs::path& directoryPath) {
    setTerminalText(Input_path);
    for (const auto& entry : fs::recursive_directory_iterator(directoryPath)) {
        if (!continueProcessing) {
            return;
        }
        if (entry.is_regular_file()) {
            // Check if the file has the required extension
            std::string extension = entry.path().extension().string();
            if (isExtensionValid(extension)) {
                // Process the file
                processFile(entry.path());
            }
        }
    }
}

void XstrProcessor::run() {
    Log_file.open(LogFilePath);
    if (!Log_file.is_open()) {
        logEntry("Error creating log file!");
        Log_file.close();
        continueProcessing = false;
        return;
    }

    std::string fullPath = Output_filepath + Output_filename;

    Output_file.open(fullPath);

    fs::path directoryPath = Input_path;
    if (!fs::is_directory(directoryPath)) {
        logEntry("Invalid directory path.");
        continueProcessing = false;
        return;
    }

    if (!Output_file.is_open()) {
        logEntry("Error creating output file!");
        Output_file.close();
        continueProcessing = false;
        return;
    }

    // Log our settings for this run...
    logEntry("Initializing settings...", false);
    std::string thisPath = "Directory: " + Input_path;
    logEntry(thisPath, false);
    std::string thisFile = "Output File: " + fullPath;
    logEntry(thisFile, false);
    std::string thisOffset = "Starting new XSTR IDs at " + Offset;
    logEntry(thisOffset, false);
    
    logEntry("List of extensions:", false);
    for (auto ext : Valid_extensions) {
        logEntry(ext, false);
    }

    logEntry("List of regex pattern settings", false);
    for (auto set : Valid_patterns) {
        std::string s_pos = "++String position: " + set.string_position;
        std::string i_pos = "++ID position: " + set.id_position;

        logEntry(set.pattern_string, false);
        logEntry(s_pos, false);
        logEntry(i_pos, false);
    }

    continueProcessing = true;
    logEntry("Running!", false);

    Output_file << "#default\n" << std::endl;

    processDirectory(directoryPath);

    Output_file << "\n#end" << std::endl;
    Output_file.close();

    std::string msg = "Processing completed. Output saved to " + Output_filename;
    logEntry(msg);
    Log_file.close();

    continueProcessing = false;
    return;
}

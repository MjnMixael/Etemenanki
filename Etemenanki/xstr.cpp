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

int XstrProcessor::getNumFileExtensions() {
    return static_cast<int>(Valid_extensions.size());
}

int XstrProcessor::getNumRegexPatterns() {
    return static_cast<int>(Valid_patterns.size());
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

std::string XstrProcessor::replacePattern(const std::string& input, const std::string& somestring, int counter) {
    std::regex pattern("\"([^\"]+)\",\\s*(-?\\d+)");
    std::smatch match;

    if (std::regex_search(input, match, pattern)) {
        // Replace the original number with the new one
        return std::regex_replace(input, pattern, "\"" + somestring + "\", " + std::to_string(counter));
    }

    return input;
}

void XstrProcessor::savePair(const std::string& line, int id) {
    // A quick verification that -1 is the only invalid ID stored
    if (id < 0) {
        id = -1;
    }

    xstrPair newPair = { id, line };
    XSTR_list.push_back(newPair);
    Output_file << newPair.id << ", \"" << newPair.text << "\"" << std::endl;
}

void XstrProcessor::replacePairID(const std::string& line, int new_id) {
    for (auto& pair : XSTR_list) {
        if (pair.text == line) {
            pair.id = new_id;
            return;
        }
    }
}

int XstrProcessor::getNewId() {
    int new_id = Offset + Counter++;

    while (findPair(new_id) != nullptr) {
        new_id = Offset + Counter++;
    }

    return new_id;
}

xstrPair* XstrProcessor::findPair(const std::string& text) {
    for (size_t i = 0; i < XSTR_list.size(); i++) {
        if (XSTR_list[i].text == text) {
            return &XSTR_list[i];
        }
    }

    return nullptr;
}

xstrPair* XstrProcessor::findPair(const int& id) {
    for (size_t i = 0; i < XSTR_list.size(); i++) {
        if (XSTR_list[i].id == id) {
            return &XSTR_list[i];
        }
    }

    return nullptr;
}

void XstrProcessor::validateXSTR(const std::string& line, int& id) {
    xstrPair* thisPair = findPair(line);

    if (thisPair == nullptr) {
        // Didn't find a string match. Now check for duplicate IDs
        thisPair = findPair(id);
        if (thisPair == nullptr) {
            // This pair is valid and brand new
            savePair(line, id);
            return;
        } else {
            // The ID matches, but the strings didn't so this is an invalid ID. Time to get a new one.
            id = getNewId();
            return;
        }
    } else {
        // The string matches, does the ID?
        if (thisPair->id == id) {
            // It does, so this is a valid XSTR
            return;
        } else {
            // It does not, so we need to fix the ID so it matches
            id = thisPair->id;
            return;
        }
    }
    
}

void XstrProcessor::replaceLineID(std::string& line, const std::string& current_string, int& current_id) {
    // If Replace_existing then we don't bother validating. Everything gets a new ID
    if (Replace_existing) {
        line = replacePattern(line, current_string, Offset + Counter++);
    } else {
        validateXSTR(current_string, current_id);
        line = replacePattern(line, current_string, current_id);
    }
}

void XstrProcessor::processFile(const fs::path& filePath, bool write) {
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

                std::string current_string = match[set.string_position].str();
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

                if (write) {
                    replaceLineID(line, current_string, id);
                } else {
                    validateXSTR(current_string, id);
                }
            }
        }

        // Append the modified line to the modified content
        modifiedContent += line + '\n';
    }
    inputFile.close();

    if (write && found) {
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


bool XstrProcessor::isExtensionValid(const std::string& extension) {
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
        std::string s_pos = "++String position: " + std::to_string(set.string_position);
        std::string i_pos = "++ID position: " + std::to_string(set.id_position);

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

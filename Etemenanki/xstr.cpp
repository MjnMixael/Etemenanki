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

void XstrProcessor::setComprehensiveScan(bool val) {
    Comprehensive_scan = val;
}

void XstrProcessor::setFillEmptyIds(bool val) {
    Fill_empty_ids = val;
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

void XstrProcessor::savePair(const std::string& line, int id) {
    // A quick verification that -1 is the only invalid ID stored
    if (id < 0) {
        id = -1;
    }

    xstrPair newPair = { id, line };
    XSTR_list.push_back(newPair);
}

void XstrProcessor::writePair(const std::string& line, const int& id) {
    Output_file << id << ", \"" << line << "\"" << std::endl;
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

void XstrProcessor::updateIds() {
    if (Replace_existing) {
        for (auto& pair : XSTR_list) {
            pair.id = getNewId();
        }
    } else {
        std::set<int> existingIds;
        int maxId = -1;

        // Identify maxId and build a set of existing IDs
        for (const auto& pair : XSTR_list) {
            if (pair.id >= 0) {
                existingIds.insert(pair.id);
                maxId = std::max(maxId, pair.id);
            }
        }

        // Set Counter to maxId to save time generating new IDs
        Counter = maxId;

        // If we are filling in empty ids then set Offset to 0 to ensure
        // there are no breaks between the highest existing Id and the next one
        // generated after all missing Ids are used up
        if (Fill_empty_ids) {
            Offset = 0;
        }

        std::set<int> missingIds;

        // Build a set of missing IDs
        for (int i = 0; i < maxId; ++i) {
            if (existingIds.find(i) == existingIds.end()) {
                missingIds.insert(i);
            }
        }

        // Assign new IDs
        for (auto& pair : XSTR_list) {
            if (pair.id < 0) {
                if (Fill_empty_ids && !missingIds.empty()) {
                    pair.id = *missingIds.begin();
                    missingIds.erase(missingIds.begin());
                } else {
                    pair.id = getNewId();
                }
            }
        }
    }
}


void XstrProcessor::validateXSTR(const std::string& line, int& id) {
    xstrPair* thisPair = findPair(line);

    if (thisPair == nullptr) {
        // Didn't find a string match. Now check for duplicate IDs
        thisPair = findPair(id);
        if (thisPair == nullptr) {
            // This pair is valid and brand new, but we should verify it's ID is valid first
            // However, if this is a comprehensive scan then we just save the -1 and adjust later
            if (!Comprehensive_scan && (id < 0)) {
                id = getNewId();
            }
            savePair(line, id);
        } else {
            // The ID matches, but the strings didn't so this is an invalid ID. Time to get a new one
            // However if this is a comprehensive scan then we just save as -1 because it'll get adjusted later
            if (!Comprehensive_scan) {
                id = getNewId();
            } else {
                id = -1;
            }
            savePair(line, id);
        }
    } else {
        // The string matches, does the ID?
        if (thisPair->id == id) {
            // It does, but is it a valid id?
            // However in a comprehensive scan we can just keep it as -1 because it'll get adjusted later
            if (!Comprehensive_scan && (id < 0)) {
                // ID is not valid, so we need a new one and update both
                id = getNewId();
                thisPair->id = id;
            }
        } else {
            // It does not, so we need to fix the ID so it matches. Prefer the stored one if it's valid
            if (thisPair->id >= 0) {
                id = thisPair->id;
            } else {
                // The stored ID was invalid so now we should prefer the current one as long as it's valid
                if (id >= 0) {
                    thisPair->id = id;
                } else {
                    // Neither was valid so now we need a new ID and to update both
                    // However in a comprehensive scan we can just keep it as -1 because it'll get adjusted later
                    if (!Comprehensive_scan) {
                        id = getNewId();
                        thisPair->id = id;
                    }
                }
            }
        }
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

void XstrProcessor::replaceLineID(std::string& line, const std::string& current_string, int& current_id) {
    // In a comprehensive run then validation is already complete so
    // find the string in the list and use it's xstr in all cases
    if (Comprehensive_scan) {
        xstrPair* thisPair = findPair(current_string);
        if (thisPair != nullptr) {
            current_id = thisPair->id;
            line = replacePattern(line, current_string, current_id);
        } else {
            // Well this isn't a good place to be.. log and skip!
            std::string msg = "Failed to find ID for string '" + current_string + "', skipping!";
            logEntry(msg);
        }
    // If Replace_existing then we don't bother validating. Everything gets a new ID
    } else if (Replace_existing) {
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
    std::string msg = "";

    bool found = false;

    // Print to the terminal
    if (write) {
        msg = "Processing file: " + filePath.filename().string();
    } else {
        msg = "Reading file: " + filePath.filename().string();
    }
    
    logEntry(msg);

    std::string line;
    while (std::getline(inputFile, line)) {
        for (const auto& set : Valid_patterns) {
            std::smatch match;
            if (continueProcessing && std::regex_search(line, match, set.pattern)) {

                if (!found) {
                    found = true;

                    // List the filename we found a match in
                    if (write) {
                        std::string file = filePath.filename().string();
                        Output_file << ";;" << filePath.filename().string() << std::endl;
                    }

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
                    writePair(current_string, id);
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

void XstrProcessor::processDirectory(const fs::path& directoryPath, bool write) {
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
                processFile(entry.path(), write);
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

    // If Replace Existing is set then comprehensive is pointless!
    if (Replace_existing) {
        Comprehensive_scan = false;
    }

    continueProcessing = true;
    logEntry("Running!", false);

    Output_file << "#default\n" << std::endl;

    std::string msg = "";

    // Start the scan!
    processDirectory(directoryPath, !Comprehensive_scan);

    // If we're doing Comprehensive then we need to go again
    // but this time we'll actually write the files
    if (continueProcessing && Comprehensive_scan) {
        // Write a message to the log and give the user a chance to cancel
        logEntry("Finished reading files! Processing...");
        std::this_thread::sleep_for(std::chrono::seconds(5));

        updateIds();

        // Write a message to the log and give the user one more chance to cancel before we write files
        logEntry("XSTR Database complete! Beginning file write process...");
        std::this_thread::sleep_for(std::chrono::seconds(5));

        processDirectory(directoryPath, true);
    }

    Output_file << "\n#end" << std::endl;
    Output_file.close();

    msg = "Processing completed. Output saved to " + Output_filename;
    logEntry(msg);
    Log_file.close();

    continueProcessing = false;
    return;
}

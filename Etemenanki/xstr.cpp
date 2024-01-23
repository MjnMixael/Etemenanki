/*
 * Copyright (C) Mike Nelson
 *
 * All source code herein is subject to the GPL license included.
 *
*/

#include "xstr.h"

bool XstrProcessor::isRunning() {
    return g_continueProcessing == true;
}

void XstrProcessor::setLogFilePath(QString path) {
    m_logFilePath = path.toStdString();
}

void XstrProcessor::setInputPath(std::string path) {
    m_inputFilepath = path;
}

void XstrProcessor::setOutputFilepath(std::string path) {
    if (!path.empty() && path.back() != '\\') {
        path += '\\';
    }
    m_outputFilepath = path;
}

void XstrProcessor::setOutputFilename(std::string file) {
    m_outputFilename = file;
}

void XstrProcessor::setReplaceExisting(bool val) {
    m_replaceExisting = val;
}

void XstrProcessor::setComprehensiveScan(bool val) {
    m_comprehensiveScan = val;
}

void XstrProcessor::setFillEmptyIds(bool val) {
    m_fillEmptyIds = val;
}

void XstrProcessor::setSortingType(int val) {
    m_sortingType = val;
}

void XstrProcessor::setHeadersToggle(bool val) {
    m_headerAnnotations = val;
}

void XstrProcessor::setAnnotationsToggle(bool val) {
    m_verboseAnnotations = val;
}

void XstrProcessor::setReadOnlyToggle(bool val) {
    m_readOnly = val;
}

void XstrProcessor::setCaseInsensitiveToggle(bool val) {
    m_caseInsensitive = val;
}

void XstrProcessor::setOffset(int val) {
    m_offset = val;
}

int XstrProcessor::getNumFileExtensions() {
    return static_cast<int>(m_validExtensions.size());
}

int XstrProcessor::getNumRegexPatterns() {
    return static_cast<int>(m_validPatterns.size());
}

void XstrProcessor::clearVectors() {
    m_validExtensions.clear();
    m_validPatterns.clear();
}

void XstrProcessor::addFileExtension(std::string ext) {
    m_validExtensions.push_back(ext);
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
    RegexPattern thisPattern = { reg, string_pos, id_pos, idx, pattern };
    m_validPatterns.push_back(thisPattern);
}

void XstrProcessor::setTerminalText(const std::string &text) {
    QString msg = QString::fromStdString(text);
    emit update_terminal_text(msg);
}

void XstrProcessor::logEntry(const std::string& text, bool update_terminal) {
    if (m_logFile.is_open()) {
        m_logFile << text << std::endl;
    }
    if (update_terminal) {
        setTerminalText(text);
    }
}

void XstrProcessor::savePair(const std::string& line, int id, bool invalid) {
    // A quick verification that -1 is the only invalid ID stored
    if (id < 0) {
        id = -1;
    }

    XstrPair newPair;
    newPair.id = id;
    newPair.text = line;
    newPair.files.push_back(m_currentFile);
    newPair.discovery_order = ++m_total;
    newPair.invalid = invalid;

    m_xstrList.push_back(newPair);
}

void XstrProcessor::writePair(XstrPair* this_pair) {
    // Everything should be validated by now but just to be sure...
    if (this_pair != nullptr) {
        // Only write if we haven't written it yet
        if (!this_pair->printed) {
            this_pair->printed = true;
            m_outputFile << this_pair->id << ", \"" << this_pair->text << "\"" << std::endl;
        }
    } else {
        // Well this isn't a good place to be.. log and skip!
        std::string msg = "Passed invalid pair! Could not write to strings.tbl!";
        logEntry(msg);
    }
}

void XstrProcessor::appendLocationToPair(XstrPair* this_pair) {
    bool new_location = true;

    for (auto location : this_pair->files) {
        if (location == m_currentFile) {
            new_location = false;
            break;
        }
    }

    if (new_location) {
        this_pair->files.push_back(m_currentFile);
    }
}

bool XstrProcessor::caseInsensitiveStringCompare(const std::string& str1, const std::string& str2) {
    std::string str1Lower = str1;
    std::string str2Lower = str2;

    // Convert both strings to lowercase
    std::transform(str1Lower.begin(), str1Lower.end(), str1Lower.begin(), ::tolower);
    std::transform(str2Lower.begin(), str2Lower.end(), str2Lower.begin(), ::tolower);

    // Perform the comparison
    return str1Lower == str2Lower;
}

XstrPair* XstrProcessor::findPair(const std::string& text) {
    for (size_t i = 0; i < m_xstrList.size(); i++) {
        if (m_caseInsensitive) {
            if (caseInsensitiveStringCompare(m_xstrList[i].text, text)) {
                return &m_xstrList[i];
            }
        } else {
            if (m_xstrList[i].text == text) {
                return &m_xstrList[i];
            }
        }
    }

    return nullptr;
}

XstrPair* XstrProcessor::findPair(const int& id) {
    for (size_t i = 0; i < m_xstrList.size(); i++) {
        if (m_xstrList[i].id == id) {
            return &m_xstrList[i];
        }
    }

    return nullptr;
}

int XstrProcessor::getNewId() {
    int newId = m_offset + m_counter++;

    while (findPair(newId) != nullptr) {
        newId = m_offset + m_counter++;
    }

    return newId;
}

void XstrProcessor::updateIds() {
    if (m_replaceExisting) {
        for (auto& pair : m_xstrList) {
            pair.id = getNewId();
        }
    } else {
        std::set<int> existingIds;
        int maxId = -1;

        // Identify maxId and build a set of existing IDs
        for (const auto& pair : m_xstrList) {
            if (pair.id >= 0) {
                existingIds.insert(pair.id);
                maxId = std::max(maxId, pair.id);
            }
        }

        // Set Counter to maxId to save time generating new IDs
        m_counter = maxId;

        std::set<int> missingIds;

        // Build a set of missing IDs
        for (int i = 0; i < maxId; ++i) {
            if (existingIds.find(i) == existingIds.end()) {
                missingIds.insert(i);
            }
        }

        // Assign new IDs
        for (auto& pair : m_xstrList) {
            if (pair.id < 0) {
                if (m_fillEmptyIds && !missingIds.empty()) {
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
    XstrPair* thisPair = findPair(line);

    if (thisPair == nullptr) {
        // Didn't find a string match. Now check for duplicate IDs
        thisPair = findPair(id);
        if (thisPair == nullptr) {
            // This pair is valid and brand new, but we should verify it's ID is valid first
            // However, if this is a comprehensive scan then we just save the -1 and adjust later
            if (!m_readOnly && !m_comprehensiveScan && (id < 0)) {
                id = getNewId();
            }
            savePair(line, id);
        } else {
            bool invalid = false;
            // The ID matches, but the strings didn't so this is an invalid ID. Time to get a new one
            // However if this is a comprehensive scan then we just save as -1 because it'll get adjusted later
            if (!m_readOnly && !m_comprehensiveScan) {
                id = getNewId();
            } else {
                if (m_comprehensiveScan) {
                    id = -1;
                }
                // If we're in read only mode then we keep the ID exactly as is and flag it as invalid
                if (m_readOnly) {
                    invalid = true;
                }
            }
            savePair(line, id, invalid);
        }
    } else {
        // The string matches, does the ID?
        if (thisPair->id == id) {
            // It does, but is it a valid id?
            // However in a comprehensive scan we can just keep it as -1 because it'll get adjusted later
            if (!m_readOnly && !m_comprehensiveScan && (id < 0)) {
                // ID is not valid, so we need a new one and update both
                id = getNewId();
                thisPair->id = id;
            }
        // The string matches, but the IDs do not...
        } else {
            // In read only let's mark this as invalid and move on
            if (m_readOnly) {
                thisPair->invalid = true;
            // Otherwise let's try and fix it up
            } else {
                // We need to fix the ID so it matches. Prefer the stored one if it's valid
                if (thisPair->id >= 0) {
                    id = thisPair->id;
                } else {
                    // The stored ID was invalid so now we should prefer the current one as long as it's valid
                    if (id >= 0) {
                        thisPair->id = id;
                    } else {
                        // Neither was valid so now we need a new ID and to update both
                        // However in a comprehensive scan we can just keep it as -1 because it'll get adjusted later
                        if (!m_comprehensiveScan) {
                            id = getNewId();
                            thisPair->id = id;
                        }
                    }
                }
            }
        }

        // If we got to this point then we found a string match and fixed up the IDs. Now we should
        // maybe save the current location to the pair's files vector
        appendLocationToPair(thisPair);
    }
}

std::string XstrProcessor::replacePattern(const std::string& input, const std::string& current_string, const int& current_id) {
    std::regex pattern("\"([^\"]+)\",\\s*(-?\\d+)");
    std::smatch match;

    if (std::regex_search(input, match, pattern)) {
        // Replace the original number with the new one
        return std::regex_replace(input, pattern, "\"" + current_string + "\", " + std::to_string(current_id));
    }

    return input;
}

void XstrProcessor::replaceLineID(std::string& line, const std::string& current_string, int& current_id) {
    // In a comprehensive run then validation is already complete so
    // find the string in the list and use it's xstr in all cases
    if (m_comprehensiveScan) {
        XstrPair* thisPair = findPair(current_string);
        if (thisPair != nullptr) {
            current_id = thisPair->id;
            line = replacePattern(line, current_string, current_id);
        } else {
            // Well this isn't a good place to be.. log and skip!
            std::string msg = "Failed to find ID for string '" + current_string + "', skipping!";
            logEntry(msg);
        }
    // If Replace_existing then everything gets a new ID
    } else if (m_replaceExisting) {
        current_id = m_offset + m_counter++;
        validateXSTR(current_string, current_id);
        line = replacePattern(line, current_string, current_id);
    } else {
        validateXSTR(current_string, current_id);
        line = replacePattern(line, current_string, current_id);
    }
}

// Custom comparison function for sorting by discovery order
bool XstrProcessor::compareByDiscovery(const XstrPair& a, const XstrPair& b) {
    return a.discovery_order < b.discovery_order;
}

// Custom comparison function for sorting by id
bool XstrProcessor::compareById(const XstrPair& a, const XstrPair& b) {
    return a.id < b.id;
}

// Custom comparison function for sorting by file
bool XstrProcessor::compareByFile(const XstrPair& a, const XstrPair& b) {
    // Convert both strings to lowercase for case-insensitive comparison
    std::string fileA = a.files.front();
    std::string fileB = b.files.front();
    std::transform(fileA.begin(), fileA.end(), fileA.begin(), ::tolower);
    std::transform(fileB.begin(), fileB.end(), fileB.begin(), ::tolower);

    // Use the case-insensitive comparison
    return fileA < fileB;
}

// Write the entire xstrList at once, assuming they are already in the correct order
void XstrProcessor::writeOutput() {
    std::string filename = "";
    for (auto pair : m_xstrList) {
        
        // Maybe print current file header comments
        if (m_headerAnnotations && (pair.files.front() != filename)) {
            filename = pair.files.front();
            m_outputFile << ";;" << filename << std::endl;
        }

        if (m_verboseAnnotations) {
            std::string verbose_comment = ";; This string also found in these files: ";
            bool print_line = true;

            // If we're not printing filename headers then let's be fully inclusive here
            if (!m_headerAnnotations) {
                verbose_comment = ";; This string found in these files: ";

                // Print the entire list of files
                for (auto it = pair.files.begin(); it != pair.files.end(); ++it) {
                    verbose_comment += *it;
                    if (std::next(it) != pair.files.end()) {
                        verbose_comment += ", ";
                    }
                }
            } else {
                // Otherwise we only need to print if there are additional files to display
                if (pair.files.size() <= 1) {
                    print_line = false;
                } else {
                    // Print all but the first file because it's in the comment header for this section
                    for (auto it = std::next(pair.files.begin()); it != pair.files.end(); ++it) {
                        verbose_comment += *it;
                        if (std::next(it) != pair.files.end()) {
                            verbose_comment += ", ";
                        }
                    }
                }
            }
            if (print_line) {
                m_outputFile << verbose_comment << std::endl;
            }
        }
        
        // In read only mode we don't do any ID verification but we can still print possible errors to look at
        if (m_readOnly && pair.invalid && (pair.id >= 0)) {
            std::string warning = ";;-------------------This string has a possible conflict with another string or ID!-------------------";
            m_outputFile << warning << std::endl;
        }

        writePair(&pair);
    }
}

void XstrProcessor::processFile(const fs::path& file_path, bool write) {
    if (!g_continueProcessing) {
        return;
    }

    // Skip the file if it is our output file
    std::string thisPath = file_path.string();
    std::string outputPath = m_outputFilepath + m_outputFilename;
    if (thisPath == outputPath) {
        return;
    }

    std::ifstream inputFile(file_path);
    if (!inputFile.is_open()) {
        std::string msg = "Error opening file: " + file_path.string();
        setTerminalText(msg);
        return;
    }

    std::string modifiedContent = "";
    std::string msg = "";

    bool found = false;

    // Print to the terminal
    if (write) {
        msg = "Processing file: " + file_path.filename().string();
    } else {
        msg = "Reading file: " + file_path.filename().string();
    }
    
    logEntry(msg);

    std::string line;
    while (std::getline(inputFile, line)) {
        for (const auto& set : m_validPatterns) {
            std::smatch match;
            if (g_continueProcessing && std::regex_search(line, match, set.pattern)) {

                if (!found) {
                    found = true;

                    // Save the filename to a global member
                    m_currentFile = file_path.filename().string();

                    // Print to the terminal
                    msg = "Found XSTR matches using pattern \"" + set.pattern_string + "\"";
                    logEntry(msg, false);
                }

                std::string currentString = match[set.string_position].str();
                std::string currentId = match[set.id_position].str();

                int id;
                try {
                    id = std::stoi(currentId);
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
                    replaceLineID(line, currentString, id);
                } else {
                    validateXSTR(currentString, id);
                }
            }
        }

        // Append the modified line to the modified content
        modifiedContent += line + '\n';
    }
    inputFile.close();

    if (write && found) {
        // Write the modified content back to the file
        std::ofstream outputFile(file_path, std::ofstream::trunc);
        if (!outputFile.is_open()) {
            msg = "Error saving file: " + file_path.string();
            logEntry(msg, false);
            return;
        }

        outputFile << modifiedContent;
        outputFile.close();
    }

    msg = "Processing completed for file: " + file_path.string();
    logEntry(msg, false);
}


bool XstrProcessor::isExtensionValid(const std::string& extension) {
    for (auto ext : m_validExtensions) {
        if (extension == ext) {
            return true;
        }
    }

    return false;
}

void XstrProcessor::processDirectory(const fs::path& directory_path, bool write) {
    setTerminalText(m_inputFilepath);
    for (const auto& entry : fs::recursive_directory_iterator(directory_path)) {
        if (!g_continueProcessing) {
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
    // Make sure we can write to the log file
    m_logFile.open(m_logFilePath);
    if (!m_logFile.is_open()) {
        logEntry("Error creating log file!");
        m_logFile.close();
        g_continueProcessing = false;
        return;
    }

    // Make sure we have a valid input directory
    fs::path directoryPath = m_inputFilepath;
    if (!fs::is_directory(directoryPath)) {
        logEntry("Invalid directory path.");
        g_continueProcessing = false;
        return;
    }

    std::string fullPath = m_outputFilepath + m_outputFilename;
    m_outputFile.open(fullPath);

    // Make sure we can write to the output file
    if (!m_outputFile.is_open()) {
        logEntry("Error creating output file!");
        m_outputFile.close();
        g_continueProcessing = false;
        return;
    }

    // Log our settings for this run...
    logEntry("Initializing settings...", false);
    std::string thisPath = "Directory: " + m_inputFilepath;
    logEntry(thisPath, false);
    std::string thisFile = "Output File: " + fullPath;
    logEntry(thisFile, false);

    if (m_headerAnnotations) {
        logEntry("Got command to write header filename annotations...", false);
    }

    if (m_verboseAnnotations) {
        logEntry("Got command to write verbose annoations...", false);
    }

    if (m_comprehensiveScan) {
        logEntry("Doing comprehensive scan!", false);
    }

    if (m_fillEmptyIds) {
        logEntry("Attempting to use all empty IDs!", false);

        // If we are filling in empty ids then set Offset to 0 to ensure
        // there are no breaks between the highest existing Id and the next one
        // generated after all missing Ids are used up
        m_offset = 0;
    }

    std::string thisOffset = "Starting new XSTR IDs at " + m_offset;
    logEntry(thisOffset, false);

    if (m_readOnly) {
        logEntry("Running in Read Only mode! No input files will be edited...", false);
    }
    
    logEntry("List of extensions:", false);
    for (auto ext : m_validExtensions) {
        logEntry(ext, false);
    }

    logEntry("List of regex pattern settings", false);
    for (auto set : m_validPatterns) {
        std::string stringPos = "++String position: " + std::to_string(set.string_position);
        std::string idPos = "++ID position: " + std::to_string(set.id_position);

        logEntry(set.pattern_string, false);
        logEntry(stringPos, false);
        logEntry(idPos, false);
    }

    // If Replace Existing is set then comprehensive is pointless!
    if (m_replaceExisting) {
        m_comprehensiveScan = false;
    }

    m_xstrList.clear();

    // Notify that processing is starting
    g_continueProcessing = true;
    logEntry("Running!", false);

    std::string msg = "";

    bool write_files = (!m_readOnly && !m_comprehensiveScan);

    // Start the scan!
    processDirectory(directoryPath, write_files);

    // If we're doing Comprehensive then we need to go again
    // but this time we'll actually write the files
    if (g_continueProcessing && !m_readOnly && m_comprehensiveScan) {
        // Write a message to the log and give the user a chance to cancel
        logEntry("Finished reading files! Processing...");
        std::this_thread::sleep_for(std::chrono::seconds(5));

        updateIds();

        // Write a message to the log and give the user one more chance to cancel before we write files
        logEntry("XSTR Database complete! Beginning file write process...");
        std::this_thread::sleep_for(std::chrono::seconds(5));

        processDirectory(directoryPath, true);
    }

    switch (m_sortingType) {
        case PARSING_ORDER:
            std::sort(m_xstrList.begin(), m_xstrList.end(), compareByDiscovery);
            break;
        case XSTR_ID_ORDER:
            std::sort(m_xstrList.begin(), m_xstrList.end(), compareById);
            break;
        case FILENAME_ORDER:
            std::sort(m_xstrList.begin(), m_xstrList.end(), compareByFile);
            break;
        default:
            // If we end up here then we just don't do any sorting and print in whatever
            // order the vector is already in.
            break;
    }

    // Now actually write the output file
    m_outputFile << "#default\n" << std::endl;

    writeOutput();

    m_outputFile << "\n#end" << std::endl;
    m_outputFile.close();

    msg = "Processing completed. Output saved to " + m_outputFilename;
    logEntry(msg);
    m_logFile.close();

    g_continueProcessing = false;
    return;
}

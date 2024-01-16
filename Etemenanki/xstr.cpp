#include "xstr.h"

namespace fs = std::filesystem;

QLabel* Terminal;

int Counter = 0;

int Offset = 0;
bool Replace_existing = false;

std::string Output_filename;

std::vector<xstrPair> XSTR_list;

std::ofstream Output_file;

std::vector<std::string> Valid_extensions;

std::vector<regexPattern> Valid_patterns;

std::string Input_path;

void setTerminalText(std::string text) {
    QString msg = QString::fromStdString(text);
    Terminal->setText(msg);
}

int savePair(std::string line, int id) {
    xstrPair newPair = { id, line };
    XSTR_list.push_back(newPair);
    Output_file << newPair.id << ", \"" << newPair.text << "\"" << std::endl;

    return newPair.id;
}

std::string replacePattern(const std::string& input, const std::string& somestring, int counter) {
    std::regex pattern("\"([^\"]+)\",\\s*(-?\\d+)");
    std::smatch match;

    if (std::regex_search(input, match, pattern)) {
        // Replace the original number with the new one
        return std::regex_replace(input, pattern, "\"" + somestring + "\", " + std::to_string(counter));
    }

    return input;
}

int getExistingXSTR(std::string line) {
    for (auto& pair : XSTR_list) {
        if (pair.text == line) {
            return pair.id;
        }
    }

    return -1;
}

bool hasInvalidID(std::string line, int id) {
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

int getXSTR(std::string line) {
    int id = getExistingXSTR(line);

    if (id >= 0) {
        return id;
    }

    id = Offset + Counter++;

    return savePair(line, id);
}

void processFile(const fs::path& filePath) {
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
    setTerminalText(msg);

    std::string line;
    while (std::getline(inputFile, line)) {
        for (const auto& set : Valid_patterns) {
            std::smatch match;
            if (std::regex_search(line, match, set.pattern)) {

                if (!found) {
                    found = true;

                    // List the filename we found a match in
                    std::string file = filePath.filename().string();
                    Output_file << ";;" << filePath.filename().string() << std::endl;

                    // Print to the terminal
                    msg = "Found XSTR matches using pattern \"" + set.pattern_string + "\"";
                    //setTerminalText(msg);
                }

                std::string somestring = match[set.string_position].str();
                std::string current_id = match[set.id_position].str();

                int id;
                try {
                    id = std::stoi(current_id);
                }
                catch (const std::invalid_argument& e) {
                    std::string error = e.what();
                    msg = "Invalid argument: " + error;
                    //setTerminalText(msg);
                    id = -1;
                }
                catch (const std::out_of_range& e) {
                    std::string error = e.what();
                    msg = "Out of range: " + error;
                    //setTerminalText(msg);
                    id = -1;
                }
                catch (...) {
                    //setTerminalText("An unknown error occurred.");
                    id = -1;
                }

                if (id >= 0) {
                    msg = "Existing Integer in line: " + id;
                    //setTerminalText(msg);
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
            //setTerminalText(msg);
            return;
        }

        outputFile << modifiedContent;
        outputFile.close();
    }

    msg = "Processing completed for file: " + filePath.string();
    //setTerminalText(msg);
}


bool isExtensionValid(std::string extension) {
    for (auto ext : Valid_extensions) {
        if (extension == ext) {
            return true;
        }
    }

    return false;
}

void processDirectory(const fs::path& directoryPath) {
    setTerminalText(Input_path);
    for (const auto& entry : fs::recursive_directory_iterator(directoryPath)) {
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

int run() {
    setTerminalText("Running!");

    Output_file.open(Output_filename);

    fs::path directoryPath = Input_path;
    if (!fs::is_directory(directoryPath)) {
        std::cerr << "Invalid directory path." << std::endl;
        return 1;
    }

    if (!Output_file.is_open()) {
        std::cerr << "Error creating output file." << std::endl;
        Output_file.close();
        return 1;
    }

    Output_file << "#default\n" << std::endl;

    processDirectory(directoryPath);

    Output_file << "\n#end" << std::endl;

    Output_file.close();

    std::string msg = "Processing completed. Output saved to " + Output_filename;
    setTerminalText(msg);

    return 0;
}

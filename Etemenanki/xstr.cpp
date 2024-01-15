#include "xstr.h"

namespace fs = std::filesystem;

QLabel* Terminal;

int Counter = 0;
int Offset = 4000;

std::vector<xstrPair> XSTR_list;

std::ofstream Output_file("output.txt");

std::vector<std::string> Valid_extensions = { ".tbm", ".tbl", ".lua", ".fs2", ".fc2", ".cfg" };

std::regex Valid_patterns[] = {
        std::regex("XSTR\\(\"([^\"]+)\",\\s*(-?\\d+)\\)"),
        std::regex("([a-zA-Z0-9_]+)\\s*=\\s*\\{\\s*\"([^\"]+)\",\\s*(-?\\d+)\\}"),
        std::regex("utils\\.xstr\\(\\{\\s*\"([^\"]+)\",\\s*(-?\\d+)\\}\\)"),
        std::regex("\\+Val:\\s*([a-zA-Z0-9_]+)\\s*\\(\\s*\"([^\"]+)\",\\s*(-?\\d+)\\)"),
        std::regex("\\[\"([^\"]+)\",\\s*(-?\\d+)\\]")
};

std::string Input_path = "C:\\Games\\FreespaceOpen\\FS2\\BtA-2.0.0";

std::string replacePattern(const std::string& input, const std::string& somestring, int counter) {
    std::regex pattern("\"([^\"]+)\",\\s*(-?\\d+)");
    std::smatch match;

    if (std::regex_search(input, match, pattern)) {
        // Replace the original number with the new one
        return std::regex_replace(input, pattern, "\"" + somestring + "\", " + std::to_string(counter));
    }

    return input;
}

int getXSTR(std::string line) {
    for (auto& pair : XSTR_list) {
        if (pair.text == line) {
            return pair.id;
        }
    }

    int id = Offset + Counter++;

    xstrPair newPair = { id, line };

    XSTR_list.push_back(newPair);

    //Save new line to output file
    Output_file << newPair.id << ", \"" << newPair.text << "\"" << std::endl;

    return newPair.id;
}

void printExistingInteger(std::string text) {
    int existingInteger = std::stoi(text);

    if (existingInteger != -1) {
        //std::cout << "Existing Integer in line: " << existingInteger << std::endl;
        std::string text = "Existing Integer in line: " + existingInteger;
        Etemenanki dlg;
        dlg.set_terminal_text(text);
    }
}

void processFile(const fs::path& filePath) {
    std::ifstream inputFile(filePath);
    if (!inputFile.is_open()) {
        std::cerr << "Error opening file: " << filePath.string() << std::endl;
        return;
    }

    bool found = false;

    // Print to the terminal
    //std::cout << "Processing file: " << filePath.filename().string() << std::endl;
    std::string text = "Processing file: " + filePath.filename().string();
    QString q_text;
    q_text.fromStdString(text);
    Terminal->setText(q_text);

    std::string line;
    while (std::getline(inputFile, line)) {
        int count = 0;
        for (const auto& pattern : Valid_patterns) {
            count++;
            std::smatch match;
            if (std::regex_search(line, match, pattern)) {

                if (!found) {
                    found = true;

                    // List the filename we found a match in
                    std::string file = filePath.filename().string();
                    Output_file << ";;" << filePath.filename().string() << std::endl;

                    // Print to the terminal
                    std::cout << "Found XSTR matches using pattern " << count << "!" << std::endl;
                }

                std::string somestring = match[1].str();

                // Debug notice only if we're not in a normal XSTR pattern
                if (count == 1) {
                    std::cout << "Processing pattern 1" << std::endl;
                    printExistingInteger(match[2].str());
                }
                if (count == 2) {
                    std::cout << "Processing pattern 2" << std::endl;
                    printExistingInteger(match[3].str());
                }
                if (count == 3) {
                    std::cout << "Processing pattern 3" << std::endl;
                    printExistingInteger(match[2].str());
                }
                if (count == 4) {
                    std::cout << "Processing pattern 4" << std::endl;
                    printExistingInteger(match[3].str());
                }
                if (count == 5) {
                    std::cout << "Processing pattern 5" << std::endl;
                    printExistingInteger(match[2].str());
                }
                // Replace using the same pattern
                line = replacePattern(line, somestring, getXSTR(somestring));
            }
        }
    }

    inputFile.close();
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

int run(QLabel* terminal) {
    Terminal = terminal;

    Terminal->setText("Running!");

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

    std::cout << "Processing completed. Output saved to output.txt." << std::endl;

    return 0;
}

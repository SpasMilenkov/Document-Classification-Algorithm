/**
@file main.cpp
*/

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <map>
#include <filesystem>
#include <algorithm>
std::map<std::string, std::vector<std::string>> catalog {};

struct SearchResult {
    std::string topicName;
    int count{};
};


std::vector<std::string> tokenize(const std::string& s, const std::string_view &del = " ")
{
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = s.find(del);

    while (end != std::string::npos) {
        tokens.push_back(s.substr(start, end - start));
        start = end + del.size();
        end = s.find(del, start);
    }
    tokens.push_back(s.substr(start)); // Add the last token after the last delimiter
    return tokens;
}

void readCatalog() {
    // Open the input file named "input.txt"
    std::ifstream inputFile("../catalog.txt");
    // Check if the file is successfully opened
    if (!inputFile.is_open()) {
        std::cerr << "Error opening the file!" << std::endl;
        throw std::invalid_argument("Catalog file does not exist");
    }

    std::string line; // Declare a string variable to store each
    // line of the file

    // Read each line of the file and print it to the
    // standard output stream
    std::cout << "File Content: " << std::endl;
    while (getline(inputFile, line)) {
        std::vector<std::string> vector =  tokenize(line, "@%");
        catalog.emplace(vector[0], tokenize(vector[1], ","));
        std::cout << line << std::endl; // Print the current line
    }

    // Close the file
    inputFile.close();
}

 std::pair<std::string, std::vector<SearchResult>> findAllOccurrences(const std::string& fileName) {
    std::ifstream inputFile(fileName);
    // Check if the file is successfully opened
    if (!inputFile.is_open()) {
        std::cerr << "Error opening the file!" << std::endl;
        throw std::invalid_argument("Catalog file does not exist");
    }

    std::string line; // Declare a string variable to store each
    std::string text; // Declare a string variable to store each
    // line of the file

    // Read each line of the file and print it to the
    // standard output stream
    std::cout << "File Content: " << std::endl;
    while (getline(inputFile, line)) {
        text += line;
    }
    std::vector<SearchResult> matches {};
    for (auto const& [topic, identifiers] : catalog) {
        std::vector<int> occurrences;
        int count = 0;
        for (const std::string& term : identifiers) {
            size_t pos = text.find(term, 0);
            while (pos != std::string::npos) {
                occurrences.push_back(pos);
                pos = text.find(term, pos + term.length());
                count++;
            }
        }
        matches.emplace_back(SearchResult {topic, static_cast<int>(occurrences.size())});
    }

    return std::pair {fileName, matches} ;
}


std::vector<std::string> getAllFilesInDirectory(const std::string& directoryPath, const std::vector<std::string>& extensions) {
    std::vector<std::string> files;
    for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
        if (std::filesystem::is_regular_file(entry.path())) {
            std::string extension = entry.path().extension().string();
            if (std::find(extensions.begin(), extensions.end(), extension) != extensions.end()) {
                files.push_back(entry.path().string());
            }
        }
    }
    return files;
}

SearchResult fillSearchResult(const std::string& name, int count) { // Update to only accept count
    SearchResult sr;
    sr.topicName = name;
    sr.count = count;
    return sr;
}

void writeResultsToFile(const std::vector<std::pair<std::string, std::vector<SearchResult>>> matches, const std::string& filename) {
    std::ofstream outputFile(filename);
    if (!outputFile.is_open()) {
        std::cerr << "Error opening file for writing!" << std::endl;
        return;
    }

    for (const auto&[docName, topics] : matches) {
        outputFile << docName << '\n';
        for (const auto&[topicName, count] : topics) {
            outputFile << topicName << "," << count << "\n";
        }
        outputFile << "\n"; // Separate sets of search results
    }

    outputFile.close();
}

std::vector<std::vector<SearchResult>> readResultsFromFile(const std::string& filename) {
    std::vector<std::vector<SearchResult>> matches;

    std::ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        std::cerr << "Error opening file for reading!" << std::endl;
        return matches;
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        std::vector<SearchResult> searchResults;
        if (line.empty()) {
            // Empty line indicates the end of one set of search results
            matches.push_back(std::move(searchResults));
            continue;
        }

        std::istringstream iss(line);
        std::string topicName;
        int count;
        if (std::getline(iss, topicName, ',') && (iss >> count)) {
            searchResults.push_back({topicName, count});
        }
        matches.push_back(std::move(searchResults));
    }

    inputFile.close();
    return matches;
}

std::unordered_map<std::string, std::string> determineRelevantTopics(const std::vector<std::pair<std::string, std::vector<SearchResult>>>& matches) {
    std::unordered_map<std::string, std::string> relevantTopics;

    for (const auto& [docName, topics] : matches) {
        int maxCount = 0;
        std::string relevantTopic;

        for (const auto& [topicName, count] : topics) {
            if (count > maxCount) {
                maxCount = count;
                relevantTopic = topicName;
            }
        }

        relevantTopics[docName] = relevantTopic;
    }

    return relevantTopics;
}

int main() {
    readCatalog();
    // std::string directoryPath = "../sample_documents/";
    std::string directoryPath = "../testDocuments/";
    // Specify the file extensions to filter
    std::vector<std::string> extensions = {".html", ".txt", ".tex"};

    std::vector<std::string> files = getAllFilesInDirectory(directoryPath, extensions);

    std::cout << "Files in directory with extensions (.html, .txt, .tex):" << std::endl;


    std::vector<std::pair<std::string, std::vector<SearchResult>>> matches {};
    matches.reserve(files.size());
    for (const auto& file : files) {
        matches.emplace_back(findAllOccurrences(file));
    }
    writeResultsToFile(matches, "results.csv");

    // Read the data from the file
    std::vector<std::vector<SearchResult>> resultsFromFile = readResultsFromFile("results.csv");

    // Print the read data
    for (const auto&[docName, topics] : matches) {
        std::cout  << docName << '\n';
        for (const auto& [topicName, count] : topics) {
            std::cout << "Topic: " << topicName << ", Count: " << count << std::endl;
        }
        std::cout << std::endl;
    }

    std::unordered_map<std::string, std::string> relevantTopics = determineRelevantTopics(matches);

    // Print the relevant topics
    for (const auto& [docName, relevantTopic] : relevantTopics) {
        std::cout << "Document: " << docName << ", Relevant Topic: " << relevantTopic << std::endl;
    }

    return 0;
}

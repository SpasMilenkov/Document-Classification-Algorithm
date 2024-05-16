/**
* @file main.cpp
 * @brief MPI-based document classification using a catalog and multiple processes.
 */
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <algorithm>
#include <mpi.h>

using namespace std;

std::map<std::string, std::vector<std::string>> catalog{};
/**
 * @brief Tokenizes a string based on a delimiter.
 * @param s The input string to tokenize.
 * @param del The delimiter to use for tokenization.
 * @return Vector of tokens.
 */
std::vector<std::string> tokenize(const std::string& s, const std::string& del = " ")
{
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = s.find(del);

    while (end != std::string::npos)
    {
        tokens.push_back(s.substr(start, end - start));
        start = end + del.size();
        end = s.find(del, start);
    }
    tokens.push_back(s.substr(start));
    return tokens;
}
/**
 * @brief Reads the catalog data from a file.
 * @details The catalog file format is expected to be:
 *          Topic1@%Identifier1,Identifier2,Identifier3
 *          Topic2@%Identifier4,Identifier5,Identifier6
 */
void readCatalog()
{

    std::ifstream inputFile("./actualCatalog.txt");

    if (!inputFile.is_open())
    {
        std::cerr << "Error opening the file!" << std::endl;
        throw std::invalid_argument("Catalog file does not exist");
    }

    std::string line;
    while (getline(inputFile, line))
    {
        std::vector<std::string> vector = tokenize(line, "@%");
        catalog.emplace(vector[0], tokenize(vector[1], ","));
    }

    inputFile.close();
}
/**
 * @brief Represents the result of a document classification.
 */
struct SearchResult {
    string fileName;
    vector<pair<string, int>> TopicsList;
};
/**
 * @brief Writes the classification results to a file.
 * @param match The SearchResult object containing the classification results.
 * @details Appends the classification results to a file named "classification_results.txt".
 *          Each line in the file has the format:
 *          FileName:    Topic1;MatchCount1,    Topic2;MatchCount2,    ...
 */
void writeResultsToFile(const SearchResult& match) {
    std::ofstream outputFile("classification_results.txt", std::ios_base::app);
    if (!outputFile.is_open()) {
        std::cerr << "Error opening file for writing!" << std::endl;
        return;
    }

        outputFile << match.fileName << ":\t";
        for(auto [topic, matchCount] : match.TopicsList) {
            outputFile << topic << ';' << matchCount << ",\t";
        }
    outputFile << '\n';
    outputFile.close();
}
/**
 * @brief Extracts the file name from a given file path.
 * @param filePath The full path of the file.
 * @return The file name.
 */
std::string getFileNameFromPath(const std::string& filePath) {
    // Use filesystem::path to get the filename
    std::filesystem::path pathObj(filePath);
    return pathObj.filename().string();
}
/**
 * @brief Classifies a document based on the catalog.
 * @param filePath The path to the document file.
 * @details Reads the contents of the document, counts matches with identifiers from the catalog,
 *          and stores the results in a SearchResult object.
 */
void classifyDocument(const string& filePath)
{
    std::string fileName = getFileNameFromPath(filePath);
    std::ifstream inputFile(filePath);
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
    while (getline(inputFile, line)) {
        text += line;
    }
    SearchResult result {fileName, {}};
    for (auto const& [topic, identifiers] : catalog) {
        int count = 0;

        for (const std::string& term : identifiers) {
            size_t pos = text.find(term, 0);
            while (pos != std::string::npos) {
                pos = text.find(term, pos + term.length());
                count++;
            }
        }

        result.TopicsList.emplace_back(topic, count);
    }
    writeResultsToFile(result);
}
/**
 * @brief Retrieves all files with specific extensions in a directory.
 * @param directoryPath The path to the directory.
 * @param extensions The list of file extensions to consider.
 * @return Vector of file paths.
 * @details Searches the specified directory for files with extensions provided in the 'extensions' parameter.
 *          Returns a vector containing the paths of these files.
 */
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
/**
 * @brief Main function.
 * @param argc Number of command-line arguments.
 * @param argv Command-line arguments.
 * @return Status code.
 * @details This function is the entry point of the program.
 *          It initializes MPI, broadcasts the catalog to worker processes, and distributes
 *          document classification tasks among worker processes.
 *          The manager process reads the catalog, retrieves document paths, and distributes
 *          them to worker processes for classification.
 *          Worker processes receive their assigned documents, classify them, and write
 *          the results to an output file.
 *          Finally, the program calculates the execution time and prints it (only by rank 0).
 */
int main(int argc, char** argv)
{
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;

    auto t1 = high_resolution_clock::now();
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2)
    {
        cerr << "At least 2 processes are required: 1 manager and 1 worker." << endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (rank == 0)
    {
        readCatalog();
        std::stringstream ss{};
        for (const auto& pair : catalog)
        {
            ss << pair.first << ":";
            for (const auto& item : pair.second)
            {
                ss << item << ",";
            }
            ss << ";";
        }

        std::string serialized_catalog = ss.str();
        int catalogSize = serialized_catalog.size();
        MPI_Bcast(&catalogSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&serialized_catalog[0], serialized_catalog.size(), MPI_CHAR, 0, MPI_COMM_WORLD);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0)
    {
        vector<string> documents = getAllFilesInDirectory("./sample_documents", { ".txt", ".html", ".tex" });

        int numDocumentsPerWorker = documents.size() / (size - 1);
        int remainingDocuments = documents.size() % (size - 1);

        int startIdx = 0;

        for (int i = 1; i < size; ++i)
        {
            int numDocsToSend = numDocumentsPerWorker + (i <= remainingDocuments ? 1 : 0);
            MPI_Send(&numDocsToSend, 1, MPI_INT, i, 0, MPI_COMM_WORLD);

            for (int j = 0; j < numDocsToSend; ++j)
            {
                int docSize = documents[startIdx].size() + 1;
                MPI_Send(&docSize, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                MPI_Send(&documents[startIdx][0], documents[startIdx].size() + 1, MPI_CHAR, i, 0, MPI_COMM_WORLD);
                ++startIdx;
            }
        }
    }
    else
    {
        // Worker processes
        int catalogSize;
        MPI_Bcast(&catalogSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
        char* received_catalog = new char[catalogSize];
        MPI_Bcast(received_catalog, catalogSize, MPI_CHAR, 0, MPI_COMM_WORLD);

        std::stringstream ss;
        ss << received_catalog;
        std::string entry;

        while (std::getline(ss, entry, ';'))
        {
            std::istringstream iss(entry);
            std::string category;
            std::getline(iss, category, ':');
            std::string item;
            std::vector<std::string> items;
            while (std::getline(iss, item, ','))
            {
                items.push_back(item);
            }
            catalog[category] = items;
        }
        int numDocsToReceive;
        MPI_Recv(&numDocsToReceive, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Receive documents
        for (int i = 0; i < numDocsToReceive; ++i)
        {
            int docSize;
            MPI_Recv(&docSize, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            char document[docSize];
            MPI_Recv(document, docSize, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            classifyDocument(document);
        }
    }
    MPI_Finalize();
    auto t2 = high_resolution_clock::now();
    if(rank == 0) {
        /* Getting number of milliseconds as a double. */
    	duration<double, std::milli> ms_double = t2 - t1;
	
    	std::cout << "" << ms_double.count() / 1000 << "s\n";
    }

    return 0;
}

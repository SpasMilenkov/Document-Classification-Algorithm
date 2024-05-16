# Document-Classification-Algorithm

This repository contains implementations of a document classification algorithm using both single-process and MPI-based parallel processing techniques. This project is part of the TU Sofia Parallel Processing of Information course.

## Table of Contents
- [Description](#description)
- [Technologies Used](#technologies-used)
- [Installation](#installation)
- [Usage](#usage)
- [License](#license)

## Description
The Document-Classification-Algorithm project demonstrates two different approaches to document classification:
1. **Single-Process Implementation**: A straightforward implementation that processes documents one by one.
2. **Open MPI-Based Parallel Implementation**: A parallelized version that uses MPI to distribute the workload among multiple processes, improving performance and scalability.

Both versions classify documents based on a catalog of topics and identifiers, reading from text files and determining the relevance of each document to different topics.

### Single-Process Implementation
- Reads a catalog of topics and identifiers.
- Tokenizes and processes each document sequentially.
- Outputs the classification results to a CSV file.

### MPI-Based Parallel Implementation
- Utilizes MPI for parallel processing.
- Distributes document classification tasks across multiple processes.
- Aggregates and outputs the results.

## Technologies Used
- **C++**: The core programming language used for both implementations.
- **Open MPI (Message Passing Interface)**: Used for parallel processing in the MPI-based implementation.
- **Standard Template Library (STL)**: Utilized for data structures and algorithms.
- **Filesystem Library**: Used for directory and file handling.
- **Chrono Library**: Used for measuring execution time.

## Installation
1. Clone the repository:
    ```sh
    git clone https://github.com/yourusername/Document-Classification-Algorithm.git
    ```
2. Navigate to the project directory:
    ```sh
    cd Document-Classification-Algorithm
    ```

### For Single-Process Implementation
3. Compile the single-process code:
    ```sh
    g++ ./main.cpp -o single_classification
    ```

### For MPI-Based Parallel Implementation
4. Compile the MPI-based code:
    ```sh
      mpicxx -I/usr/lib/x86_64-linux-gnu/openmpi/include main.cpp -o parallel
    ```
    ```sh
      mpirun -n 4 paralel
    ```
    Where 4 is the amount of processes you want Open MPI to spawn.

## Usage
### Single-Process Implementation
1. Run the single-process classification:
    ```sh
    ./single_classification
    ```

### MPI-Based Parallel Implementation
2. Run the MPI-based classification (example with 4 processes):
    ```sh
    mpirun -np 4 ./mpi_classification
    ```

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.


*This project is part of the TU Sofia Parallel Processing of Information course.*

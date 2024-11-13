#include "FileSystem.h"
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>

File::File(std::string name) : name(name), content("") {}

Directory::Directory(std::string name) : name(name) {}

FileSystem::FileSystem() {
  root = std::make_shared<Directory>("/");
  currentDirectory = root;
}

void FileSystem::createFile(const std::string& fileName) {
  std::lock_guard<std::mutex> lock(currentDirectory->dirMutex); // Lock for thread safety

  // Check if the file already exists in the current directory
  if (currentDirectory->files.find(fileName) != currentDirectory->files.end()) {
    std::cout << "Error: File '" << fileName << "' already exists in the current directory." << std::endl;
    return;
  }

  // Create a new file and add it to the files map
  currentDirectory->files[fileName] = std::make_shared<File>(fileName);
  std::cout << "File '" << fileName << "' created successfully." << std::endl;
}

void FileSystem::readFile(const std::string& fileName) {
  std::lock_guard<std::mutex> lock(currentDirectory->dirMutex); // Ensure thread safety

  // Check if the file exists in the current directory
  auto fileIter = currentDirectory->files.find(fileName);
  if (fileIter != currentDirectory->files.end()) {
    // Output the content of the file
    std::cout << "Reading from in-memory file '" << fileName << "':" << std::endl;
    std::cout << fileIter->second->content << std::endl; // Display the file content
  } else {
    std::cout << "Error: File '" << fileName << "' not found in memory." << std::endl;
  }
}

void FileSystem::writeFile(const std::string& fileName, const std::string& content) {
  std::lock_guard<std::mutex> lock(currentDirectory->dirMutex); // Ensure thread safety

  // Check if the file exists
  auto fileIter = currentDirectory->files.find(fileName);
  if (fileIter != currentDirectory->files.end()) {
    // If the file exists, update its content
    fileIter->second->content = content;
    std::cout << "File '" << fileName << "' has been updated." << std::endl;
  } else {
    std::cout << "Error: File '" << fileName << "' not found." << std::endl;
  }
}

void FileSystem::deleteFile(const std::string& fileName) {
  std::lock_guard<std::mutex> lock(currentDirectory->dirMutex); // Ensure thread safety

  // Check if the file exists
  auto fileIter = currentDirectory->files.find(fileName);
  if (fileIter != currentDirectory->files.end()) {
    // Erase the file from the directory
    currentDirectory->files.erase(fileIter);
    std::cout << "File '" << fileName << "' has been deleted." << std::endl;
  } else {
    std::cout << "Error: File '" << fileName << "' not found." << std::endl;
  }
}

void FileSystem::createDirectory(const std::string& dirName) {
  std::lock_guard<std::mutex> lock(currentDirectory->dirMutex); // Ensure thread safety

  // Check if directory already exists
  if (currentDirectory->subDirectories.find(dirName) != currentDirectory->subDirectories.end()) {
    std::cout << "Error: Directory '" << dirName << "' already exists." << std::endl;
    return;
  }

  // Create new directory and set its parent
  auto newDir = std::make_shared<Directory>(dirName);
  newDir->parent = currentDirectory; // Set the parent
  currentDirectory->subDirectories[dirName] = newDir;

  std::cout << "Directory '" << dirName << "' created." << std::endl;
}

void FileSystem::changeDirectory(const std::string& dirName) {
  std::lock_guard<std::mutex> lock(currentDirectory->dirMutex); // Ensure thread safety

  // Check if the directory exists
  auto dirIter = currentDirectory->subDirectories.find(dirName);
  if (dirIter != currentDirectory->subDirectories.end()) {
    // Change current directory
    currentDirectory = dirIter->second;
    std::cout << "Changed to directory '" << dirName << "'." << std::endl;
  } else {
    std::cout << "Error: Directory '" << dirName << "' not found." << std::endl;
  }
}

void FileSystem::goToParent() {
  std::lock_guard<std::mutex> lock(fsMutex); // Ensure thread safety for navigating

  if (currentDirectory == root) {
    std::cout << "Already at the root directory." << std::endl;
  } else {
    auto parentDir = currentDirectory->parent.lock(); // Get shared pointer to parent
    if (parentDir) {
      currentDirectory = parentDir;
      std::cout << "Moved to parent directory." << std::endl;
    } else {
      std::cout << "Error: Parent directory no longer exists." << std::endl;
    }
  }
}

void FileSystem::goToRoot() {
  std::lock_guard<std::mutex> lock(fsMutex); // Ensure thread safety

  currentDirectory = root; // Go to root directory
  std::cout << "Moved to root directory." << std::endl;
}

class FileSystemCLI {
private:
  FileSystem fs;

public:
  void start() {
    std::string command;
    std::cout << "Welcome to the File System CLI!" << std::endl;

    // Main loop for the CLI
    while (true) {
      std::cout << "fs> "; // CLI prompt
      std::getline(std::cin, command); // Get command input

      // Exit command
      if (command == "exit") {
        break;
      }

      // Process command
      processCommand(command);
    }
  }

  void processCommand(const std::string& command) {
    std::istringstream stream(command);
    std::string cmd;
    stream >> cmd;

    if (cmd == "create") {
      std::string fileName;
      stream >> fileName;
      fs.createFile(fileName); // Call the createFile function
    } else if (cmd == "read") {
      std::string fileName;
      stream >> fileName;
      fs.readFile(fileName); // Call the readFile function
    } else if (cmd == "write") {
      std::string fileName, content;
      stream >> fileName;
      std::getline(stream, content); // Get the entire content as a string
      fs.writeFile(fileName, content); // Call the writeFile function
    } else if (cmd == "delete") {
      std::string fileName;
      stream >> fileName;
      fs.deleteFile(fileName); // Call the deleteFile function
    } else if (cmd == "createDir") {
      std::string dirName;
      stream >> dirName;
      fs.createDirectory(dirName); // Call the createDirectory function
    } else if (cmd == "cd") {
      std::string dirName;
      stream >> dirName;
      fs.changeDirectory(dirName); // Call the changeDirectory function
    } else if (cmd == "parent") {
      fs.goToParent(); // Go to parent directory
    } else if (cmd == "root") {
      fs.goToRoot(); // Go to root directory
    } else {
      std::cout << "Invalid command." << std::endl;
    }
  }
};

int main() {
  FileSystemCLI cli;
  cli.start();
  return 0;
}
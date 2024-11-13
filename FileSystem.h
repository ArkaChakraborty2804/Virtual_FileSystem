#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <vector>

class File {
public:
  std::string name;
  std::string content;
  File(std::string name);
};

class Directory {
public:
  std::string name;
  std::unordered_map<std::string, std::shared_ptr<Directory>> subDirectories;
  std::unordered_map<std::string, std::shared_ptr<File>> files;
  std::mutex dirMutex;
  std::weak_ptr<Directory> parent;
  
  Directory(std::string name);
};


class FileSystem {
private:
  std::shared_ptr<Directory> root;
  std::shared_ptr<Directory> currentDirectory;  // Tracks the current directory
  std::mutex fsMutex;

public:
  FileSystem();

  void createFile(const std::string& fileName);
  void readFile(const std::string& fileName);
  void writeFile(const std::string& fileName, const std::string& content);
  void deleteFile(const std::string& fileName);
  void createDirectory(const std::string& dirName);
  void changeDirectory(const std::string& dirName);
  void goToParent();
  void goToRoot();
};


#endif
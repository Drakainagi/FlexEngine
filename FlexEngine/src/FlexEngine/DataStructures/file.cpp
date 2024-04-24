#include "pch.h"

#include "file.h"

namespace FlexEngine
{

  // static member initialization
  File::FileRegistry File::s_file_registry;

  #pragma region File Registry Management Functions

  File& File::Open(const Path& path)
  {
    // check if the file already exists
    if (FILE_REGISTRY.count(path) == 0)
    {
      // create a new file
      FILE_REGISTRY[path] = { path, "" };
      // TODO: read the file's contents
    }
    return FILE_REGISTRY[path];
  }

  void File::Close(const Path& path)
  {
    // check if the file exists
    if (FILE_REGISTRY.count(path) != 0)
    {
      // delete the file
      FILE_REGISTRY.erase(path);
    }
  }

  #pragma endregion

  #pragma region File Management Functions

  std::string File::Read()
  {
    FLX_FLOW_FUNCTION();

    // guard
    if (!path.is_file())
    {
      Log::Warning("Attempted to read a non-file: " + path.get().string());
      return data;
    }

    std::ifstream file(path.get(), std::ios::binary);
    if (!file.is_open())
    {
      Log::Error("Failed to open file: " + path.get().string());
      return data;
    }

    // Determine the file size
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    // Read the entire contents of the file into a string
    data.resize(size);
    file.read(data.data(), size);

    if (file.fail())
    {
      Log::Error("Failed to read file: " + path.get().string());
      return data = ""; // return an empty string in case data was partially read/corrupted
    }

    return data;
  }

  void File::Write(const std::string& _data)
  {
    FLX_FLOW_FUNCTION();

    // guard
    if (!path.is_file())
    {
      Log::Warning("Attempted to write to a non-file: " + path.get().string());
      return;
    }

    std::ofstream file(path.get(), std::ios::binary);
    if (!file.is_open())
    {
      Log::Error("Failed to open file: " + path.get().string());
      return;
    }

    file << _data;

    if (file.fail())
    {
      Log::Error("Failed to write to file: " + path.get().string());
      return;
    }
  }

  #pragma endregion

  #pragma region Operator Overloads

  File::operator std::filesystem::path() const { return path; }

  bool operator==(const File& lhs, const File& rhs) { return lhs.path == rhs.path; }
  bool operator!=(const File& lhs, const File& rhs) { return lhs.path != rhs.path; }
  bool operator<(const File& lhs, const File& rhs) { return lhs.path < rhs.path; }
  bool operator>(const File& lhs, const File& rhs) { return lhs.path > rhs.path; }

  #pragma endregion


}

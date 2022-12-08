#include <iostream>
#include <memory>
#include <numeric>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

using FileSize = std::size_t;

class File
{
private:
    std::string _name;
    FileSize _size;

public:
    File()
        : _name("")
        , _size(0)
    {}
    
    File(const std::string & name, FileSize size)
        : _name(name)
        , _size(size)
    {}

    ~File() = default;

    auto name() const { return _name; }
    auto size() const { return _size; }
};

class Directory;
using DirectoryPtr = std::shared_ptr<Directory>;
using DirectoryPtrConst = std::shared_ptr<const Directory>;

class Directory
    : public std::enable_shared_from_this<Directory>
{
public:
private:
    std::optional<DirectoryPtr> _parent;
    std::string _name;
    std::vector<DirectoryPtr> _directories;
    std::vector<File> _files;

public:
    Directory()
        : _parent(std::nullopt)
        , _name("")
    {}

    Directory(DirectoryPtr parent,
              const std::string & name)
        : _parent({parent})
        , _name(name)
    {}

    ~Directory() = default;

    auto parent() const { return _parent; }
    auto name()   const { return _name;   }


    std::optional<DirectoryPtr> getDirectory(const std::string & name)
        const
    {
        auto found = std::find_if(
            _directories.begin(),
            _directories.end(),
            [&name](auto dirPtr){ return name == dirPtr->name(); });

        if (found != _directories.end()) {
            return {*found};
        }

        return {};
    }

    bool hasDirectory(const std::string & name)
        const
    {
        return getDirectory(name).has_value();
    }

    bool hasFile(const std::string & name)
        const
    {
        auto found = std::find_if(
            _files.begin(),
            _files.end(),
            [&name](const auto & file){ return name == file.name(); });

        return found != _files.end();
    }

    std::vector<DirectoryPtrConst> findAllDirectories()
        const
    {
        std::vector<DirectoryPtrConst> result;
        std::vector<DirectoryPtrConst> queue{ shared_from_this() };

        while (!queue.empty()) {
            auto curr = queue.back();
            queue.pop_back();

            result.push_back(curr);

            std::for_each(
                curr->_directories.begin(),
                curr->_directories.end(),
                [&queue](auto dirPtr){ queue.push_back(dirPtr); });
        }

        return result;
    }


    FileSize getTotalSize()
        const
    {
        auto filesSize = std::accumulate(
            _files.begin(),
            _files.end(),
            0,
            [](auto t, const auto & file){ return t + file.size(); });

        auto directoriesSize = std::accumulate(
            _directories.begin(),
            _directories.end(),
            0,
            [](auto t, const auto & dirPtr){ return t + dirPtr->getTotalSize(); });

        return filesSize + directoriesSize;
    }

#ifdef DEBUG
    std::string getPathString()
        const
    {
        auto currentDirectory = std::make_optional(shared_from_this());
        std::stack<std::string> pathComponents;

        do {
            pathComponents.push(currentDirectory.value()->name());
            currentDirectory = currentDirectory.value()->parent();
        } while (currentDirectory.has_value());

        std::stringstream ss;

        do {
            ss << pathComponents.top() << "/";
            pathComponents.pop();
        } while (!pathComponents.empty());

        return ss.str();
    }
#endif

    void addDirectory(std::string directory)
    {
#ifdef DEBUG
        if (hasDirectory(directory)) {
            std::cerr << "attempt to add an existing directory" << std::endl;
            return;
        }
#endif

        _directories.push_back(std::make_shared<Directory>(shared_from_this(), directory));
    }

    void addFile(std::string name, FileSize size)
    {
#ifdef DEBUG
        if (hasFile(name)) {
            std::cerr << "attempt to add an existing file" << std::endl;
            return;
        }
#endif

        _files.push_back({name, size});
    }
};

class InputParser
{
private:
    DirectoryPtr _fileSystemRoot;
    DirectoryPtr _currentDirectory;

    inline static const std::regex cd_cmd{"^\\$ cd ([[:alnum:]_\\./]+)$"};
    inline static const std::regex ls_cmd{"^\\$ ls$"};
    inline static const std::regex file_line{"^(\\d+) ([[:alnum:]_\\.]+)$"};
    inline static const std::regex dir_line{"^dir ([[:alnum:]_\\.]+)$"};

    void changeDirectory(const std::string & directory)
    {
#ifdef DEBUG
        std::cout << ">>> cd " << directory << std::endl;
#endif

        if (directory == "/") {
            _currentDirectory = _fileSystemRoot;
            return;
        }

        if (directory == "..") {
            auto parent = _currentDirectory->parent();

            if (!parent.has_value()) {
                std::cerr << "file system root has no parent directory" << std::endl;
                std::terminate();
            }

            _currentDirectory = parent.value();
            return;
        }

        auto nextDirectory = _currentDirectory->getDirectory(directory);

        if (nextDirectory.has_value()) {
            _currentDirectory = nextDirectory.value();
            return;
        }

        std::cerr << "unable to change to directory: " << directory << std::endl;
        std::terminate();
    }

    void listDirectoryContents()
    {
#ifdef DEBUG
        std::cout << ">>> ls" << std::endl;
#endif
    }

    void addFile(const std::string & name, FileSize size)
    {
#ifdef DEBUG
        std::cout << name << " :: " << size << std::endl;
#endif

        _currentDirectory->addFile(name, size);
    }

    void addDirectory(const std::string & name)
    {
#ifdef DEBUG
        std::cout << name << " :: DIRECTORY" << std::endl;
#endif

        _currentDirectory->addDirectory(name);
    }

    void parseLine(std::string line)
    {
        std::smatch matches;

        if (std::regex_match(line, matches, cd_cmd)) {
            changeDirectory(matches[1]);
        }
        else if (std::regex_match(line, matches, ls_cmd)) {
            listDirectoryContents();
        }
        else if (std::regex_match(line, matches, file_line)) {
            addFile(matches[2], std::stoi(matches[1]));
        }
        else if (std::regex_match(line, matches, dir_line)) {
            addDirectory(matches[1]);
        }
        else {
            std::cerr << "unable to parse line: \"" << line << "\"" << std::endl;
            std::terminate();
        }
    }
public:
    InputParser()
        : _fileSystemRoot(std::make_shared<Directory>())
    {}

    ~InputParser() = default;
    
    DirectoryPtr parseInput(std::istream & is)
    {
        for (std::string line; std::getline(is, line);) {
            parseLine(line);
        }

        return _fileSystemRoot;
    }
};

constexpr FileSize fileSystemSizeLimit = 70000000;
constexpr FileSize updateSize = 30000000;

FileSize getScore(const DirectoryPtr & fileSystemRoot)
{
    auto usedSize = fileSystemRoot->getTotalSize();
    auto unusedSize = fileSystemSizeLimit - usedSize;
    auto requiredSize = updateSize - unusedSize;

#ifdef DEBUG
    std::cout
        << "Total disk space:  " << fileSystemSizeLimit << std::endl
        << "Used disk space:   " << usedSize << std::endl
        << "Unused disk space: " << unusedSize << std::endl
        << "Update size:       " << updateSize << std::endl
        << "Required size:     " << requiredSize << std::endl;
#endif

    auto dirs = fileSystemRoot->findAllDirectories();
    decltype(dirs) candidates;

    std::for_each(
        dirs.begin(),
        dirs.end(),
        [&candidates, requiredSize](auto dirPtr) {
            if (dirPtr->getTotalSize() >= requiredSize) {
                candidates.push_back(dirPtr);
            }
        });

    std::sort(
        candidates.begin(),
        candidates.end(),
        [](auto dirPtrA, auto dirPtrB) {
            return dirPtrA->getTotalSize() < dirPtrB->getTotalSize();
        });

    return candidates.front()->getTotalSize();
}

int main()
{
    InputParser inputParser;
    DirectoryPtr fileSystemRoot(inputParser.parseInput(std::cin));

    std::cout << getScore(fileSystemRoot) << std::endl;

    return 0;
}

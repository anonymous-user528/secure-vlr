#pragma once

#include <string>
#include <map>

class ConfigFile{
public:
    ConfigFile(std::string const& file_path);
    std::string const& value(std::string const& section, std::string const& entry) const;
private:
    std::map<std::string, std::string> content;
};


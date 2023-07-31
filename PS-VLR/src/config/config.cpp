#include "config.h"
#include <fstream>

std::string trim(const std::string& source, const char* delims = " \t\r\n"){
    std::string result(source);
    std::string::size_type index = result.find_last_not_of(delims);
    if(index != std::string::npos){
        result.erase(++index);
    }
    index = result.find_first_not_of(delims);
    if(index != std::string::npos){
        result.erase(0, index);
    }
    else{
        result.erase();
    }
    return result;
}

ConfigFile::ConfigFile(const std::string& configFile){
    std::ifstream file(configFile.c_str());

    std::string line;
    std::string name;
    std::string value;
    std::string inSection;
    int posEqual;
    while(std::getline(file, line)){
        if(!line.length() || line[0] == '#' || line[0] == ';'){
            continue;
        }
        else if(line[0] == '['){
            inSection = trim(line.substr(1, line.find(']') - 1));
            continue;
        }
        else {
            posEqual = line.find('=');
            name = trim(line.substr(0, posEqual));
            value = trim(line.substr(posEqual + 1));
            content[inSection + '/' + name] = value;
        }
    }
}

std::string const& ConfigFile::value(const std::string& section, const std::string& entry) const{
    std::map<std::string, std::string>::const_iterator ci = content.find(section + '/' + entry);
    if(ci == content.end()){
        throw "does not exist";
    }
    else{
        return ci->second;
    }
}
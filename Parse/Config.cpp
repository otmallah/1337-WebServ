#include "Config.hpp"

/*
    constructor that does all the needed work
    takes the config file name as a parameter
*/
Config::Config(std::string s)
{
    readFileIntoBuff(s);

    size_t end = 0;
    while (end < _fileBuff.size())
    {
        serverParse server = serverParse(_fileBuff, end);
        end = server.parseBlock();
        this->servers.push_back(server);
    }
    if (_fileBuff.size() == 0)
        error("Empty config file");
}

/*
    reads the given file into a vector of strings
*/
void Config::readFileIntoBuff(const std::string &s)
{
    _configFile.open(s.c_str(), std::fstream::in);
    if (!_configFile.is_open())
    {
        perror("open()");
        exit(1);
    }
    std::string tmp;
    while (std::getline(_configFile, tmp))
    {
        tmp = trim(tmp);
        if (!tmp.empty())
            _fileBuff.push_back(tmp);
    }
    _configFile.close();
}

/*
    clears the vector of servers;
*/
Config::~Config() { servers.clear(); }


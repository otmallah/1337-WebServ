#include "Config.hpp"

/*
    gets the path of the location
    if the path is missing an error is written to stderr
    and the programme is exited
*/
void Config::locationParse::collectPath(const std::string &s)
{
    size_t i = 0;
    while (s[i] && !isWhiteSpace(s[i]))
        i++;
    while (s[i] && isWhiteSpace(s[i]))
        i++;
    while (s[i])
        path += s[i++];
    if (path.empty())
        error("Empty location path");
}


/*
    a constructor that initialize the needed data
*/
Config::locationParse::locationParse(const std::vector<std::string> &a, int i)
    : _start(i), _fileBuff(a), autoIndex(OFF) {}

/*
    checks if passed key matches any known directives 
    if yes the corresponding pair is set to <key, values> with values set to 
    the passed argument if the key does not match an error is thrown
*/
void Config::locationParse::setDirective(const std::string &key, std::vector<std::string> &values)
{
    if (key == "root")
        _root = std::make_pair(key, values);
    else if (key == "index")
        _index = std::make_pair(key, values);
    else if (key == "error_page")
    {
        if (values.size() < 2)
            error("Invalid directive arguments");
        if (!isNumber(values.front()))
            error("Invalid directive arguments");
        this->errorPages.insert(std::make_pair(atoi(values.front().c_str()),
                                               values.back()));
    }
    else if (key == "body_size")
    {
        if (values.size() !=  1)
            error("Invalid directive arguments");
        if (!isNumber(values.front()))
            error("Invalid directive arguments");
        this->_bodySize = std::make_pair(key, values);
    }
    else if (key == "allowed_methods")
        _allowedMethods = std::make_pair(key, values);
    else if (key == "auto_index")
    {
        if (values.size() != 1)
            error("Invalid arguments");
        if (values.front() != "on" and values.front() != "off")
            error("Invalid directive arguments");
        autoIndex = values.front() == "on" ? ON : OFF;
    }
    else if (key == "upload_path")
    {
        if (values.size() != 1)
            error("Invalid directive arguments");
        _uploadPath = std::make_pair(key, values);
    }
    else if (key == "upload")
    {
        if (values.size() != 1)
            error("Invalid directive arguments");
        if (values.front() != "on" and values.front() != "off")
            error("Invalid directive Arguments");
        _upload = std::make_pair(key, values);
    }
    else if (key == "redirect")
    {
        if (values.size() != 2)
            error("Invalid directive arguments");
        if (!isNumber(values.front()))
            error("Invalid directive arguments");
        _redirect = std::make_pair(key, values);
    }
    else
    {
        if (key != "{" && key != "}")
            error("miplaced or invalid directive");
    }
}

/*
    the logic implementation of filling the directives
    here where the collection of key and values is gathered
*/
void Config::locationParse::fillDirective(const std::string &s, const std::string &key)
{
    size_t i = 0;
    std::string val;
    std::vector<std::string> values;

    while (s[i] && !isWhiteSpace(s[i]))
        i++;
    while (s[i] && isWhiteSpace(s[i]))
        i++;
    while (s[i])
    {
        while (isWhiteSpace(s[i]))
            i++;
        val += s[i];
        i++;
        if (!s[i] or isWhiteSpace(s[i]))
        {
            values.push_back(val);
            val = "";
        }
    }
    setDirective(key, values);
}

/*
    a destructor that frees the data
*/
Config::locationParse::~locationParse()
{
    std::map<std::string, std::vector<std::string> >::iterator it = this->data.begin();
    while (it != data.end())
    {
        it->second.clear();
        ++it;
    }
    this->data.clear();
}

/*
    the logic implementation of parsing a location block
*/
void Config::locationParse::parseBlock()
{
    size_t i = _start;
    size_t j = 0;
    std::string key;
    bool isInsideLocation = false;
    std::vector<std::string> newBuff;
    while (i < _fileBuff.size() && _fileBuff[i] != "}")
    {
        j = 0;
        key.clear();
        while (_fileBuff[i][j] && !isWhiteSpace(_fileBuff[i][j]))
            key += _fileBuff[i][j++];
        while (_fileBuff[i][j] && isWhiteSpace(_fileBuff[i][j]))
            j++;
        if (key == "location")
        {
            collectPath(_fileBuff[i]);
            isInsideLocation = true;
        }
        else
        {
            if (isInsideLocation)
                fillDirective(_fileBuff[i], key);
        }
        if (_fileBuff[i] == "}")
            isInsideLocation = false;
        i++;
    }
    data.insert(this->_upload);
    data.insert(this->_root);
    data.insert(this->_index);
    data.insert(this->_uploadPath);
    data.insert(this->_allowedMethods);
    data.insert(this->_bodySize);
    data.insert(this->_redirect);
}
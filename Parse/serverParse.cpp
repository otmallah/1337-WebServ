#include "Config.hpp"

Config::serverParse::serverParse(const std::vector<std::string> &__fileBuff, int __start)
{
    _lastKey = "";
    _serverIsOpened = _locationIsOpened = false;
    _isInsideServer = false;
    _fileBuff = __fileBuff;
    _start = __start;
}

bool isCurlyBracket(const std::string &s)
{
    return (s == "{" or s == "}");
}

void Config::serverParse::handleErrors(const std::string &_fileBuff)
{
    if (isCurlyBracket(_fileBuff) or _fileBuff == "server")
    {
        if (_fileBuff == "{")
        {
            if (_lastKey == "server")
                _serverIsOpened = true;
            else if (_lastKey == "location")
                _locationIsOpened = true;
            else
                error("invalid config file");
        }
        else if (_fileBuff == "}")
        {
            if (_locationIsOpened)
                _locationIsOpened = false;
            else if (_serverIsOpened)
                _serverIsOpened = false;
            else
                error("invalid config file");
        }
    }
}

std::string Config::serverParse::getKey(const std::string &_fileBuff, int &j)
{
    std::string key;
    while (_fileBuff[j] && !isWhiteSpace(_fileBuff[j]))
        key += _fileBuff[j++];
    return key;
}

void Config::serverParse::getValues(std::vector<std::string> &values,
                            const std::string &_fileBuff, int &j)
{
    std::string val;
    while (_fileBuff[j])
    {
        while (_fileBuff[j] && isWhiteSpace(_fileBuff[j]))
            j++;
        val += _fileBuff[j];
        j++;
        if (!_fileBuff[j] or isWhiteSpace(_fileBuff[j]))
        {
            values.push_back(val);
            val = "";
        }
    }
}

size_t Config::serverParse::parseBlock()
{
    size_t i = _start;

    int j = 0;
    std::string key;
    std::string val;
    int locationsCount = 0;
    std::vector<std::string> values;

    while (i < _fileBuff.size())
    {
        _isInsideServer = !_locationIsOpened;
        j = 0;
        values.clear();
        _lastKey = key;
        val = key = "";
        handleErrors(_fileBuff[i]);
        key = getKey(_fileBuff[i], j);
        if ((key == "server" && _fileBuff[i + 1] != "{") ||
            (key == "location" && _fileBuff[i + 1] != "{"))
            error("block not opened");
        if (key == "location")
        {
            if (_locationIsOpened)
                error("unclosed block");
            locations.push_back(locationParse(this->_fileBuff, i));
            locations[locationsCount].parseBlock();
            locationsCount++;
        }
        getValues(values, _fileBuff[i], j);
        fillDirective(key, values);
        i++;
        if (key == "}" and _lastKey == "}")
            break;
    }
    if (_serverIsOpened or _locationIsOpened)
        error("block not closed");
    insertDirectives();
    fillEmptyRequiredDirectives();
    return i;
}

void Config::serverParse::insertDirectives(void)
{
    data.insert(_cgiPath);
    data.insert(_upload);
    data.insert(_index);
    data.insert(_root);
    data.insert(_port);
    data.insert(_serverName);
    data.insert(_allowedMethods);
    data.insert(_upload_path);
    data.insert(_bodySize);
}

void Config::serverParse::fillEmptyRequiredDirectives(void)
{
    for (size_t i = 0; i < locations.size(); i++)
    {
        if (locations[i].data["root"].size() == 0)
            locations[i].data["root"] = this->data["root"];
        if (locations[i].data["index"].size() == 0)
            locations[i].data["index"] = this->data["index"];
        if (locations[i].data["allowed_methods"].size() == 0)
            locations[i].data["allowed_methods"] = this->data["allowed_methods"];
        if (locations[i].data["cgi_path"].size() == 0)
            locations[i].data["cgi_path"] = this->data["cgi_path"];
        if (locations[i].data["upload"].size() == 0)
            locations[i].data["upload"] = this->data["upload"];
        if (locations[i].data["upload_path"].size() == 0)
            locations[i].data["upload_path"] = this->data["upload_path"];
        if (locations[i].data["body_size"].size() == 0)
            locations[i].data["body_size"] = this->data["body_size"];
    }
}

bool Config::serverParse::isWhiteSpace(char c)
{
    return (c == ' ' or c == '\t');
}

void Config::serverParse::error(const std::string &s) const
{
    std::cerr << s << std::endl;
    exit(EXIT_FAILURE);
}

bool Config::serverParse::isNumber(const std::string &s)
{
    for (size_t i = 0; i < s.size(); i++)
    {
        if (!isdigit(s[i]))
            return false;
    }
    return true;
}

void Config::serverParse::fillDirective(const std::string &key,
                                const std::vector<std::string> &values)
{
    if (key == "listen")
        _port = std::make_pair(key, values);
    else if (key == "upload")
    {
        if (!_locationIsOpened)
        {
            if (values.size() != 1)
                error("Invalid arguments");
            if (values.at(0) != "on" && values.at(0) != "off")
                error("invalid arguments");
            _upload = std::make_pair(key, values);
        }
    }
    else if (key == "body_size")
    {
        if (!_locationIsOpened)
        {
            if (values.size() != 1)
                error("Invalid arguments");
            _bodySize = std::make_pair(key, values);
        }
    }
    else if (key == "upload_path")
    {
        if (!_locationIsOpened)
        {
            if (values.size() != 1)
                error("Invalid arguments");
            _upload_path = std::make_pair(key, values);
        }
    }
    else if (key == "cgi_path")
        _cgiPath = std::make_pair(key, values);
    else if (key == "root")
    {
        if (!_locationIsOpened)
            _root = std::make_pair(key, values);
    }
    else if (key == "index")
    {
        if (!_locationIsOpened)
            _index = std::make_pair(key, values);
    }
    else if (key == "auto_index")
    {
        if (!_locationIsOpened)
        {
            if (values.size() != 1)
                error("Invalid Arguments");
            if (values.front() != "on" and values.front() != "off")
                error("Invalid Arguments");
            if (!_locationIsOpened)
                autoIndex = values[0] == "on" ? true : false;
        }
    }
    else if (key == "server_name")
        _serverName = std::make_pair(key, values);
    else if (key == "allowed_methods")
    {
        if (!_locationIsOpened)
            _allowedMethods = std::make_pair(key, values);
    }
    else if (key == "error_page")
    {
        if (values.size() < 2)
            error("Invalid Arguments");
        if (!isNumber(values.front()))
            error("Invalid Arguments");
        if (!_locationIsOpened)
            errorPages[atoi(values.front().c_str())] = values.back();
    }
    else
    {
        if (key != "location" && key != "server" && !isCurlyBracket(key))
            error("invalid directive");
    }
}

std::string Config::serverParse::trim(const std::string &s)
{
    std::string trimmed;
    int _start = 0;
    int end = s.size() - 1;
    if (end < 0)
        return "";
    while (s[_start] && isWhiteSpace(s[_start]))
        _start++;
    while (end > 0 && isWhiteSpace(s[end]))
        end--;
    while (_start <= end)
        trimmed += s[_start++];
    return trimmed;
}

Config::serverParse::~serverParse()
{
    std::map<std::string, std::vector<std::string> >::iterator it = this->data.begin();
    while (it != data.end())
    {
        it->second.clear();
        ++it;
    }
    this->data.clear();
}
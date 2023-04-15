#include "requestParse.hpp"

void requestParse::collectCookies(std::string &headerValue)
{
    size_t pos;
    while (true)
    {
        pos = headerValue.find(";");
        if (pos == std::string::npos)
            break;
        std::string cookie = headerValue.substr(0, pos);
        size_t start = cookie.find_first_not_of(" ");
        size_t end = cookie.find_last_not_of(" ");
        cookie = cookie.substr(start, end + 1);
        this->cookies.insert(cookie);
        headerValue.erase(0, pos + 1);
    }
    if (!headerValue.empty())
    {
        size_t start = headerValue.find_first_not_of(" ");
        size_t end = headerValue.find_last_not_of(" ");
        headerValue = headerValue.substr(start, end);
        this->cookies.insert(headerValue);
    }
}

void requestParse::setUp(std::string _requestParse)
{
    std::stringstream ss(_requestParse);
    ss >> this->data["method"] >> this->data["path"] >> this->data["version"];
    std::string headerName, headerValue;
    while (getline(ss, headerName, ':') && getline(ss, headerValue))
    {
        if (headerValue.size() > 3)
            headerValue = headerValue.substr(1, headerValue.size() - 2);
        removeWhiteSpaces(headerName);
        if (headerName == "Cookie")
            collectCookies(headerValue);
        if (headerName == "Content-Length")
            this->data["content-length"] = headerValue;
        else if (headerName == "Host")
            this->data["host"] = headerValue;
        else if (headerName == "Content-Type")
            this->data["content-type"] = headerValue;
    }
    if (this->data["content-length"].empty() && this->data["method"] != "GET")
        this->data["transfer-encoding"] = "Chunked";
    size_t pos = _requestParse.find("\r\n\r\n");
    if (pos == std::string::npos)
        return;
    _requestParse.erase(0, pos + 4);
    this->body.content = _requestParse;
}

requestParse::requestParse() {}

requestParse::~requestParse()
{
    this->data.clear();
    this->cookies.clear();
}
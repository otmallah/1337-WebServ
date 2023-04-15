#if !defined(requestParse_HPP)
#define requestParse_HPP

#include "../inc.hpp"

struct Body
{
    std::string content;
    std::string contentName;
};

class requestParse
{
private:
    void parseRequestLine(std::string &s, const std::string &delimiter);
    void getHost(std::string &s);
public:
    void collectCookies(std::string &headerValues);
    void converChunkedRequest();
    void setUp(std::string);
    std::set<std::string> cookies;
    std::map<std::string, std::string> data;
    Body body;
    requestParse();
    ~requestParse();
};

#endif // requestParse_HPP
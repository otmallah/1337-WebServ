#include "../inc.hpp"

bool isWhiteSpace(char c)
{
    return (c == ' ' or c == '\t');
}

void error(const std::string &s)
{
    std::cerr << s << std::endl;
    exit(EXIT_FAILURE);
}

void error_fatal(const char *s)
{
    perror(s);
    exit(1);
}

bool isNumber(const std::string &s)
{
    for (size_t i = 0; i < s.size(); i++)
    {
        if (!isdigit(s[i]))
            return false;
    }
    return true;
}

bool isCurlyBracket(const std::string &s)
{
    return (s == "{" or s == "}");
}

void removeWhiteSpaces(std::string &headerName)
{
    size_t i;
    for (i = 0; i < headerName.size(); ++i)
        if (headerName[i] != '\n' && headerName[i] != '\r' && headerName[i] != ' ')
            break;
    headerName = headerName.substr(i);
}

/*
    returns a new string with spaces trimmed from
    the beginning and the end of the string
*/
std::string trim(const std::string &s)
{
    std::string trimmed;
    int start = 0;
    int end = s.size() - 1;
    if (end < 0)
        return "";
    while (s[start] && isWhiteSpace(s[start]))
        start++;
    while (end > 0 && isWhiteSpace(s[end]))
        end--;
    while (start <= end)
        trimmed += s[start++];
    return trimmed;
}
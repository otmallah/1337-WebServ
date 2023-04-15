#if !defined(INC_HPP)
#define INC_HPP

#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <iostream>
#include <map>
#include <vector>
#include <fcntl.h>
#include <dirent.h>
#include <fstream>
#include <algorithm>
#include <cstdlib> // for exit()
#include <cstring> // for memset()
#include <sstream>
#include <set>

bool isWhiteSpace(char c);
void error(const std::string &s);
void error_fatal(const char *s);
bool isNumber(const std::string &s);
std::string trim(const std::string &s);
void removeWhiteSpaces(std::string &headerName);
bool isCurlyBracket(const std::string &s);
#endif // INC_HPP

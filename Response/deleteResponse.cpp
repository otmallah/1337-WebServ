/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   deleteResponse.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: otmallah <otmallah@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/31 01:28:20 by otmallah          #+#    #+#             */
/*   Updated: 2023/04/08 02:58:28 by otmallah         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include <sys/stat.h>

int     Response::checkPathOfDeletemethod(Config::serverParse& server, std::string line, int index)
{
    std::string path;
    _indexLocation = index;
    path = server.locations[index].data["root"][0] + line;
    _deletePath = path;
    DIR *dir = opendir(path.c_str());
    if (!dir)
    {
        std::ifstream infile;
        infile.open(path.c_str());
        if (infile)
            return true;
        return false;
    }
    if (methodAllowed(server, index) == false)
    {
        return false;
    }
    if (path[path.size() - 1] != '/')
    {
        std::cout << "301 moved -> path = " << path << std::endl;
        path += "/";
        _deletePath = path;
        _statusCode = 301;
    }
    return true;
}

void    Response::success()
{
    std::ifstream infile("./index/200.html");
    std::string line;
    while (getline(infile,  line))
        _body += line;
    postResponse();

}

int     Response::deleteMethod(Config& config)
{
    struct stat filestat;
    if (getMatchedLocation(config) == false)
    {
        errorPages(config.servers[0], 0, 404);
        return 0;
    }
    DIR *dir = opendir(_deletePath.c_str());
    if (dir)
    {
        dirent *name = readdir(dir);
        int _ok = 0;
        while (name != NULL)
        {
            std::string file = _deletePath + name->d_name;
            if (stat(file.c_str(), &filestat) == 0) 
            {
                if ((S_ISDIR(filestat.st_mode)))
                {
                    if (filestat.st_mode & S_IREAD)
                        {}
                    else
                    {
                        _ok = 1;
                        break;
                    }
                }
                if (!(filestat.st_mode & S_IWUSR))
                {
                    _ok = 1;
                    break;
                }
            }
            name = readdir(dir);
        }
        if (_ok == 0)
        {
            dir = opendir(_deletePath.c_str());
            dirent *name = readdir(dir);
            while (name != NULL)
            {
                std::string file = _deletePath + name->d_name;
                remove(file.c_str());
                name = readdir(dir);
            }
            success();
            return 0;
        }
        errorPages(config.servers[_indexServer], _indexLocation, 403);
        postResponse();
        return 0;
    }
    else if (stat(_deletePath.c_str(), &filestat) == 0)
    {
        if (filestat.st_mode & S_IWUSR)
        {
            remove(_deletePath.c_str());
            success();
        }
        else
            errorPages(config.servers[_indexServer], _indexLocation, 403);
    }
    return 0;
}
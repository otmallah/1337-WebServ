/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   postResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ytouate <ytouate@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/28 14:57:39 by otmallah          #+#    #+#             */
/*   Updated: 2023/04/12 17:53:57 by ytouate          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#define CHUNK 1024

std::string _checker;
int Response::checkPathOfPostmethod(Config::serverParse &server, std::string line, int index)
{
    std::string path;
    path = line;
    _postPath = path;
    if (server.data["upload"].size() > 0 && server.data["upload"].front() == "on")
    {
        if (server.data["upload_path"].size() > 0)
            _uploadPath = server.data["upload_path"].front();
        _indexLocation = index;
    }
    if (server.locations[index].data["redirect"].size() > 0)
    {
        int status = atoi(server.locations[index].data["redirect"][0].c_str());
        if (status >= 300 && status < 400)
            path = server.locations[index].data["redirect"][1];
        else
        {
            _body += server.locations[index].data["redirect"][1];
            return true;
        }
    }
    size_t pos = path.find("?");
    if (pos != std::string::npos)
    {
        std::string cookie = path.substr(pos + 1);
        path.erase(pos);
        request.collectCookies(cookie);
    }
    DIR *dir = opendir(path.c_str());
    if (!dir)
    {
        std::ifstream infile;
        infile.open(path.c_str());
        if (infile)
        {
            infile.close();
            return true;
        }
        infile.close();
        return false;
    }
    if (methodAllowed(server, index) == false)
        return false;
    if (path[path.size() - 1] != '/')
    {
        path += "/";
        _statusCode = 301;
        postResponse();
        return false;
    }
    closedir(dir);
    return true;
}

void Response::postResponse(void)
{
    char buffer[100];
    if (_flag == 1)
    {
        sprintf(buffer, "%s 200 OK\r\n", request.data["version"].c_str());
        this->_response += buffer;
    }
    else
    {

        sprintf(buffer, "%s %d\r\n", request.data["version"].c_str(), _statusCode);
        this->_response += buffer;
        _header += buffer;
        sprintf(buffer, "content-type : %s\r\n\r\n", "text/html");
        this->_response += buffer;
        _header += buffer;
    }
    this->_response += _body;
}

void Response::postType(std::string path)
{
    if (path == "text/css")
        this->_contentType = ".css";
    else if (path == "text/csv")
        this->_contentType = ".csv";
    else if (path == "image/gif")
        this->_contentType = ".gif";
    else if (path == "text/html")
        this->_contentType = ".htm";
    else if (path == "text/html" || path == ".php")
        this->_contentType = ".html";
    else if (path == "image/x-icon")
        this->_contentType = ".ico";
    else if (path == "image/jpeg")
        this->_contentType = ".jpeg";
    else if (path == "image/jpeg")
        this->_contentType = ".jpg";
    else if (path == "application/javascript")
        this->_contentType = ".js";
    else if (path == "application/json")
        this->_contentType = ".json";
    else if (path == "image/png")
        this->_contentType = ".png";
    else if (path == "application/pdf")
        this->_contentType = ".pdf";
    else if (path == "image/svg+xml")
        this->_contentType = ".svg";
    else if (path == "text/plain")
        this->_contentType = ".txt";
    else if (path == "video/mp4")
        this->_contentType = ".mp4";
    else if (path == "video/webm")
        this->_contentType = ".WebM";
    else if (path == "video/ogg")
        this->_contentType = ".Ogg";
    else if (path == "video/x-msvideo")
        this->_contentType = ".AVI";
    else if (path == "video/mpeg")
        this->_contentType = ".MPEG";
    else if (path == "application/zip")
        this->_contentType = ".zip";
    else if (path == "image/tiff")
        this->_contentType = ".tiff";
}

int Response::postMethod(Config &config)
{
    srand(time(0));
    int number = rand() / 3;
    std::string line;
    if (getMatchedLocation(config) == true)
    {
        if (_indexLocation != -1)
        {
            line = _postPath;
            if (line.erase(0, line.rfind(".")) == ".php")
            {
                if (!config.servers[_indexServer].locations[_indexLocation].data["cgi"].empty())
                {
                    executeCgi(config.servers[_indexServer], 0, 1);
                    postResponse();
                    return 0;
                }
            }
            
            if (request.body.content.size() > 0 && request.body.contentName.size() == 0)
            {
                postType(request.data["content-type"]);
                request.body.contentName = std::to_string(number) + _contentType;
            }
            if (request.body.content.size() < 4)
            {
                _statusCode = 403;
                _body += "<h1> BODY KHAWI </h1>";
                postResponse();
                return 0;
            }
            if (_uploadPath.size() > 0)
            {
                DIR *dir = opendir(_uploadPath.c_str());
                if (!dir)
                    _uploadPath = "./upload/";
            }
            else
                _uploadPath = "./upload/";
            _statusCode = 201;
            uploded = true;
            _uploadPath += "upload_";
            _uploadPath += request.body.contentName;
            _fd = open(_uploadPath.c_str(), O_CREAT | O_TRUNC | O_RDWR, 0644);
            fdIsOpened = true;
            _body += "<h1> kolchi daze </h1>";
            postResponse();
            return 0;
        }
        _indexLocation = 0;
        DIR *dir = opendir(_uploadPath.c_str());
        if (dir)
        {
            if (config.servers[_indexServer].locations[_indexLocation].data["index"].size() > 0)
            {
                if (config.servers[_indexServer].locations[_indexLocation].data["cgi_path"].size() > 0)
                {
                    executeCgi(config.servers[_indexServer], _indexLocation, 1);
                    return 0;
                }
            }
            errorPages(config.servers[_indexServer], _indexLocation, 403);
            closedir(dir);
        }
        else
        {
            closedir(dir);
            if (config.servers[_indexServer].locations[_indexLocation].data["cgi_path"].size() > 0)
            {
                executeCgi(config.servers[_indexServer], _indexLocation, 1);
                return 0;
            }
            errorPages(config.servers[_indexServer], _indexLocation, 403);
        }
    }
    else
    {
        if (_body.size() == 0)
            errorPages(config.servers[_indexServer], _indexLocation, 404);
    }
    postResponse();
    return 0;
}
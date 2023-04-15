/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: otmallah <otmallah@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/13 18:03:24 by otmallah          #+#    #+#             */
/*   Updated: 2023/04/12 03:14:30 by otmallah         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _RESPONSE_HPP_
#define _RESPONSE_HPP_

#include "../inc.hpp"
#include "../Parse/requestParse.hpp"
#include "../Parse/Config.hpp"


class Response
{
    private:
        int         _statusCode;
        int         _fdFile;
        std::string _contentType;
        std::string _requestPath;
        std::string _checker;
        requestParse request;
        std::string _body;
        std::string _postPath;
        std::string _uploadPath;
        std::string _deletePath;
        std::string _getPath;
        int         _flag;
        bool    getMatchedLocation(Config& config);
        bool    checkPathIfValid(Config::serverParse& server, int index , std::string line);
        void    getContentType();
        void    faildResponse();
        int     getIndexOfServerBlock(Config &config);
        int     checkContent();
        void    errorPages(Config::serverParse& server, int id, int statusCode);
        bool    methodAllowed(Config::serverParse& server, int index);
        bool    validFile(Config::serverParse& server, int index, std::string path);
        int     getMethod(Config &config);
        int     postMethod(Config& config);
        int     deleteMethod(Config& config);
        int     checkPathOfPostmethod(Config::serverParse& server, std::string line, int index);
        int     checkPathOfDeletemethod(Config::serverParse& server, std::string line, int index);
        int    validateRequest();
        void    postResponse(void);
        void    success();
        void    postType(std::string path);
        bool    validRequestFormat(Config &config);
        bool    noLocations(Config& config, int index);
        bool    cgiPython(Config::serverParse& server, int index);
        std::vector<std::string>    setEnv();
    public:
        bool    executeCgi(Config::serverParse& server, int index, int flag);
        bool    _executeCgi(const std::string _body);
        bool        uploded;
        int         _indexLocation;
        int         _indexServer;
        long        _contentLength;
        int         _fd;
        bool        fdIsOpened;
        std::string _header;
        std::string _response;
        Response(Config &config, requestParse& _request);
        Response();
        void setUp(Config &config, requestParse& _request);
        ~Response();
};

#endif

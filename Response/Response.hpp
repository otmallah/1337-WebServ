/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ytouate <ytouate@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/13 18:03:24 by otmallah          #+#    #+#             */
/*   Updated: 2023/04/07 23:56:30 by ytouate          ###   ########.fr       */
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
        long        _contentLength;
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
        int         _indexLocation;
        int         _indexServer;
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
        bool    executeCgi(Config::serverParse& server, int index, int flag);
        void    postResponse(void);
        void    success();
        void    postType(std::string path);
        std::vector<std::string>    setEnv();
    public:
        char **g_env;
        std::string _header;
        std::string _response;
        Response(Config &config, requestParse& _request);
        ~Response();
};

#endif

#include "../inc.hpp"
#include "Server.hpp"
#include "../Parse/requestParse.hpp"
#include "../Response/Response.hpp"

#include "../Parse/Config.hpp"

#define MAX_REQUEST_SIZE 4096
#define MAX_CHUNK_SIZE 524

// Write the last error happened in s method and exit

/*
    initializing the socket and binding it to local address
    and listening for new connections
*/
void Server::initServerSocket(const char *host, const char *port)
{
    struct addrinfo hints, *data;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;
    int statusCode = getaddrinfo(host, port, &hints, &data);
    if (statusCode)
    {
        std::cerr << gai_strerror(statusCode) << std::endl;
        std::exit(1);
    }
    int ret = socket(data->ai_family, data->ai_socktype, data->ai_protocol);
    if (ret == -1)
        error_fatal("socket()");
    _serverSockets.push_back(ret);
    int optval = 1;
    setsockopt(_serverSockets.back(), SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    fcntl(_serverSockets.back(), F_SETFL, O_NONBLOCK);
    if (bind(_serverSockets.back(), data->ai_addr, data->ai_addrlen))
        error_fatal("bind()");
    if (listen(_serverSockets.back(), 0))
        error_fatal("listen()");
    freeaddrinfo(data);
}

/*
    adds new client to the _readyToWriteTo and _readyToReadFrom fd_sets
    and handles communication with already connected clients using select
*/
void Server::waitForClients()
{
    FD_ZERO(&_readyToReadFrom);
    FD_ZERO(&_readyToWriteTo);
    int maxSocket = _serverSockets.front();
    for (size_t i = 0; i < _serverSockets.size(); ++i)
    {
        FD_SET(_serverSockets[i], &_readyToReadFrom);
        if (_serverSockets[i] > maxSocket)
            maxSocket = _serverSockets[i];
    }
    std::vector<Client>::iterator it = _clients.begin();
    while (it != _clients.end())
    {
        FD_SET(it->socket, &_readyToReadFrom);
        FD_SET(it->socket, &_readyToWriteTo);
        if (it->response.fdIsOpened && it->response._fd != -1)
        {
            FD_SET(it->response._fd, &_readyToReadFrom);
            if (it->response._fd > maxSocket)
                maxSocket = it->response._fd;
        }
        if (it->socket > maxSocket)
            maxSocket = it->socket;
        ++it;
    }
    if (select(maxSocket + 1, &_readyToReadFrom, &_readyToWriteTo, 0, 0) == -1)
        error_fatal("select()");
}

/*
    Destructor that clears the _clients vector
*/
Server::~Server()
{
    _clients.clear();
}

/*
    returns the client's ip address in form of a string
*/
const std::string Client::getClientAddress()
{
    char buff[100];
    getnameinfo((struct sockaddr *)&this->clientAddress,
                this->addressLenght, buff, sizeof buff, 0, 0,
                NI_NUMERICHOST);
    const std::string save(buff);
    return save;
}

/*
    checks if the server socket able to be read from
    if so a connection is accepted and a new client is
    added to the _clients vector
*/
void Server::acceptConnection(int serverIndex)
{
    if (FD_ISSET(_serverSockets[serverIndex], &_readyToReadFrom))
    {
        Client _newClient;
        _newClient.addressLenght = sizeof _newClient.clientAddress;
        _newClient.socket = accept(_serverSockets[serverIndex],
                                   (struct sockaddr *)&_newClient.clientAddress,
                                   &_newClient.addressLenght);
        if (_newClient.socket == -1)
            error_fatal("accept()");
        _clients.push_back(_newClient);
    }
}

/*
    returns true if the passed string is a hexadecimal number
    representing the chunk line
*/
bool isChunkLine(const std::string &s)
{
    if (s.empty())
        return false;
    if (s.back() != '\r')
    {
        return false;
    }
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (!std::isxdigit(s[i]) && (s[i] != '\r' && s[i] != '\n'))
        {
            return false;
        }
    }
    return true;
}

/*
    in case the request header contains transfer-encoding: chunked
    the chuned line that contains the chunk size are removed from the
    body of the response
*/
void requestParse::converChunkedRequest(void)
{
    if (this->data["transfer-encoding"] != "Chunked")
        return;

    std::istringstream f(this->body.content);
    std::string new_body;
    std::string line;

    while (std::getline(f, line))
    {
        if (line == "0")
            break;

        if (isChunkLine(line))
        {
            size_t chunk_size;
            std::istringstream iss(line);
            iss >> std::hex >> chunk_size;
            std::string chunk_data(chunk_size, '\0');
            f.read(&chunk_data[0], chunk_size);
            new_body += chunk_data;
            f.ignore(2);
        }
        else
        {
            new_body += line;
            f.ignore(2);
        }
    }

    this->body.content = new_body;
    this->data["content-length"] = std::to_string(new_body.size());
}

/*
    returns a requestParse object which contains the
    request of a given client parsed after reading it
*/
requestParse Server::getRequest(const Client &_client)
{
    // fcntl(_client.socket, F_SETFL, O_NONBLOCK);
    int bytesRead;
    size_t bytesLeft;
    char buff[MAX_REQUEST_SIZE];
    std::string header;
    size_t totalSize = 0;
    while ((bytesRead = recv(_client.socket, buff, MAX_REQUEST_SIZE, 0)) > 0)
    {
        header.append(std::string(buff, bytesRead));
        totalSize += bytesRead;
    }
    requestParse request;
    request.setUp(header);
    bytesLeft = atoi(request.data["content-length"].c_str());
    int BUFFER_SIZE = 512;
    float factor = 0;
    if (bytesLeft == 0 && request.data["transfer-encoding"] != "Chunked")
    {
        return request;
    }
    else if ((bytesLeft * 0.000001) <= 100)
        factor += 0.25;
    else if ((bytesLeft * 0.000001) <= 300)
        factor += 0.50;
    else
        factor += 1;
    BUFFER_SIZE *= factor;
    char c[BUFFER_SIZE];
    int i = 0;
recvAgain:
    while (true)
    {
        if ((bytesRead = recv(_client.socket, c, BUFFER_SIZE, 0)) < 1)
        {
            close(_client.socket);
            break;
        }
        if (bytesRead == 0)
            return request;
        request.body.content += std::string(c, bytesRead);
        i += bytesRead;
    }
    if (request.data["transfer-encoding"] == "Chunked")
    {
        if (request.body.content.find("\r\n0\r\n") == std::string::npos)
            goto recvAgain;
    }
    else if (request.body.content.size() < bytesLeft)
        goto recvAgain;
    request.converChunkedRequest();
    return request;
}

Client::Client() : received(0), remaining(0) {}

void Server::readRequestBody(Client &_client)
{
    _client.socketSuccess = true;
    size_t bytesLeft = atoi(_client.request.data["content-length"].c_str());
    int BUFFER_SIZE = 512;
    float factor = 0;
    if (bytesLeft == 0 && _client.request.data["transfer-encoding"] != "Chunked")
        return;
    if ((bytesLeft * 0.000001) <= 100)
        factor += 0.25;
    else if ((bytesLeft * 0.000001) <= 300)
        factor += 0.50;
    else
        factor += 1;
    BUFFER_SIZE *= factor;
    char c[BUFFER_SIZE];
    int i = 0;
recvAgain:
    while (true)
    {
        int bytesRead;
        if ((bytesRead = recv(_client.socket, c, BUFFER_SIZE, 0)) < 1)
        {
            if (bytesRead == 0)
            {
                _client.socketSuccess = false;
                return ;
            }
            
            break;
        }
        if (bytesRead == 0)
            break;
        _client.request.body.content += std::string(c, bytesRead);
        i += bytesRead;
    }
    if (_client.request.data["transfer-encoding"] == "Chunked")
    {
        if (_client.request.body.content.find("\r\n0\r\n") == std::string::npos)
            goto recvAgain;
    }
    else if (_client.request.body.content.size() < bytesLeft)
        goto recvAgain;
    _client.socketSuccess = bytesLeft >= _client.request.body.content.size();
    _client.request.converChunkedRequest();
}

void Server::readHeader(Client &_client)
{
    char buff[MAX_REQUEST_SIZE];
    int ret = recv(_client.socket, buff + _client.received,
                   MAX_REQUEST_SIZE - _client.received, 0);
    if (ret < 1)
        _client.socketSuccess = false;
    else
    {
        _client.requestString.append(buff, ret);
        _client.socketSuccess = true;
    }
}

void Server::postWithoutCGI(Client &_client)
{
    size_t contentLength = atoi(_client.request.data["content-length"].c_str());
    while (contentLength > 0)
    {
        if (!_client.response.uploded)
            break;
        char buff[MAX_CHUNK_SIZE];
        int ret;
        if (FD_ISSET(_client.socket, &_readyToReadFrom))
        {
            ret = recv(_client.socket, buff, MAX_CHUNK_SIZE, 0);
            if (ret > 0)
                _client.request.body.content.append(buff, ret);
            else
                _client.socketSuccess = false;
        }
        ret = write(_client.response._fd, &_client.request.body.content[0] + _client.received,
                    _client.request.body.content.size() - _client.received);
        if (ret < 1)
        {
            _client.socketSuccess = false;
        }
        else
        {
            contentLength -= ret;
            _client.received += ret;
        }
    }
    int ret = send(_client.socket, &_client.response._response[0], _client.response._response.size(), 0);
    _client.socketSuccess = ret > 0 && _client.socketSuccess == true;
    if (!_client.socketSuccess)
    {
        close(_client.socket);
        close(_client.response._fd);
        _client.response.fdIsOpened = false;
    }
}

/*
    sends the response to all the clients that have requests
    queued and able to write to their sockets the connection
    is closed after the client have been served
*/
void Server::readRestOfBody(Client &_client)
{
    _client.socketSuccess = true;
    if (FD_ISSET(_client.socket, &_readyToWriteTo))
    {
        int ret = send(_client.socket, _client.response._response.c_str(),
                       _client.response._response.size(), 0);
        _client.socketSuccess = ret > 0;
    }
}

void readFile(Client &_client, std::string &buff)
{
    int ret = read(_client.response._fd, &buff[0] + _client.received,
                   _client.response._contentLength - _client.received);
    _client.socketSuccess = ret > 0;
    if (_client.socketSuccess)
        _client.received += ret;
    else
    {
        close(_client.socket);
        close(_client.response._fd);
        _client.response.fdIsOpened = false;
    }
}

void Server::serveContent()
{
    signal(SIGPIPE, SIG_IGN);
    std::vector<Client>::iterator it = _clients.begin();
    while (it != _clients.end())
    {
        fcntl(it->socket, F_SETFL, O_NONBLOCK);
        int i = 0;
        it->socketSuccess = true;
        if (FD_ISSET(it->socket, &_readyToReadFrom))
        {
            readHeader(*it);
            if (!it->socketSuccess)
            {
                close(it->socket);
                it = _clients.erase(it);
                continue;
            }
            it->request.setUp(it->requestString);
            if (it->request.data["method"] == "POST")
            {
                readRequestBody(*it);
                if (!it->socketSuccess)
                {
                    close(it->socket);
                    close(it->response._fd);
                    it->response.fdIsOpened = false;
                    it = _clients.erase(it);
                    continue;
                }
            }
            it->response.setUp(_configFile, it->request);
            if (it->request.data["method"] == "POST")
            {
                postWithoutCGI(*it);
                if (!it->socketSuccess)
                    it = _clients.erase(it);
            }
            else
            {
                readRestOfBody(*it);
                if (!it->socketSuccess)
                {
                    close(it->socket);
                    it = _clients.erase(it);
                    continue;
                }
                if (it->response.fdIsOpened)
                {
                    std::string buff(it->response._contentLength, '\0');
                    readFile(*it, buff);
                    if (!it->socketSuccess) // close and break;
                    {
                        it = _clients.erase(it);
                        break;
                    }
                    while (it->received > 0)
                    {
                        int chunk = std::min((int)it->received, MAX_REQUEST_SIZE);
                        char _buff[chunk];
                        for (int j = 0; j < chunk; ++j)
                            _buff[j] = buff[i++];
                        if (FD_ISSET(it->socket, &_readyToWriteTo))
                        {
                            int ret = send(it->socket, _buff, chunk, 0);

                            if (ret < 1) // close and break;
                            {
                                close(it->response._fd);
                                it->response.fdIsOpened = false;
                                close(it->socket);
                                it = _clients.erase(it);
                                break;
                            }
                            it->remaining += ret;
                            it->received -= ret;
                        }
                    } // end sending loop
                }
            }

            if (it < _clients.end())
            {
                if (it->response.fdIsOpened)
                {
                    close(it->response._fd);
                    it->response.fdIsOpened = false;
                }
                close(it->socket);
                it = _clients.erase(it);
            }
        }
        else
            it++;
    }
}

/*
    a constructor which calls all the helper methods
    and starts the server main loop
*/
Server::Server(std::string file) : _configFile(file)
{
    srand(time(NULL));
    std::vector<std::string> usedPorts;

    std::vector<std::string> _ports;
    for (size_t i = 0; i < _configFile.servers.size(); ++i)
    {
        if (_configFile.servers[i].data["listen"].size() >= 1)
        {
            const char *host = _configFile.servers[i].data["host"].size() != 1
                                   ? NULL
                                   : _configFile.servers[i].data["host"].front().c_str();
            if (std::find(usedPorts.begin(), usedPorts.end(), _configFile.servers[i].data["listen"].front()) == usedPorts.end())
            {

                initServerSocket(host,
                                 _configFile.servers[i].data["listen"].front().c_str());
                usedPorts.push_back(_configFile.servers[i].data["listen"].front());
                if (host == NULL)
                    host = "localhost";
                std::cout << "Server: " << i << " is listening on http://"
                          << host << ":"
                          << _configFile.servers[i].data["listen"].front().c_str() << std::endl;
            }
        }
        else
            error("server block must contain listen directive");
    }
    while (1)
    {
        for (size_t i = 0; i < _serverSockets.size(); i++)
        {
            waitForClients();
            acceptConnection(i);
            serveContent();
        }
    }
    for (size_t i = 0; i < _serverSockets.size(); i++)
        close(_serverSockets[i]);
}

void sighander(int sig)
{
    if (sig == SIGINT)
    {
        std::cout << "\n== Webserver Quit Succeffuly ==\n";
        exit(0);
    }
}
int main(int ac, char **av)
{
    signal(SIGINT, sighander);
    if (ac == 2)
        Server server(av[1]);
}

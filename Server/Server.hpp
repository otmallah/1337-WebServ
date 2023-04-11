#if !defined(SERVER_HPP)
#define SERVER_HPP

#include "../inc.hpp"
#include "../Parse/Config.hpp"
#include "../Parse/requestParse.hpp"
#include "../Response/Response.hpp"

#define MAX_CHUNK_SIZE 4096

/*
    a struct which identifies each connected
    client
*/
struct Client
{
    socklen_t addressLenght;
    struct sockaddr_storage clientAddress;
    int socket;
    size_t received;
    int remaining;
    std::string requestString;
    char buff[MAX_CHUNK_SIZE];
    requestParse request;
    Response response;
    const std::string getClientAddress();
    Client();

};

/*
    class which represents a server ready to accept connections
    takes a config file name in it's constructor
*/
class Server
{
private:
    size_t nbServers;
    // the server socket
    std::vector<int>             _serverSockets;
    // vector of clients
    std::vector<Client>         _clients; 
    // the sockets/fds that are ready to be read from
    fd_set                      _readyToReadFrom; 
    // the sockets/fds that are ready to write to
    fd_set                      _readyToWriteTo; 
    // The config file was parsed and stored in this object to make it easier to work with.
    Config                      _configFile; 
    requestParse    getRequest(const Client &_client);
    std::string     getRequestBuffer(Client &);
    void            checkCientCases();
    void            waitForClients();
    void            acceptConnection(int serverIndex);
    void            serveContent();
    void            initServerSocket(const char *port);
public:
    Server(std::string file);
    ~Server();
};

#endif // SERVER_HPP

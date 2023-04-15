#if !defined(ACONFIG_HPP)
#define ACONFIG_HPP


#define  ON 1
#define  OFF 0

#include "../inc.hpp"

class Config
{
private:
    std::fstream _configFile;
    std::vector<std::string> _fileBuff;

    void readFileIntoBuff(const std::string &s);

public:
    class locationParse
    {
    private:
        typedef std::pair<std::string, std::vector<std::string> > directive;

        directive                   _root;
        directive                   _index;
        directive                   _allowedMethods;
        directive                   _upload;
        directive                   _uploadPath;
        directive                   _bodySize;
        directive                   _redirect;
        int                         _start;
        std::vector<std::string>    _fileBuff;

        void setDirective(const std::string &key, std::vector<std::string> &values);
        void collectPath(const std::string &s);
        void parseBlock();
        void fillDirective(const std::string &s, const std::string &key);
        
        friend class Config;

    public:
        std::string path;
        bool autoIndex;
        std::map<std::string, std::vector<std::string> > data;
        std::map<int, std::string> errorPages;
        locationParse(const std::vector<std::string> &, int);
        ~locationParse();
    };
    class serverParse
    {
    private:
        typedef std::pair<std::string, std::vector<std::string> > directive;
        int _start;
        std::string _lastKey;
        bool _serverIsOpened;
        bool _locationIsOpened;
        bool _isInsideServer;
        directive _root;
        directive _index;
        directive _cgiPath;
        directive _allowedMethods;
        directive _upload_path;
        directive _upload;
        directive _port;
        directive _bodySize;
        directive _host;
        directive _serverName;
        std::vector<std::string> _fileBuff;
        std::vector<std::string> _locationBuff;

        void insertDirectives(void);
        void fillEmptyRequiredDirectives(void);
        void fillDirective(const std::string &, const std::vector<std::string> &);
        void handleErrors(const std::string &);
        void setFileBuff(const std::vector<std::string> &);
        void getValues(std::vector<std::string> &values, const std::string &fileBuff, int &j);
        size_t parseBlock();
        std::string getKey(const std::string &fileBuff, int &j);

        friend class Config;

    public:
        bool autoIndex;
        std::vector<locationParse> locations;
        std::map<std::string, std::vector<std::string> > data;
        std::map<int, std::string> errorPages;
        serverParse(const std::vector<std::string> &, int);
        ~serverParse();
    };
    std::vector<serverParse> servers;
    Config(std::string s);
    ~Config();
};

#endif // ACONFIG_HPP

#ifndef _SOCKET_
#define _SOCKET_
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cstring>
#include <unordered_map>
#include "threadpool.h"

using namespace std;

class Socket
{
public:
    Socket();
    ~Socket();
    bool start();
    void evenAccept();
    void eventClose();
    void httpResolve(const char* buffer);
    bool fileExists(const string& filename);
    string readFile(const string &path);
    string getContentType(const string & path);
    void worker(int socketfd);
    void select_model();
private:
    int m_socket;
    int new_socket;
    int opt = 1;
    struct sockaddr_in address;
    const int m_port = 8080;
    int m_backlog = 4; //连接队列最大长度
    Threadpool* pool;

};

#endif
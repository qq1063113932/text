#ifndef _CONNECTION_POOL_
#define _CONNECTION_POOL_

#include <mysql/mysql.h>
#include <stdio.h>
#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <json/json.h>

using namespace std;

class ConnectionPool
{
public:
    //单例模式，静态成员函数
    static ConnectionPool* GetInstance();
    //禁用拷贝构造
    ConnectionPool(const ConnectionPool& obj) = delete;
    ConnectionPool& operator=(const ConnectionPool& obj) = delete;
private:
    ConnectionPool();
    ~ConnectionPool();
    void ParseJsonFile();

    string m_ip;
    string m_user;
    string m_password;
    string m_database;
    unsigned int port;//端口
    int m_minSize;
    int m_maxSize;
    int m_timeout;
    int m_maxIdleTime;
    mutex m_mutexQ;
    queue<MYSQL*> m_connQueue; //连接池
    condition_variable m_cond;
};


#endif

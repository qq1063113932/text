#ifndef _CONNECTION_POOL_
#define _CONNECTION_POOL_

#include <mysql/mysql.h>
#include <stdio.h>
#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <json/json.h>
#include <fstream>
#include <thread>
#include "MysqlConn.h"

using namespace std;

class ConnectionPool
{
public:
    //单例模式，静态成员函数
    static ConnectionPool* GetInstance();
    //禁用拷贝构造
    ConnectionPool(const ConnectionPool& obj) = delete;
    ConnectionPool& operator=(const ConnectionPool& obj) = delete;
    //获取连接
    shared_ptr<MysqlConn> GetConnetion();

private:
    ConnectionPool();
    ~ConnectionPool();
    bool ParseJsonFile();
    void ProduceConnetion();
    void recycleConnetion();
    void AddConnetion();

    string m_ip;
    string m_user;
    string m_password;
    string m_database;
    unsigned short m_port;//端口
    int m_minSize;
    int m_maxSize;
    int m_timeout;
    int m_maxIdleTime;
    mutex m_mutexQ;
    queue<MysqlConn*> m_connQueue; //连接池
    condition_variable m_cond;
};


#endif

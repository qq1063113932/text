#ifndef _MYSQLCONN_
#define _MYSQLCONN_
#include <stdio.h>
#include <iostream>
#include <string>
#include <mysql/mysql.h>
#include <chrono>
using namespace std;

class MysqlConn
{
public:
    MysqlConn();
    ~MysqlConn();
    bool connet(string ip, string user, string password, string database, unsigned short port = 3306);
    bool update(string sql);
    bool query(string sql);
    //遍历查询得到的结果集
    bool next();
    //得到结果集中的字段值
    string value(int index);
    //事务操作
    bool transaction();
    //提交事务
    bool commit();
    //事务回滚
    bool rollback();
    //刷新起始空闲时间点    
    void refreshAliveTime();
    //计算连接时间长
    long long getAliveTime();
    
private:
    void freeResult();
    MYSQL* m_con = nullptr;
    MYSQL_RES* m_result = nullptr;
    MYSQL_ROW m_row = nullptr;
    chrono::steady_clock::time_point m_alivetime;
};

#endif
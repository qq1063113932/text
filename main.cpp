#include <stdio.h>
#include <iostream>
#include "mysql_connection_pool.h"
#include "threadpool.h"
#include "socket.h"
#include "log.h"

void query()
{
    MysqlConn con;
    con.connet("localhost", "root", "123456", "textdb");
    // string sql = "INSERT INTO user (username, passwd) VALUES ('bin', 48)";
    // con.update(sql);
    string sql = "delete from user where username = 'bin' limit 2";
    con.update(sql);
    sql = "select * from user";
    con.query(sql);
    while(con.next())
    {
        cout<<con.value(0) << ',' << con.value(1) << endl;
    }
}

int add(int a, int b)
{
    cout << "当前线程：" << this_thread::get_id() << endl;
    return a + b;
}

void threadwork()
{
    Threadpool* threadpool = new Threadpool(4);
    for(int i = 0; i < 20; ++i)
    {
        auto resfuture = threadpool->enqueue([](int a, int b) -> int{
            cout << "当前线程：" << this_thread::get_id() << endl;
            return a + b;
        }, 10*i, 10*i);
        cout << "result: " << resfuture.get() << endl;
    }
    delete threadpool;
}

void logtext()
{
    Logger logger("log.txt");
    logger.log(LogLevel::INFO, "start logger");
    int id = 42;
    string action = "login";
    double duration = 3.2;
    logger.log(LogLevel::DEBUG, "User {} performed {} in {} seconds", id , action, duration);
    logger.log(LogLevel::WARNING, "hello world", action, action, action);
    logger.log(LogLevel::ERROR, "User {} performed {} in seconds ", id , action, duration);
    logger.log(LogLevel::INFO, "User {} performed {} in {} seconds ", id , action);
    logger.log(LogLevel::INFO, "User {} performed {} in {} seconds ");
    logger.log(LogLevel::DEBUG, "User {} performed {} in {} seconds ");
}

int main()
{
    // query();
    // threadwork();
    // Socket server;
    // server.start();
    logtext();
    return 0;
}
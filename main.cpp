#include <stdio.h>
#include <iostream>
#include "mysql_connection_pool.h"
#include "threadpool.h"
#include "socket.h"

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
    Threadpool* threadpool = Threadpool::GetInstance(4);
    for(int i = 0; i < 20; ++i)
    {
        auto resfuture = threadpool->enqueue([](int a, int b) -> int{
            cout << "当前线程：" << this_thread::get_id() << endl;
            return a + b;
        }, 10*i, 10*i);
        cout << "result: " << resfuture.get() << endl;
    }
}

int main()
{
    // query();
    // threadwork();
    Socket server;
    server.start();
    return 0;
}
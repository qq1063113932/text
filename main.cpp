#include <stdio.h>
#include <iostream>
#include "mysql_connection_pool.h"

using namespace std;

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

int main()
{
    query();
    return 0;
}
#include "mysql_connection_pool.h"

ConnectionPool* ConnectionPool::GetInstance()
{
    static ConnectionPool pool;//静态局部变量
    return &pool;
}

void ConnectionPool::ParseJsonFile()
{
    
}
#include "mysql_connection_pool.h"

ConnectionPool* ConnectionPool::GetInstance()
{
    static ConnectionPool pool;//静态局部变量
    return &pool;
}

bool ConnectionPool::ParseJsonFile()
{
    ifstream ifs("dbcofig.json");
    Json::Reader rd;
    Json::Value root;
    rd.parse(ifs, root);
    if(root.isObject())
    {
        m_ip = root["ip"].asCString();
        m_user = root["username"].asCString();
        m_password = root["password"].asCString();
        m_database = root["database"].asCString();
        m_port = root["port"].asInt();
        m_minSize = root["minSize"].asInt();
        m_maxSize = root["maxSize"].asInt();
        m_maxIdleTime = root["maxIdleTime"].asInt();
        m_timeout = root["timeout"].asInt();
        return true;
    }
    return false;
}

ConnectionPool::ConnectionPool()
{
    //加载配置文件
    if(!ParseJsonFile())
    {
        return;
    }

    //初始化数据库连接
    for(int i = 0; i < m_minSize; ++i)
    {
        AddConnetion();  
    }
    thread producer(&ConnectionPool::ProduceConnetion, this);
    thread recycler(&ConnectionPool::recycleConnetion, this);
    producer.detach();
    recycler.detach();
}

ConnectionPool::~ConnectionPool()
{
    while(!m_connQueue.empty())
    {
        MysqlConn* con = m_connQueue.front();
        m_connQueue.pop();
        delete con;
    }
}

void ConnectionPool::AddConnetion()
{
    MysqlConn* con = nullptr;
    con->connet(m_ip, m_user, m_password, m_database, m_port);
    con->refreshAliveTime();
    m_connQueue.push(con);
}

void ConnectionPool::ProduceConnetion()
{
    while(true)
    {
        unique_lock<mutex> locker(m_mutexQ);
        while(m_connQueue.size() >= m_minSize)
        {
            m_cond.wait(locker);
        }
        AddConnetion();
        m_cond.notify_all();
    }
}

void ConnectionPool::recycleConnetion()
{
    while(true)
    {
        this_thread::sleep_for(chrono::seconds(1));
        lock_guard<mutex> locker(m_mutexQ);
        while(m_connQueue.size() > m_minSize)
        {
            MysqlConn* con = m_connQueue.front();
            if(con->getAliveTime() > m_maxIdleTime)
            {
                m_connQueue.pop();
                delete con;
            }
            else
            {
                break;
            }
        }
    }
}

shared_ptr<MysqlConn> ConnectionPool::GetConnetion()
{
    unique_lock<mutex> locker(m_mutexQ);
    while(m_connQueue.empty())
    {
        if(cv_status::timeout == m_cond.wait_for(locker, chrono::milliseconds(m_timeout)))
        {
            if(m_connQueue.empty())
            {
                // return nullptr;
                continue;
            }
        }
    }
    shared_ptr<MysqlConn> conptr(m_connQueue.front(), [this](MysqlConn* con){
        lock_guard<mutex> locker(m_mutexQ);
        con->refreshAliveTime();
        m_connQueue.push(con);
    });
    m_connQueue.pop();
    m_cond.notify_all();
    return conptr;
}
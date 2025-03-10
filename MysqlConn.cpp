#include "MysqlConn.h"

MysqlConn::MysqlConn()
{
    m_con = mysql_init(nullptr);
    mysql_set_character_set(m_con, "utf8");

}

MysqlConn::~MysqlConn()
{
    if(m_con != nullptr)
    {
        mysql_close(m_con);
    }
    freeResult();
}

bool MysqlConn::connet(string ip, string user, string password, string database, unsigned short port)
{
    m_con = mysql_real_connect(m_con, ip.c_str(), user.c_str(), password.c_str(), database.c_str(), port, nullptr, 0);
    if(m_con == nullptr)
    {
        cout << "mysql error" << endl;
        return false;
    }
    return true;
}

bool MysqlConn::update(string sql)
{
    if( mysql_query(m_con, sql.c_str()) )
    {
        cout << "mysql update error" << endl;
        return false;
    }
    return true;
}

bool MysqlConn::query(string sql)
{
    freeResult();
    if( mysql_query(m_con, sql.c_str()) )
    {
        cout << "mysql update error" << endl;
        return false;
    }
    m_result = mysql_store_result(m_con);
    return true;
}

bool MysqlConn::next()
{
    if(m_result != nullptr)
    {
        m_row = mysql_fetch_row(m_result);
        if(m_row != nullptr)
        {
            return true;
        }
    }
    return false;
}

string MysqlConn::value(int index)
{
    int rowCount = mysql_num_fields(m_result);
    if(index >= rowCount || index < 0)
    {
        cout << "mysql index invalid" << endl;
        return string();
    }
    char* value = m_row[index];
    //value是一个二进制数据中间位置有\0，只能把前面的数据转换成字符串类型
    unsigned long length = mysql_fetch_lengths(m_result)[index];
    return string(value, length);
}

bool MysqlConn::transaction()
{
    return mysql_autocommit(m_con, false); //false 手动提交 
}

bool MysqlConn::commit()
{
    return mysql_commit(m_con);
}

bool MysqlConn::rollback()
{
    return mysql_rollback(m_con);
}

void MysqlConn::freeResult()
{
    if(m_result)
    {
        mysql_free_result(m_result);
        m_result = nullptr;
    }
}

void MysqlConn::refreshAliveTime()
{
    m_alivetime = chrono::steady_clock::now();
}

long long MysqlConn::getAliveTime()
{
    chrono::nanoseconds res = chrono::steady_clock::now() - m_alivetime;
    chrono::milliseconds millisec = chrono::duration_cast<chrono::milliseconds>(res);
    return millisec.count();
}
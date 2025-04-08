#ifndef _LOG_
#define _LOG_
#include<iostream>
#include<fstream>
#include<sstream>
#include<vector>
#include<queue>
#include<string>
#include<atomic>
#include<stdexcept>
#include<ctime>
#include"threadpool.h"

using namespace std;

//将单个参数转化为字符串
template<typename T>
string toString(T&& arg)
{
    ostringstream oss;
    oss << forward<T>(arg);
    return oss.str();
}

enum LogLevel
{
    INFO, DEBUG, WARNING, ERROR
};

class LogQueue
{
public:
    LogQueue() : isStop(false) {};
    void push(const string& msg);
    bool pop(string& msg);
    void stop();

private:
    queue<string> m_queue;
    mutex m_mutex;
    condition_variable cv;
    atomic<bool> isStop;

};

class Logger
{
public:
    Logger(const string& filename);
    ~Logger();
    template<typename... Args>
    void log(LogLevel level, const string&format, Args&&... args)
    {
        string strlevel;
        switch (level)
        {
            case LogLevel::INFO:
                strlevel = "[INFO] ";
                break;

            case LogLevel::DEBUG:
                strlevel = "[DEBUG] ";
                break;

            case LogLevel::WARNING:
                strlevel = "[WARNING] ";
                break;

            case LogLevel::ERROR:
                strlevel = "[ERROR] ";
                break;
        
            default:
                break;
        }
        time_t now = time(nullptr); // 获取当前时间
        tm* ltm = localtime(&now); // 转换为本地时间
        // 格式化时间
        char buffer[80];
        strftime(buffer, sizeof(buffer), " [%Y-%m-%d %H:%M:%S] ", ltm);
        logqueue.push(strlevel + buffer + formatMessage(format, forward<Args>(args)...));
    }
private:
    template<typename... Args>
    string formatMessage(const string& format, Args&&... args)
    {
        vector<string> vecString = { toString(forward<Args>(args))... };
        ostringstream oss;
        int argsIndex = 0;
        int pos = 0;
        int insert = format.find("{}", pos);
        while(insert != string::npos)
        {
            if(argsIndex < vecString.size())
            {
                oss << format.substr(pos, insert - pos);
                oss << vecString[argsIndex++];
                pos = insert + 2;
                insert = format.find("{}", pos);
            }
            else
            {
                break;
            } 
        }
        oss << format.substr(pos);
        while(argsIndex < vecString.size())
        {
            oss << vecString[argsIndex++];
        }
        return oss.str();
    }

    LogQueue logqueue;
    thread worker;
    ofstream logfile;
    atomic<bool> isExit;
};

#endif
#include"log.h"

void LogQueue::push(const string& msg)
{
    {
        lock_guard<mutex> locker(m_mutex);
        m_queue.push(msg);
    }
    cv.notify_one();
}

bool LogQueue::pop(string& msg)
{
    unique_lock<mutex> locker(m_mutex);
    cv.wait(locker, [this] {return isStop || !m_queue.empty();} );
    if(isStop && m_queue.empty())
    {
        return false;
    }
    // while(!m_queue.empty())
    // {
    //     msg = m_queue.front();
    //     m_queue.pop();
    // }
    cout << " 111" << endl; 
    msg = m_queue.front();
    m_queue.pop();
    return true;
}

void LogQueue::stop()
{
    isStop = true;
    cv.notify_all();
}

// std::ios::out：以写模式打开文件，清空现有内容。
// std::ios::app：以追加模式打开文件，保留现有内容。
// std::ios::out | std::ios::app：以写模式打开文件，并将新内容追加到文件末尾。
Logger::Logger(const string& filename) : logfile(filename, ios::out | ios::app), isExit(false)
{
    if(!logfile.is_open())
    {
        throw runtime_error("文件打开失败");
    }
    worker = thread([this] {
        string msg;
        while(logqueue.pop(msg))
        {
            logfile << msg << endl;
        }
    });
}

Logger::~Logger()
{
    isExit = true;
    logqueue.stop();
    if(worker.joinable())
    {
        worker.join();
    }

    if(logfile.is_open())
    {
        logfile.close();
    }
}



#ifndef _THREADPOOL_
#define _THREADPOOL_
#include <functional>
#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <future>
#include <memory>

using namespace std;

class Threadpool
{
public:
    static Threadpool* GetInstance(int ThreadNum)
    {
        static Threadpool pool(ThreadNum);
        return &pool;
    }

    //任务队列
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> future< typename result_of<F(Args...)>::type >
    {
        using functype = typename result_of<F(Args...)>::type; //获取返回值
    
        //1. make_shared 比 new + share_ptr更安全 ; make_shared<指针指向类型>(构造函数参数)
        //2. bind将可调用对象f和参数args...绑定在一起，创建一个新的可调用对象
        //3. bind创建的可调用对象作为 packaged_task 的参数构造 packaged_task 对象，参数类型是 functype()
        //4. functype()表示可调用对象可以作为参数构造 packaged_task 对象，functype是返回类型（例如int）不能作为参数构造 packaged_task 对象
        //5. forward用于完美转发。
        auto task = make_shared< packaged_task<functype()> >( bind(forward<F>(f), forward<Args>(args)...) );
    
        future<functype> resfuture = task->get_future();
    
        //将任务添加至队列
        {
            unique_lock<mutex> locker(mtx);
            if(isStop)
            {
                throw runtime_error("线程池已经停止工作");
            }
            taskQueue.push([task](){
                (*task)(); //task是智能指针，指向的是 packaged_task 封装F(Args...) 的可调用对象。
            });
        }
    
        //通知线程取执行任务
        cv.notify_one();
    
        //返回future
        return resfuture;
    }

private:
    Threadpool(int ThreadNum)
    {
        for(int i = 0; i < ThreadNum; ++i)
        {
            workers.emplace_back([this](){
                this->worker();
            });
        }
    }

    ~Threadpool()
    {
        {
            unique_lock<mutex> locker(mtx);
            isStop = true;
        }
    
        //通知所有阻塞线程
        cv.notify_all();
    
        //确保线程执行完成
        // for(thread& onethread : workers)
        // {
        //     onethread.join();
        // }
        for(int i = 0; i < workers.size(); ++i)
        {
            workers[i].join();
        }
    }

    void worker()
    {
        while(true)
        {
            //定义任务
            function<void()> task;
    
            //从队列中获取任务
            {
                unique_lock<mutex> locker(mtx);
                cv.wait(locker, [this] {return isStop || !taskQueue.empty();} ); //工作停止 或 队列不为空往下执行
                if(isStop && taskQueue.empty()) // 如果工作停止且队列为空 退出
                {
                    return;
                }
                task = move(taskQueue.front());
                taskQueue.pop();
            }
            //执行任务
            task();
        }
    }

    queue<function<void()>> taskQueue; //这个function<void()>是lambda表达式
    vector<thread> workers;
    mutex mtx;
    condition_variable cv;
    bool isStop;

};

#endif
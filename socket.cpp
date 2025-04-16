#include "socket.h"

Socket::Socket()
{    
    pool = new Threadpool(4);
}

Socket::~Socket()
{
    delete pool;
}

bool Socket::start()
{
    //建立Socket
    m_socket = socket(AF_INET, SOCK_STREAM, 0); //TCP
    if(m_socket < 0)
    {
        cout<< "socket create error" << endl;
        return false;        
    }
    
    //设置Socket 选项
    setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    //设置服务器地址信息
    address.sin_family = AF_INET;           //ipv4
    address.sin_addr.s_addr = INADDR_ANY;   //允许接收的ip地址
    address.sin_port = htons(m_port);       //设置监听的端口号

    //绑定到指定地址和端口
    if(bind(m_socket, (struct sockaddr*)&address, sizeof(address)) < 0)
    {
        cout<< "socket bind error" << endl;
        return false;
    }

    //监听连接请求
    if(listen(m_socket, m_backlog) < 0)
    {
        cout << "socket listen error" << endl;
        return false;
    }
    cout << "socket is listening on port " << m_port << "..." << endl;
    // evenAccept();
    // select_model();
    epoll_model();
    close(m_socket);
    return true;
}

void Socket::worker(int socketfd)
{
    while (true)
    {
        //接收客户端发送的消息
        char buffer[4096] = {0}; //接收缓冲区
        ssize_t bytes_read = recv(socketfd, buffer, sizeof(buffer), 0);
        if(bytes_read < 0)
        {
            cout << "socket accept error" << endl;
            break;
        }
        else if(bytes_read == 0)
        {
            cout << "socket disconnect" << endl;
            break;
        }
        else
        {
            cout << "clinent: \n" << buffer << endl;
            string s(buffer, 3);
            if(s == "GET")
            {
                httpResolve(buffer);    
            }
            else
            {
                const char* message = "hello from server";
                send(socketfd, message, strlen(message), 0); //发送数据
            }
            
        }       
    }
    // 关闭客户端连接
    close(socketfd);
}

//无限循环，接收客户端的连接
void Socket::evenAccept()
{
    while(true)
    {
        socklen_t addrlen = sizeof(address);
        new_socket = accept(m_socket, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if(new_socket < 0)
        {
            cout << "socket accept error" << endl;
        }
        pool->enqueue(bind(&Socket::worker, this, new_socket)); // worker是成员函数有隐藏的this指针，使用bind或者lambda表达式
        // worker(new_socket);
    }
}

void Socket::eventClose()
{
    close(m_socket);
}

bool Socket::fileExists(const string& filename) {
    ifstream file(filename);
    return file.is_open();  // 文件存在返回 true，否则 false
}

string Socket::readFile(const string &path)
{
    //创建输入文件流对象,以二进制模式打开文件
    ifstream file(path, ios::binary);
    if(!file.is_open())
    {
        return "";
    }

    ostringstream ss;

    ss << file.rdbuf();

    return ss.str();
}

string Socket::getContentType(const string &path)
{
    static std::unordered_map<std::string, std::string> mimeTypes = {
        {".html", "text/html"},
        {".htm", "text/html"},
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".json", "application/json"},
        {".png", "image/png"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".gif", "image/gif"},
        {".svg", "image/svg+xml"},
        {".mp4", "video/mp4"},
        {".mp3", "audio/mpeg"},
        {".pdf", "application/pdf"},
        {".txt", "text/plain"}
    };
    int dotpos = path.find_last_of(".");
    if(dotpos != string::npos)
    {
        string ext = path.substr(dotpos);
        if(mimeTypes.count(ext))
        {
            return mimeTypes[ext];
        } 
    }
    return "text/plain";
}

void Socket::httpResolve(const char* buffer)
{
    istringstream request(buffer);
    string method, path, version;
    request >> method >> path >> version;

    //默认首页
    if(path == "/") path = "/restore/index.html";

    //构建文件路径
    string filepath = "." + path;

    //处理GET请求
    if(method == "GET")
    {
        if(fileExists(filepath))
        {
            cout << "http" << endl;
            string content = readFile(filepath);
            string contentType = getContentType(filepath);
            ostringstream response;
            response << "HTTP/1.1 200 OK\r\n";
            response << "Content-Type: " << contentType << "\r\n";
            response << "Content-Length: " << content.length() << "\r\n";
            response << "\r\n";
            response << content;

            //发送响应
            send(new_socket, response.str().c_str(), response.str().length(), 0);
        }
        else
        {
            //404 Not Found
            string notFound = "HTTP/1.1 404 Not Found\r\n\r\n404 Not Found";
            send(new_socket, notFound.c_str(), notFound.length(), 0);
        }
    }
    else if(method == "POST")
    {

    }
    
}

void Socket::select_model()
{
    fd_set readfds, tmpfds;
    FD_ZERO(&readfds);
    FD_SET(m_socket, &readfds);
    tmpfds = readfds;
    int maxfd = m_socket;
    while (true)
    {
        tmpfds = readfds;
        int ret = select(maxfd + 1, &tmpfds, nullptr, nullptr, nullptr); //返回有读事件的文件描述符
        if(ret > 0)
        {  
            if(FD_ISSET(m_socket, &tmpfds)) //处理服务器监听fd的事件
            {
                new_socket = accept(m_socket, nullptr, nullptr);
                if(new_socket > 0)
                {
                    FD_SET(new_socket, &readfds);
                    maxfd = maxfd > new_socket ? maxfd : new_socket;
                }   
            }
            for(int client_socket = 0; client_socket <= maxfd; client_socket++) //处理客户端发送消息的fd
            {
                if(client_socket == m_socket) continue;
                if(FD_ISSET(client_socket, &tmpfds))
                {
                    //接收客户端发送的消息
                    char buffer[4096] = {0}; //接收缓冲区
                    ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);
                    if(bytes_read == 0)//客户端断开连接
                    {
                        cout << "socket disconnect" << endl;
                        close(client_socket);
                        FD_CLR(client_socket, &readfds);
                        continue;
                    }
                    else if(bytes_read < 0)
                    {
                        perror("recv error");
                        continue;
                    }   
                    else
                    {
                        cout << "clinent: \n" << buffer << endl;
                        string s(buffer, 3);
                        if(s == "GET")
                        {
                            httpResolve(buffer);    
                        }
                        else
                        {
                            const char* message = "hello from server";
                            send(client_socket, message, strlen(message), 0); //发送数据
                        }
                        
                    }
                }
            }

        }
        else if(ret == 0)
        {
            cout << "select timeout" << endl;
            break;
        }
        else
        {
            cout << "select error " << errno << endl;
            break;
        }
    } 
}

void Socket::epoll_model()
{
    //创建epoll实例，size参数已经弃用填一个大于0的数即可
    int epfd = epoll_create(1);
    if(epfd == -1)
    {
        perror("epoll_create");
        return;
    }

    //存储事件信息
    epoll_event ev;
    ev.events = EPOLLIN; //读事件
    ev.data.fd = m_socket;

    //将要监听的fd加入epoll实例
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, m_socket, &ev) == -1)
    {
        perror("epoll_ctl: add listen_fd");
        return;
    }

    epoll_event events[1024];
    int size = sizeof(events) / sizeof(events[0]);
    while(true)
    {
        int num = epoll_wait(epfd, events, size, -1);
        for(int i = 0; i < num; ++i)
        {
            int fd = events[i].data.fd;
            new_socket = fd;
            if(fd == m_socket)
            {
                int client_socket = accept(fd, nullptr, nullptr);
                //将客户端连接的fd加入epoll实例
                epoll_event ev;
                ev.events = EPOLLIN;
                ev.data.fd = client_socket;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_socket, &ev);
            }
            else
            {
                //接收客户端发送的消息
                char buffer[1024] = {0}; //接收缓冲区
                ssize_t bytes_read = recv(fd, buffer, sizeof(buffer), 0);
                if(bytes_read == 0)//客户端断开连接
                {
                    cout << "socket disconnect" << endl;
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev);                 
                    close(fd); //先删除再关闭
                    continue;
                }
                else if(bytes_read < 0)
                {
                    perror("recv error");
                    close(fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
                    continue;
                }
                else
                {
                    cout << "clinent: \n" << buffer << endl;
                    string s(buffer, 3);
                    if(s == "GET")
                    {
                        httpResolve(buffer);    
                    }
                    else
                    {
                        // const char* message = "hello from server";
                        const char* message = buffer;
                        send(fd, message, strlen(message), 0); //发送数据
                    }
                }
            }
        }

    }
}
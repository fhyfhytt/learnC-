//
// Created by Administrator on 2021/4/9 0009.
//

#ifndef LEARNC___SERVERSOCKET_H
#define LEARNC___SERVERSOCKET_H
#ifndef UNICODE
#define UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#ifndef WIN32
#include <sys/socket.h>
    #include <netinet/in.h>
#else
//win7下的头文件　　　　
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
#include <windows.h>
#include <mswsock.h>
#include <memory>
#include <deque>
#include <vector>
#include <functional>
#include <mutex>
class ServerSocket;
struct OverUnit:WSAOVERLAPPED{
    ServerSocket* socket;
    int type;
};
class ServerSocket: public std::enable_shared_from_this<ServerSocket> {
public:
    //指向自己的智能指针
    using pointer = std::shared_ptr<ServerSocket>;
    typedef std::function<void (const pointer&,const char* data,int size)> HandleRecvFunction;
    typedef std::function<void (const pointer&)> HandleClose;
    HandleRecvFunction handleRecv;
    HandleClose handleClose;
    typedef std::function<void(const pointer&,int ,int)> HandleError;
    HandleError handleError;
    ServerSocket(int id,SOCKET s);
    ~ServerSocket();
    void onError(int errorType,int errorCode);
    void onClose();
    void onFinishedSend(int transfered);
    void onFinishedRecv(int transfered);
    int id() const {return m_id;}
    void startRecv();
    void sendMessage(const std::string& info);
    void trySendMore();

private:
    int m_id;
    SOCKET m_socket;
   using Buffer=std::vector<char>;
   Buffer m_currentRecv;
   std::deque<Buffer> m_recvBuffers;
   std::mutex m_mutex;
   std::deque<std::string> m_sendBuffers;
   OverUnit m_recvUnit;
   OverUnit m_sendUnit;
   void sendFrontBuffer();
   WSABUF m_sendWSA;
   WSABUF m_recvWSA;

};
#endif //LEARNC___SERVERSOCKET_H

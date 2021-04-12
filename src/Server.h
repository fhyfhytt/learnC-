//
// Created by Administrator on 2021/4/9 0009.
//
#ifndef LEARNC___SERVER_H
#define LEARNC___SERVER_H
#ifndef UNICODE
#define UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mswsock.h>
#include <cstdio>
#include <winsock2.h>
#include <iostream>
#include <functional>
#include "ServerSocket.h"
class Server {
public:
    explicit Server(u_short port);
    ~Server();
    bool startAccept();
    void waitingForAccept();
    void waitingForIo();
    bool isRunning()const{return m_running;};
    void stop(){m_running=false;};
    typedef std::function<void(ServerSocket::pointer)> HandleNewConnect;
    HandleNewConnect newConn;
    ServerSocket::HandleRecvFunction socketRecv;
    ServerSocket::HandleClose socketClose;
    ServerSocket::HandleError socketError;
private:
    u_short m_port;
    SOCKET m_listenSocket;
    HANDLE m_completePort;
    LPFN_ACCEPTEX lpfnAcceptEx;
    SOCKET m_currentAcceptSocket;
    WSAOVERLAPPED m_acceptUnit;
    HANDLE m_ioCompletePort;
    bool m_running;
    std::vector<char> m_acceptBuffer;
    bool tryNewConn();
};


#endif //LEARNC___SERVER_H

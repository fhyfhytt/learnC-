//
// Created by Administrator on 2021/4/9 0009.
//
#include "Server.h"
#include <string>
#include <cassert>
Server::Server(u_short p):m_port(p),m_listenSocket(INVALID_SOCKET),m_completePort(NULL),lpfnAcceptEx(nullptr),m_currentAcceptSocket(INVALID_SOCKET),m_ioCompletePort(NULL),
m_running(false),m_acceptBuffer(1024) {}
Server::~Server() {
    if(m_listenSocket!=INVALID_SOCKET) closesocket(m_listenSocket);
        CloseHandle(m_ioCompletePort);
        CloseHandle(m_completePort);
}
bool Server::startAccept() {
    m_completePort=CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,0);
    if(!m_completePort){
        return false;
    }
    m_ioCompletePort=CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,0);
    if(!m_ioCompletePort){
        return false;
    }
    m_listenSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(m_listenSocket==INVALID_SOCKET){
        return false;
    }
    CreateIoCompletionPort((HANDLE)m_listenSocket,m_ioCompletePort,0,0);
    hostent* localhost=gethostbyname("");
    char ip[64];
    inet_ntop(AF_INET,(struct in_addr*)*localhost->h_addr_list,ip,sizeof(ip));
    sockaddr_in service;
    service.sin_family=AF_INET;
    service.sin_addr.s_addr=inet_addr(ip);
    service.sin_port=htons(m_port);
    BOOL reuse=TRUE;
    setsockopt(m_listenSocket,SOL_SOCKET,SO_REUSEADDR,(const char*)&reuse,sizeof (reuse));
    if(bind(m_listenSocket,(SOCKADDR*)&service,sizeof (service))==SOCKET_ERROR)
    {
        std::cout<<"bind failed width error"<<WSAGetLastError()<<"\n";
        return false;
    }
    auto result =listen(m_listenSocket,100);
    if(result==SOCKET_ERROR){
        std::cout<<"listen failed width error:"<<WSAGetLastError()<<"\n";
        return false;
    }
    DWORD dwBytes=0;
    GUID guidAcceptEx=WSAID_ACCEPTEX;
    result=WSAIoctl(m_listenSocket,SIO_GET_EXTENSION_FUNCTION_POINTER,&guidAcceptEx,sizeof(guidAcceptEx),&lpfnAcceptEx,
                    sizeof(lpfnAcceptEx),&dwBytes, nullptr, nullptr);
    if(result==SOCKET_ERROR){
        std::cout<<"WSAIoctl failed width error:"<<WSAGetLastError()<<"\n";
        return false;
    }
    m_currentAcceptSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(m_currentAcceptSocket==INVALID_SOCKET) return false;

    int outBufLen=1024;
    memset(&m_acceptUnit,0, sizeof(m_acceptUnit));
    auto ret=lpfnAcceptEx(m_listenSocket,m_currentAcceptSocket,m_acceptBuffer.data(),0,sizeof(sockaddr_in)+16, sizeof(sockaddr_in)+16,
                          nullptr,&m_acceptUnit);
    if(ret==FALSE&&WSAGetLastError()==ERROR_IO_PENDING){
        std::cout<<"start listen "<<ip<<":"<<m_port<<std::endl;
        m_running=true;
    }
    return m_running;
}
void Server::waitingForAccept() {
    int id=1;
    while(isRunning()){
        DWORD bytes=0;
        ULONG_PTR dwCompletionKey;
        LPOVERLAPPED lpoverlapped=nullptr;
        auto ok=GetQueuedCompletionStatus(m_completePort,&bytes,&dwCompletionKey,&lpoverlapped,1000);
        if(ok){
            std::string info(m_acceptBuffer.begin(),m_acceptBuffer.begin()+bytes);
            CreateIoCompletionPort((HANDLE)m_currentAcceptSocket,m_ioCompletePort,0,0);
            auto ns=std::make_shared<ServerSocket>(id,m_currentAcceptSocket);
            ns->handleClose=socketClose;
            ns->handleError=socketError;
            ns->handleRecv=socketRecv;
            if(newConn){
                newConn(ns);
            }
            ++id;
            m_currentAcceptSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
            ok=tryNewConn();
            if(!ok)break;
        }else{
            auto errorCode=GetLastError();
            if(errorCode==WAIT_TIMEOUT){
                continue;
            }
            std::cout<<"has error"<<errorCode<<" in accept new Client "<<WSAGetLastError<<"\n";
            break;
        }

    }
}
void Server::waitingForIo() {
 while(isRunning()){
     DWORD bytes=0;
     ULONG_PTR dwCompletionKey;
     LPOVERLAPPED lpOverlapped= nullptr;
     auto ok=GetQueuedCompletionStatus(m_ioCompletePort,&bytes,&dwCompletionKey,&lpOverlapped,1000);
     if(ok){
         auto unit=(OverUnit*)lpOverlapped;
         assert(unit);
         auto socket=unit->socket;
         if(bytes==0){
             socket->onClose();
         }else{
             if(unit->type==0){
                 socket->onFinishedRecv(bytes);

             }else{
                 socket->onFinishedSend(bytes);
                 socket->trySendMore();
             }
         }
     }else {
         auto errorCode=GetLastError();
         if(errorCode==WAIT_TIMEOUT){
             continue;
         }
         std::cout<<"has error"<<errorCode<<" in io new Client "<<WSAGetLastError<<"\n";
         if(!lpOverlapped){
             break;
         }else{
             OverUnit* unit =(OverUnit*)lpOverlapped;
             auto socket=unit->socket;
             if(socket){
                 socket->onError(unit->type,WSAGetLastError());
             }
         }
         break;
     }
 }
}
bool Server::tryNewConn() {
    int outBufLen=1024;
    memset(&m_acceptUnit,0, sizeof(WSAOVERLAPPED));
    auto ret=lpfnAcceptEx(m_listenSocket,m_currentAcceptSocket,m_acceptBuffer.data(),0,sizeof(sockaddr_in)+16, sizeof(sockaddr_in)+16,
                          nullptr,&m_acceptUnit);
    if(ret==FALSE&&WSAGetLastError()==ERROR_IO_PENDING){
        return true;
    }else {
        return false;
    }
}
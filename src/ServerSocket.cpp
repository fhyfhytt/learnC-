//
// Created by Administrator on 2021/4/9 0009.
//

#include "ServerSocket.h"
#include <cassert>
#include <iostream>
#include <stdexcept>
ServerSocket::ServerSocket(int id, SOCKET s) :m_id(id),m_socket(s),m_currentRecv(8192){
    memset(&m_recvUnit,0, sizeof(m_recvUnit));
    m_recvUnit.socket= this;
    m_recvUnit.type=0;
    memset(&m_sendUnit,0,sizeof (m_sendUnit));
    m_sendUnit.socket=this;
    m_sendUnit.type=1;

}
ServerSocket::~ServerSocket() {
    closesocket(m_socket);
}
void ServerSocket::onError(int errorType, int errorCode) {
    if(handleError){
        auto self=shared_from_this();
        handleError(self,errorType,errorCode);

    }
}
void ServerSocket::onFinishedSend(int transfered) {

}
void ServerSocket::onFinishedRecv(int transfered) {
    auto self=shared_from_this();
    handleRecv(self,m_currentRecv.data(),transfered);
    startRecv();
}
void ServerSocket::onClose() {
    if(handleClose){
        auto self=shared_from_this();
        handleClose(self);
    }
}
void ServerSocket::sendMessage(const std::string &info) {
    std::lock_guard<std::mutex> lock(m_mutex);
    bool sending=m_sendBuffers.empty();
    m_sendBuffers.push_back(info);
    if(!sending){
        sendFrontBuffer();
    }
}
void ServerSocket::sendFrontBuffer() {
    m_sendWSA.len=m_sendBuffers.front().size();
    m_sendWSA.buf=(char *) m_sendBuffers.front().data();

    auto result=WSASend(m_socket,&m_sendWSA,1, nullptr,0,&m_sendUnit, nullptr);
    if(result!=0){
        auto error=WSAGetLastError();
        if(error!=WSA_IO_PENDING){
            throw std::runtime_error("bad for send message");
        }
    }
}
void ServerSocket::trySendMore() {
    std::lock_guard<std::mutex> lock(m_mutex);
    assert(!m_sendBuffers.empty());
    m_sendBuffers.pop_front();
    if(!m_sendBuffers.empty()){
        sendFrontBuffer();
    }
}
void ServerSocket::startRecv() {
    m_recvWSA.len=m_currentRecv.size();
    m_recvWSA.buf=m_currentRecv.data();
    DWORD flag=0;
    auto ret=WSARecv(m_socket,&m_recvWSA,1, nullptr,&flag,static_cast<WSAOVERLAPPED*>(&m_recvUnit), nullptr);
    if(ret!=0){
        auto code=WSAGetLastError();
        if(code!=WSA_IO_PENDING){
            std::cout<<"error for "<<code<<"\n";
            throw std::runtime_error("bad for start recv");
        }
    }
}
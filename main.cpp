#include <ws2tcpip.h>
#include <mswsock.h>
#include <cstdio>
#include <iostream>
#include <functional>
#include <cassert>
#include <signal.h>
#include <thread>
#include <map>
#include <string>
#include "Server.h"
//#pragma comment(lib,"Ws2_32.lib")
std::map<int,ServerSocket::pointer> players;
void handleRecvData(const ServerSocket::pointer& p,const char* data,int size){
    std::string info(data,data+size);
    std::cout<<"recv:"<<info<<std::endl;
    for(int i=0;i<10;++i){
        Sleep(i);
        p->sendMessage("+<html>hello world</html>!");
    }

}

void handleConn(Server* s){
    try{
        s->waitingForAccept();
    }catch (...){
        std::cout<<"has error \n";
    }
};
std::function<void()> handler;
void signalHandler(int code){
    std::cout<<"handle"<<code<<std::endl;
    if(handler){
        handler();
        handler=nullptr;
    }

};
void updateClientIo(HANDLE io,bool & isOver){
    while(!isOver){
        DWORD  bytes=0;
        ULONG_PTR dwCompletionKey;
        LPOVERLAPPED lpOverlapped= nullptr;

        auto ok= GetQueuedCompletionStatus(io,&bytes,&dwCompletionKey,&lpOverlapped,1000);
        if(ok){
            auto unit=(OverUnit*)lpOverlapped;
            assert(unit);
            auto socket=unit->socket;
            if(bytes==0){
                socket->onClose();
            }else{
                if(unit->type==0){
                    socket->onFinishedRecv(bytes);

                }else if(unit->type==1){
                    socket->onFinishedSend(bytes);
                    socket->trySendMore();
                }
            }
        }else{
            auto errorCode=GetLastError();
            if(errorCode==WAIT_TIMEOUT) continue;
            std::cout<<"has error"<<errorCode<< " in accept new client"<<WSAGetLastError()<<"\n";
            if(!lpOverlapped){
                break;
            }else{
                OverUnit* unit=(OverUnit*)lpOverlapped;
                auto socket=unit->socket;
                if(socket){
                    socket->onError(unit->type,WSAGetLastError());
                }
            }
        }
    }
};
void handleAddNewPlayer(const ServerSocket::pointer& p){
    std::cout<<"hello"<<p->id()<<"\n";
    p->startRecv();
    players.emplace(p->id(),p);
}
void handleRemovePlayer(const ServerSocket::pointer& p){
    std::cout<<"bye"<<p->id()<<"\n";
    players.erase(p->id());
}
ServerSocket::pointer buildClientSocket(const char* addr,u_short port,int id,HANDLE io ){
    auto connectSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(connectSocket==INVALID_SOCKET)
        return nullptr;
    sockaddr_in clientService;
    clientService.sin_family=AF_INET;
    clientService.sin_addr.s_addr=inet_addr(addr);
    clientService.sin_port=htons(port);
    auto iResult=connect(connectSocket,(SOCKADDR*)&clientService,sizeof (clientService));
    if(iResult==SOCKET_ERROR){
        std::cout<<"connect function failed width error:"<<WSAGetLastError()<<std::endl;
        iResult=closesocket(connectSocket);
        if(iResult==SOCKET_ERROR){
            std::cout<<"closesocket function failed width error : "<< WSAGetLastError()<<std::endl;
            return nullptr;
        }
        CreateIoCompletionPort((HANDLE)connectSocket,io,0,0);
        return std::make_shared<ServerSocket>(id,connectSocket);
    }else{
        return nullptr;
    }
}
int main(){
    signal(SIGINT,signalHandler);
//    signal(SIGTERM,signalHandler);
    WSADATA data;
    auto result=WSAStartup(MAKEWORD(2,2),&data);
    if(result!=NO_ERROR){
        std::cout<<"error at WSAStartup\n";
        return 1;
    }
    auto server=std::make_unique<Server>(8901);
    if(!server){
        WSACleanup();
        return 1;
    }
    server->socketRecv=handleRecvData;
    server->socketClose=handleRemovePlayer;
    auto clientIo=CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,0);
    bool gameOver=false;
    std::thread a([clientIo,&gameOver]{
        updateClientIo(clientIo,gameOver);
    });
    auto p=server.get();
    handler=[p]{p->stop();};
    auto ok=server->startAccept();
    std::unique_ptr<std::thread> t;
    std::unique_ptr<std::thread> io;
    if(ok){
        std::cout<<"ok\n";
        server->newConn=handleAddNewPlayer;
        t.reset(new std::thread([p]{p->waitingForAccept();}));
        io.reset(new std::thread([p]{p->waitingForIo();}));
    }
    std::cout<<"continue\n";

    /*
     * aaa*/

    while(server->isRunning()){
        Sleep(1);
    }
    std::cout<<"ok and bye \n";
    gameOver=true;
    a.join();
    if(t&&t->joinable()){
        t->join();
    }
    if(io&&io->joinable()){
        io->join();
    }
    CloseHandle(clientIo);
    players.clear();
    WSACleanup();
    return 0;
}

#include <iostream>
#include <thread>
#include <future>
#include <math.h>
#include <vector>
#include <chrono>
#include <ctime>
//多线程学习
void helloworld(){

    std::cout<<"helloworld"<<std::endl;
}
double calculate(double v){
    if((v*v - std::sqrt((v+5)*(v+2.5))/2.0)<0){
        return v;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    return std::sqrt((v*v - std::sqrt((v+5)*(v+2.5))/2.0));

}
template <typename Iter,typename Fun>
double visitRange(std::thread::id id,Iter iterBegin,Iter iterEnd,Fun func){
    double v=0;
    auto curId=std::this_thread::get_id();
    if(id==curId){
        std::cout<<"hello main thread\n";
    }else{
        std::cout<<"hello work thread\n";
    }
    for (auto iter=iterBegin;iter !=iterEnd;++iter){
        v+=func(*iter);
    }
    return v;
}
int main(){
    auto mainThreadId=std::this_thread::get_id();
    std::vector<double> v;
    for(int i=0;i<1000;i++){
        v.push_back(rand());
    }
    std::cout<<v.size()<<std::endl;
    double value=0.0;
    auto  nowc=clock();
    for(auto & info:v){
//        std::cout<<calculate(info)<<std::endl;
        value+=calculate(info);
    }
    auto  finishc=clock();
    std::cout<<"single thread"<<value<<" used "<<finishc-nowc<<std::endl;
    auto iter=v.begin()+(v.size()/3);
    auto iter2=v.begin()+(v.size()/3*2);
    std::thread b;
    std::thread c;


    b.join();
    c.join();


    double antherv=0.0;
    auto iterEnd=v.end();
    nowc=clock();
    std::thread s([&antherv,mainThreadId,iter,iterEnd](){
        antherv=visitRange(mainThreadId,iter,iterEnd,calculate);
    });
//    std::thread t(helloworld);
//    t.join();
    auto halfv=visitRange(mainThreadId,v.begin(),iter,calculate);
    finishc=clock();

    s.join();
    std::cout<<"moutithread:"<<halfv+antherv<<" used "<<finishc-nowc<<std::endl;

    //使用时间大概只有一半
    return 0;
}

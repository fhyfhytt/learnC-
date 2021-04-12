//
// Created by Administrator on 2021/4/8 0008.
//
#include <thread>
#include <mutex>
#include <atomic>
#include <iostream>
#include <chrono>
#include <list>
#include <vector>
#include <string>
#include <ctime>
#include <cstring>
#include <condition_variable>//高效
class Message{
public:
    const std::string& data ()const{
        return m_data;
    };
    Message(std::string d=std::string()):m_data(std::move(d)){};
private:
    std::string m_data;
};
std::atomic<int> totalSize{0};
std::mutex mutex;
std::condition_variable cv;
std::atomic<bool> ready{false};
bool quit{false};
std::list<Message> globalList;
void worker(int i){
    while (!ready){
//        std::this_thread::sleep_for(std::chrono::seconds(1));
//        std::this_thread::yield();//没有用 非常危险
        //do some
    };
    Message msg;
    while (!quit){
        {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock,[]{return quit || !globalList.empty();});
            if(quit){
                return;
            }
            auto iter=globalList.begin();
            msg=std::move(*iter);
            globalList.erase(iter);

        };
        totalSize+=strlen(msg.data().c_str());
    }
    std::lock_guard<std::mutex>lock(mutex);
    std::cout<<"hello world"<<i<<std::endl;
};
int main(){
    const auto threadCount=4;
    for(int i=0;i<500000;++i){
        globalList.push_back("this is a test " + std::to_string(i));
    }
    std::vector<std::thread> pool;
    for(int i=0;i<threadCount;++i){
        pool.emplace_back(worker,i);
    }
    ready=true;
    for(int i=0;i<300000;++i){
        {
            std::lock_guard<std::mutex> lock(mutex);
            globalList.push_back(std::string("second"));
        }

        cv.notify_one();
    }
    while (true){
        std::lock_guard<std::mutex> lock(mutex);
        if(globalList.empty()){
            quit=true;
            cv.notify_all();
            break;
        }
    }
    for (auto &v:pool){
        if(v.joinable()){
            v.join();
        }
    }
    std::cout<<"totalSize is "<<totalSize<<std::endl;
    return 0;
}
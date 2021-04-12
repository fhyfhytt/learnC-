//
// Created by Administrator on 2021/4/8 0008.
//
//#include <vector>
//#include <iostream>
//using namespace std;
//class testDemo
//{
//public:
//    testDemo(int num):num(num){
//        std::cout << "调用构造函数" << endl;
//    }
//    testDemo(const testDemo& other) :num(other.num) {
//        std::cout << "调用拷贝构造函数" << endl;
//    }
//    testDemo(testDemo&& other) :num(other.num) {
//        std::cout << "调用移动构造函数" << endl;
//    }
//private:
//    int num;
//};
//int main()
//{
//    cout << "emplace_back:" << endl;
//    std::vector<testDemo> demo1;
//    demo1.emplace_back(2);
//    cout << "push_back:" << endl;
//    std::vector<testDemo> demo2;
//    demo2.push_back(2);
//}
#include <thread>
#include <iostream>
#include <mutex>
#include <chrono>
#include <vector>
#include <atomic>
std::mutex Mutex;
std::atomic<bool> ready{false};
void worker(int i){
    while (!ready){
        std::this_thread::sleep_for(std::chrono::seconds(1));
//        std::this_thread::yield();//没有用 非常危险
        //do some
    }
    std::lock_guard<std::mutex>lock(Mutex);
    std::cout<<"hello world"<<i<<std::endl;
};
int main(){
    const auto threadCount=4;
    std::vector<std::thread> pool;
    for(int i=0;i<threadCount;i++){
        pool.emplace_back(worker,i);//后面是参数
    }
    std::this_thread::sleep_for(std::chrono::minutes (1));
    ready=true;
    for (auto &v:pool){
        if(v.joinable()){
            v.join();
        }
    }
    std::cout<<"bye bye\n";
    return 0;
}
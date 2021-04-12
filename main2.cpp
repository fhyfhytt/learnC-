#include <iostream>
#include <thread>
#include <vector>
#include <chrono>//时间戳
#include <cstdlib>
#include <atomic>//原子性操作
#include <mutex>//临界体 边界锁住和解锁
class Counter{
public:
    void addCount(){
        std::lock_guard<std::mutex> lock(m_mutex);
        m_count++;//写入寄存器 寄存器+1 写入内存
//    m_count.fetch_add(1);
    };//多线程共享变量问题
    int count() const {return m_count;};
    Counter():m_count{0}{};
    void addResource(int r){m_resource++;};
    int aveResource(){
      if(m_count==1){
          return 1;
      }
      return m_resource/m_count;
    };
    void lockMetux() {
        m_mutex.lock();
    };
    void unlockMutex(){
        m_mutex.unlock();
    }
private:

    int m_count;
    int m_resource;
    std::mutex m_mutex;
};
int work(int a){
    return a+a;
}

template<class Iter>
void realWork(Counter &c,double &totalvalue,Iter b,Iter e){
    for(;b!=e;++b){

        totalvalue+=work(*b);
        c.lockMetux();
        c.addCount();
        c.addResource(1);

    }
}
struct  BankAccount{
    BankAccount(int b):Balance(b){};
    int Balance;
    std::mutex Mutex;
};
void transferMoney(BankAccount& a,BankAccount& b,int money){
    if(&a==&b) return ;
    std::lock(a.Mutex,b.Mutex);//全部锁住;
    std::lock_guard<std::mutex> lockA(a.Mutex,std::adopt_lock);//已经锁住 传入不让进行锁
    std::lock_guard<std::mutex> lockB(b.Mutex,std::adopt_lock);//已经锁住 传入不让进行锁
    if(a.Balance<money){
        return ;
    }
    a.Balance-=money;
    b.Balance+=money;
};
int main(){
//    unsigned int n=std::thread::hardware_concurrency(); //4
//    std::cout<<n<<" concurrency there is support"<<std::endl;
    std::vector<int> cc;
    for (int i=0;i<10000000;++i){
        cc.push_back(rand()%100);
    }
//    try{
//
//    } catch (std::exception e) {
//        std::cout<<e.what()<<std::endl;
//    }
    double totalValue=0;
    Counter counter;
//    for(auto v:cc){
//        totalValue+=work(v);
//        counter.addCount();
//    }
    realWork(counter, totalValue, cc.begin(), cc.end());
    std::cout<<"total1 time  "<<counter.count()<<"totalvalue "<<totalValue<<std::endl;
    Counter counter2;
//    totalValue=0;
    std::atomic<double> totalValue{0};
    auto iter=cc.begin()+(cc.size()/3);
    auto iter2=cc.begin()+(cc.size()/3*2);
    auto end=cc.end();
//    void real_work=[&counter2,&totalValue](auto b,auto e){
//
//    };

//
    std::thread b([&counter2,&totalValue,iter,iter2]{
      realWork(counter2, totalValue, iter, iter2);
    });

    std::thread c([&counter2,&totalValue,iter2,end]{
        realWork(counter2, totalValue, iter2, end);

    });
//    [=]{}; [&]{};[*]{};
//std::ref(c); 值变引用
    realWork(counter2, totalValue, cc.begin(), iter);
    b.join();
    c.join();
//    thread的几种死法

    std::cout<<"total2 time  "<<counter2.count()<<"totalvalue "<<totalValue<<std::endl;
    return 0;
}
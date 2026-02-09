#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <queue> 
#include <future>

class threadpool{
public:

    explicit threadpool(int n = 1): n_threds(n), stop(false){
        for (int i=0; i<n; i++){
            threads.emplace_back([this](){
                //we'll be reading from the queue 
                while(1){             
                    std::unique_lock<std::mutex> lock(mtx);
                    cv.wait(lock, [this](){
                        return !tasks.empty() || stop;
                    });
                    
                    if (stop){
                        return ;
                    }
                    
                    if (tasks.empty()){
                        continue;
                    }

                    std::function<void()> task = std::move(tasks.front());
                    tasks.pop();
                    //safe to unlock
                    lock.unlock();
                    //execute task
                    task();
                }
            });
        }
    }

    //we can use perfect forwarding 
    template <typename F, typename... Args>
    auto execute_task(F&& f, Args&&... args) -> std::future<decltype(f(args...))>{ 
        using return_type = decltype(f(args...));

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> res = task->get_future();

        std::unique_lock<std::mutex> lock(mtx);

        tasks.emplace([task]() -> void{
            (*task)();
        });

        lock.unlock();
        cv.notify_one();

        return res;
    }

    ~threadpool(){
        std::unique_lock<std::mutex> lock(mtx);
        stop = true; 
        lock.unlock();
        cv.notify_all();

        for (auto &task: threads){
            task.join();
        }
    }

private: 
    const int n_threds; //will keep this fixed !! 
    std::mutex mtx;  //mutex to prevent race conditions in the tasks queue 
    std::condition_variable cv;
    std::queue<std::function<void()>> tasks; 
    std::vector<std::thread> threads; 
    bool stop; 
};

int fun(int n, int m){
    std::this_thread::sleep_for(std::chrono::seconds(30));
    std::cout<<"function executed \n";
    return n*m; 
}

int main(){

    threadpool pool(4);
    auto v = pool.execute_task(fun,4,4);
    std::cout<<"return value "<<v.get()<<"\n";
    // std::cout<<"threadpool program \n";
    // for (int i=0; i<1000; i++){
    //     std::thread t1(fun);
    //     t1.detach();
    // }

    std::this_thread::sleep_for(std::chrono::seconds(100));
}
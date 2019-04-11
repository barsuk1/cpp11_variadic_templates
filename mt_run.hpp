#pragma once 

#include <iostream>
#include <thread>
#include <atomic>
#include <future>
#include <utility>
#include <vector>
#include <chrono>
#include <exception>

/* usage example : have your method and parameters called with the test_mt:
int my_func(int p)
{
        cout << "hello world " << p  << endl;
}

assign it to std::function
std::function<void(int)> f = my_func;

call test_mt and specify number of threads that will run it as the first parameter
int i = 123;
test_mt(5, f, i);

*/

namespace test_infra {

//number of threads
static std::atomic_int g_count(0);

template <typename Ret, typename ... FArgs, typename ... Args>
std::vector<std::chrono::duration<double, std::milli> > test_mt(int threads, std::function< Ret(FArgs...)> const & func,
                  Args &&... as)
{
    using namespace std;
    vector<chrono::duration<double, std::milli> > results(threads,chrono::duration<double, std::milli>(0));
    g_count = threads;

    vector< future<chrono::duration<double, std::milli> > > futures(threads);
    vector< promise< chrono::duration<double, std::milli>> > promises(threads);
    vector<thread> tv(g_count);
    
    chrono::duration<double, std::milli> total(0), run(0);
    chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();        
    

    for( auto i = 0; i < threads; i ++ ){
#ifdef DEBUG
        cout << "Starting thread " << i << endl;
#endif
        promise< chrono::duration<double, std::milli> > time_promise;
        futures[i] = promises[i].get_future();

        auto thread_lambda = [&promises, &func, i](Args&&... as) {
            int num = --g_count;
            int thrcnt = num;
#ifdef DEBUG
            cout << "In thread " << thrcnt  << endl;
#endif
            //wait till all threads are up 
            while(num){
                usleep(10);
                num = g_count.load();
            }
#ifdef DEBUG
            cout << "Running thread " << thrcnt << endl;
#endif
            chrono::duration<double, std::milli> t(0);
            std::chrono::time_point<std::chrono::system_clock> start, end;

            try {
                //run the func and measure how long does it take
                start = chrono::system_clock::now();    
                func(std::forward<Args>(as)...);
                end = chrono::system_clock::now();    
                t = end - start;
#ifdef DEBUG
                cout << "Thread " << thrcnt << " time " <<  t.count() << " ms "  << endl;
#endif
                promises[i].set_value(t);
            } catch(const std::exception& ex) {
                cout << ex.what() << endl;
            } catch(...) {
                cout << "Unknown exception caught" << endl;
            }

        };
        //to avoid g++ compile error, use std::ref - thread constructor takes first parameter by ref
        thread t(thread_lambda, std::ref(as)...);
        tv[i] = move(t);
    }
    
    //wait for all futures, record result and the total time
    for( auto i =0; i < futures.size(); i++ ){
        futures[i].wait();
        results[i] = futures[i].get();
            total += results[i];
    }

    //join all the running threads
    for( auto i = 0; i < tv.size(); i ++ ){
#ifdef DEBUG
        cout << "Joining thread " << i << endl;
#endif
        tv[i].join();
    }
    //print the total
    end = std::chrono::system_clock::now();    
    run = end-start;
    cout << "Total run time " << run.count() << endl;
    //return vector of durations
    return results;
}


};

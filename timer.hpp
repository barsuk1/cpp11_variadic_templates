#pragma once
#include <chrono>
#include <iostream>

namespace test_infra {

    class MsTimer {
    public:
        MsTimer(){ reset(); } 
        void start(){
            beg = std::chrono::system_clock::now();       
        }
        void stop(){
            end = std::chrono::system_clock::now();
            if( std::chrono::system_clock::to_time_t(end) > std::chrono::system_clock::to_time_t(beg) ){
                last = end - beg;
                total += last;
            }
            end = std::chrono::time_point<std::chrono::system_clock>(); 
            beg = std::chrono::time_point<std::chrono::system_clock>(); 
        }
        void reset(){
            end = std::chrono::time_point<std::chrono::system_clock>(); 
            beg = std::chrono::time_point<std::chrono::system_clock>(); 
            last = std::chrono::duration<double, std::milli> (0);
            total = std::chrono::duration<double, std::milli> (0);
        }
        uint64_t getLast()const{
            return last.count();
        }
        uint64_t getTotal()const{
            return total.count();
        }
        operator uint64_t()const{
            return getLast();
        }
    private:
        
        std::chrono::duration<double, std::milli> total, last;
        std::chrono::time_point<std::chrono::system_clock> beg, end;
    };   

    std::ostream& operator <<(std::ostream& out, const MsTimer& t){
        out << " Total Elapsed " << t.getTotal() << " ms Last Elapsed " << t.getLast() << " ms ";
    }
};

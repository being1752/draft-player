#pragma once
#include <list>
#include <mutex>

template<typename T>
class Queue {
public:
    Queue(){}
    ~Queue(){}
    void push(const T& data) {
        std::lock_guard<std::mutex> lock(mtx);
        lst.emplace_back(data);
    }
    bool pop(T& data) {
        std::lock_guard<std::mutex> lock(mtx);
        if (lst.empty()) {
            return false;
        }
        data = lst.front();
        lst.pop_front();
        return true;
    }
    int size() {
        std::lock_guard<std::mutex> lock(mtx);
        return lst.size();
    }
    bool isEmpty() {
        std::lock_guard<std::mutex> lock(mtx);
        return lst.empty();
    }

protected:
    std::list<T> lst;
    std::mutex mtx;
};
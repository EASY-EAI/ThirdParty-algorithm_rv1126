#ifndef MEGSDK_SHARED_QUENE_H
#define MEGSDK_SHARED_QUENE_H

#include <memory>
#include <mutex>
#include <queue>
#include <condition_variable>

template<typename T>
class shared_queue {
public:
    shared_queue() = default;

    shared_queue& operator=(const shared_queue&) = delete;
    shared_queue(const shared_queue& other) = delete;

    bool is_empty() { return empty(); }

    void producer(T task){
        push(std::move(task));
    }

    T consume(){
        T x = T();
        wait_and_pop(x);
        return std::move(x);
    }

    void invoke_all(){
        std::lock_guard<std::mutex> lock(m_);
        data_cond_.notify_all();
    }

    void set_runout() { _run_out = true; }

    void push(T&& item) {
        {
            std::lock_guard<std::mutex> lock(m_);
            queue_.push(std::move(item));
        }
        data_cond_.notify_one();
    }

    /// \return immediately, with true if successful retrieval
    bool try_and_pop(T& popped_item) {
        std::lock_guard<std::mutex> lock(m_);
        if (queue_.empty()) {
            return false;
        }
        popped_item = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    /// Try to retrieve, if no items, wait till an item is available and try again
    void wait_and_pop(T& popped_item) {
        std::unique_lock<std::mutex> lock(m_);
        while (queue_.empty() && !_run_out) {
            data_cond_.wait(lock);
            //  This 'while' loop is equal to
            //  data_cond_.wait(lock, [](bool result){return !queue_.empty();});
        }

        if(_run_out && queue_.empty())
        {
            popped_item = T();
        }
        else{
            popped_item = std::move(queue_.front());
            queue_.pop();
        }
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(m_);
        return queue_.empty();
    }

    unsigned size() const {
        std::lock_guard<std::mutex> lock(m_);
        return queue_.size();
    }


private:
    std::queue<T> queue_;
    mutable std::mutex m_;
    std::condition_variable data_cond_;
    bool _run_out = false;
};

#endif //MEGSDK_SHARED_QUENE_H

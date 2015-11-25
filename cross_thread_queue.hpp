/*
 * ---------------------------------------
 * File: cross_thread_queue.hpp
 * Created  Date: 2023-05-24
 * Author:  Jules
 * Contact: https://github.com/jules-ai
 * ---------------------------------------
 * - Compatible with C++14 or higher
 * - Recommended usage with smart pointer
 */
#ifndef _JULES_CROSS_THREAD_QUEUE_HPP_
#define _JULES_CROSS_THREAD_QUEUE_HPP_

#include <deque>
#include <vector>
#include <mutex>
#include <limits>
#include <thread>
#include <chrono>
using namespace std::chrono_literals;

namespace Jules::utils
{
    template <typename T>
    class CrossThreadQueue
    {
    public:
        explicit CrossThreadQueue() = default;
        CrossThreadQueue(const CrossThreadQueue &) = delete;
        CrossThreadQueue &operator=(const CrossThreadQueue &) = delete;
        CrossThreadQueue(CrossThreadQueue &&) = delete;
        CrossThreadQueue &operator=(CrossThreadQueue &&) = delete;

        /// @brief set capacity of queue
        /// @param ic target capacity of queue
        void SetMaxCount(std::size_t ic);

        /// @brief get capacity of queue
        /// @return current capacity of queue
        std::size_t GetMaxCount();

        /// @brief get size of queue
        /// @return current size of queue
        std::size_t Size();

        /// @brief check if queue is full
        /// @return true for full
        bool Full();

        /// @brief check if queue is empty
        /// @return true for empty
        bool Empty();

        /// @brief try to push element into queue
        /// @param t element
        /// @return true for pushed false for failed
        bool Try_Push(const T &t);

        /// @brief try to push elements into queue
        /// @param ts vector of elements
        /// @return true for pushed false for failed
        bool Try_Push(const std::vector<T> &ts);

        /// @brief push element into queue
        /// @param t element
        void Push(const T &t);

        /// @brief push elements into queue
        /// @param ts vector of elements
        void Push(const std::vector<T> &ts);

        /// @brief try to pop element from queue
        /// @param t pointer to poped element
        /// @return true for poped false for failed
        bool Pop(T *t = nullptr);

        /// @brief
        /// @param num
        /// @return
        auto Pop(std::size_t num = 1);

        /// @brief
        /// @warning potential risk of deadlock, better not use!
        /// @param t
        /// @return
        [[deprecated("Potential risk of deadlock, better not use!")]] bool Pop_Must(T *t);

        /// @brief clear the queue
        void Clear();

        /// @brief erase element from value
        /// @warning Be careful with (operator==)!
        /// @param t element value to erase
        /// @return true for erased false for not found
        bool Erase(const T &t);

        /// @brief Sleep auxiliary function(thread independent)
        /// @param duration time duration in millisecond
        static void Sleep(size_t duration);

    private:
        std::deque<T> queue_;
        std::mutex mutex_;
        std::size_t max_count_ = std::numeric_limits<size_t>::max();
    };

    template <typename T>
    void CrossThreadQueue<T>::SetMaxCount(std::size_t ic)
    {
        std::unique_lock<std::mutex> lck(mutex_);
        max_count_ = ic;
        while (queue_.size() >= max_count_)
        {
            queue_.pop_front();
        }
    }

    template <typename T>
    size_t CrossThreadQueue<T>::GetMaxCount()
    {
        std::unique_lock<std::mutex> lck(mutex_);
        return max_count_;
    }

    template <typename T>
    size_t CrossThreadQueue<T>::Size()
    {
        std::unique_lock<std::mutex> lck(mutex_);
        return queue_.size();
    }

    template <typename T>
    bool CrossThreadQueue<T>::Full()
    {
        std::unique_lock<std::mutex> lck(mutex_);
        return queue_.size() == max_count_;
    }

    template <typename T>
    bool CrossThreadQueue<T>::Empty()
    {
        std::unique_lock<std::mutex> lck(mutex_);
        return queue_.empty();
    }

    template <typename T>
    bool CrossThreadQueue<T>::Try_Push(const T &t)
    {
        std::unique_lock<std::mutex> lck(mutex_);
        if (queue_.size() < max_count_)
        {
            queue_.push_back(t);
            return true;
        }
        return false;
    }

    template <typename T>
    bool CrossThreadQueue<T>::Try_Push(const std::vector<T> &ts)
    {
        std::unique_lock<std::mutex> lck(mutex_);
        if (ts.size() + queue_.size() > max_count_)
            return false;

        for (auto &t : ts)
            queue_.push_back(t);
        return false;
    }

    template <typename T>
    void CrossThreadQueue<T>::Push(const T &t)
    {
        std::unique_lock<std::mutex> lck(mutex_);
        queue_.push_back(t);

        if (queue_.size() > max_count_)
            queue_.pop_front();
    }

    template <typename T>
    void CrossThreadQueue<T>::Push(const std::vector<T> &ts)
    {
        std::unique_lock<std::mutex> lck(mutex_);
        for (auto &t : ts)
        {
            queue_.push_back(t);
            if (queue_.size() >= max_count_)
            {
                queue_.pop_front();
            }
        }
    }

    template <typename T>
    bool CrossThreadQueue<T>::Pop_Must(T *t)
    {
        using namespace std::chrono_literals;
        std::unique_lock<std::mutex> lck(mutex_);
        while (queue_.empty())
        {
            std::this_thread::sleep_for(10ms);
        }
        *t = queue_.front();
        queue_.pop_front();
        return true;
    }

    template <typename T>
    bool CrossThreadQueue<T>::Pop(T *t /* = nullptr */)
    {
        std::unique_lock<std::mutex> lck(mutex_);
        if (queue_.empty())
            return false;

        if (t)
            *t = queue_.front();

        queue_.pop_front();
        return true;
    }

    template <typename T>
    auto CrossThreadQueue<T>::Pop(std::size_t num /* = 1 */)
    {
        std::unique_lock<std::mutex> lck(mutex_);
        auto sz = std::min(num, queue_.size());
        std::vector<T> ts(sz);
        for (size_t i = 0; i < sz; i++)
        {
            ts[i] = queue_.front();
            queue_.pop_front();
        }
        return ts;
    }

    template <typename T>
    void CrossThreadQueue<T>::Clear()
    {
        std::unique_lock<std::mutex> lck(mutex_);
        queue_.clear();
    }

    template <typename T>
    bool CrossThreadQueue<T>::Erase(const T &t)
    {
        std::unique_lock<std::mutex> lck(mutex_);
        for (std::size_t k = 0; k < queue_.size(); k++)
        {
            if (t == queue_.at(k))
            {
                queue_.erase(queue_.begin() + k);
                return true;
            }
        }
        return false;
    }

    template <typename T>
    void CrossThreadQueue<T>::Sleep(size_t duration)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(duration));
    }

} // ! namespace Jules::utils

#endif

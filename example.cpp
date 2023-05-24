#include "cross_thread_queue.hpp"
#include <string>
#include <memory>
#include <random>
#include <atomic>
#include <stdio.h>

int main()
{
    struct Dummy
    {
        int a;
        float b;
        double c;
        std::string d;
    };
    

    using Queue = Jules::utils::CrossThreadQueue<std::shared_ptr<Dummy>>;
    Queue que_vacant;
    Queue que_worker0;
    Queue que_worker10;
    Queue que_worker11;
    Queue que_result;
    std::atomic<bool> running{false};

    std::vector<std::shared_ptr<Dummy>> resource_pool;
    constexpr int resource_num = 200;
    for (int i = 0; i < resource_num; i++)
    {
        resource_pool.emplace_back(std::make_shared<Dummy>());
        resource_pool.back()->a = i;
        que_vacant.Push(resource_pool.back());
    }

    /* work flow:              /-- workers[1] -> workers[3]--\
     *    input -> workers[0] <                               >  -> result
     *                         \-- workers[2] -> workers[4]--/
     */

    auto work = [&running](Queue &que_input, Queue &que_output)
    {
        while (running)
        {
            auto data = que_input.Pop(1);
            if (data.empty())
            {
                que_input.Sleep(100);
                continue;
            }

            // processing data here
            // do something
            data[0]->a = 1;
            data[0]->b = 1.0;
            data[0]->c = 1.00;

            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> distrib(1, 107);
            Queue::Sleep(distrib(gen));

            que_output.Push(data);
        }
    };

    running = true;
    std::vector<std::thread> workers;
    {
        workers.emplace_back(std::thread(work, std::ref(que_vacant), std::ref(que_worker0)));
        workers.emplace_back(std::thread(work, std::ref(que_worker0), std::ref(que_worker10)));
        workers.emplace_back(std::thread(work, std::ref(que_worker0), std::ref(que_worker11)));
        workers.emplace_back(std::thread(work, std::ref(que_worker10), std::ref(que_result)));
        workers.emplace_back(std::thread(work, std::ref(que_worker11), std::ref(que_result)));
    };

    while (que_result.Size() < 200)
    {
        printf("[%3lu] [%3lu] [%3lu] [%3lu] [%3lu]\n", que_vacant.Size(), que_worker0.Size(), que_worker10.Size(), que_worker11.Size(), que_result.Size());
        fflush(stdout);
        Queue::Sleep(10);
    }

    running = false;
    for (auto &worker : workers)
    {
        worker.join();
    }
    printf("[%3lu] [%3lu] [%3lu] [%3lu] [%3lu]\n", que_vacant.Size(), que_worker0.Size(), que_worker10.Size(), que_worker11.Size(), que_result.Size());
    printf("Finished.\n");

    return 0;
}
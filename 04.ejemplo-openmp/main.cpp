#include <iostream>
#include <omp.h>
#include <fmt/core.h>

int main()
{
    /*    #pragma omp parallel
       {
           #pragma omp master
       {
           int threads_count = omp_get_num_threads();

           fmt::println("Hello serial world, hello OpenMP!");
           fmt::println("I have {} threads ", threads_count);

       }

       int thread_id = omp_get_thread_num();
       fmt::println("My thread ID is {}", thread_id);

           //int threads_count = omp_get_num_threads();
           //int thread_id = omp_get_thread_num();

           //fmt::println("Goodbye serial world, hello OpenMP!");
           //fmt::println("I have {} threads and my thread ID is {}", threads_count, thread_id);
       } */

    int num_elementos = 15;

#pragma omp parallel for num_threads(4)
    for (int i = 0; i < num_elementos; i++)
    {
        // fmt::println("Thread {} is processing element {}", omp_get_thread_num(), i);
    }

#pragma omp parallel num_threads(4)
    {
        int thread_id = omp_get_thread_num();
        int thread_num = omp_get_num_threads();

        int delta = std::ceil(num_elementos * 1.0 / thread_num);
        int start = thread_id * delta;
        int end = (thread_id + 1) * delta;

        if (thread_id == thread_num - 1)
        {
            end = num_elementos;
        }

        fmt::println("Thread {} is processing elements from {} to {}", thread_id, start, end - 1);

        for (int i = start; i < end; i++)
        {
            // fmt::println("Thread {} is processing element {}", thread_id, i);
        }
    }

    #pragma omp parallel num_threads(4)
    {
        int thread_id = omp_get_thread_num();
        int thread_num = omp_get_num_threads();

        for (int i = thread_id; i < num_elementos; i += 4)
        {
            fmt::println("Thread {} is processing element {}", thread_id, i);
        }
    }

/*     #pragma omp parallel {
        while(true) {

        }
    } */

    return 0;
}
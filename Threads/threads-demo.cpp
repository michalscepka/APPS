// ***********************************************************************
//
// Demo program for subject Computer Architectures and Paralel systems 
// Petr Olivka, Dept. of Computer Science, FEECS, VSB-TU Ostrava
// email:petr.olivka@vsb.cz
//
// Threads programming example for Linux (10/2016)
// For the propper testing is necessary to have at least 2 cores CPU
//
// ***********************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/param.h>
#include <pthread.h>

#define TYPE int

class task_part
{
public:
    int id;                 // user identification
    int first, last, N;     // data range
    TYPE* data;             // array
    TYPE max = 0;           // result
    TYPE* result;   	    // result*

    task_part() {}

    task_part(int myid, int first, int last, TYPE* ptr)
    {
        this->id = myid;
        this->first = first;
        this->last = last;
        this->data = ptr;
        this->N = last - first;
        this->result = fillArray();
    }

    TYPE get_max() { return max; }

    TYPE* get_result() { return result; }

    // function search_max search the largest number in part of array
    // from the left (included) up to the right element
    TYPE search_max()
    {
        TYPE max_elem = data[first];
        for (int i = first; i < last; i++)
            if (max_elem < data[i])
                max_elem = data[i];
        return max_elem;
    }

    TYPE* fillArray()
    {
        TYPE* result = new TYPE[N];
        for (int i = 0; i < N; i++)
        {
            result[i] = data[first + i];
        }
        return result;
    }

    TYPE* bubbleSort()
    {
        for (int i = 0; i < N - 1; i++)
        {
            for (int j = 0; j < N - i - 1; j++)
            {
                if (result[j + 1] < result[j])
                {
                    TYPE tmp = result[j + 1];
                    result[j + 1] = result[j];
                    result[j] = tmp;
                }
            }
        }
        return result;
    }

    TYPE* selectionSort()
    {
        for (int i = 0; i < N - 1; i++)
        {
            int minIndex = i;
            for (int j = i + 1; j < N; j++)
            {
                if (result[minIndex] > result[j])
                    minIndex = j;
            }
            TYPE tmp = result[i];
            result[i] = result[minIndex];
            result[minIndex] = tmp;
        }
        return result;
    }

    TYPE* insertionSort()
    {
        for (int i = 0; i < N - 1; i++)
        {
            int j = i + 1;
            TYPE tmp = result[j];
            while (j > 0 && tmp < result[j - 1])
            {
                result[j] = result[j - 1];
                j--;
            }
            result[j] = tmp;
        }
        return result;
    }
};

//vypise pole
void printArray(TYPE* pole, int N)
{
    for (int i = 0; i < N; i++)
    {
        printf("%d\t", pole[i]);
    }
    printf("\n");
}

//spoji setrizena dve pole do tretiho
TYPE* mergeArrays(TYPE* arr1, TYPE* arr2, int N1, int N2)
{
    TYPE* answer = new int[N1 + N2];
    int i = 0, j = 0, k = 0;

    while (i < N1 && j < N2)
        answer[k++] = arr1[i] < arr2[j] ? arr1[i++] : arr2[j++];

    while (i < N1)
        answer[k++] = arr1[i++];

    while (j < N2)
        answer[k++] = arr2[j++];

    return answer;
}

// Thread will search the largest element in array 
// from element arg->from with length of arg->length.
// Result will be stored to arg->max.
void *my_thread( void *void_arg )
{
    task_part *ptr_task = (task_part*)void_arg;
    printf("Thread %d started from %d to %d...\n", ptr_task->id, ptr_task->first, ptr_task->last);

    //ptr_task->max = ptr_task->search_max();
    //ptr_task->result = ptr_task->bubbleSort();
    ptr_task->result = ptr_task->selectionSort();
    //ptr_task->result = ptr_task->insertionSort();

    //printf("Found maximum in thread %d is %d\n", ptr_task->id, ptr_task->max);
    printf("\nSorted in thread %d:\n", ptr_task->id);
    printArray(ptr_task->result, ptr_task->N);

    return NULL;
}

// Time interval between two measurements
int timeval_to_ms( timeval *before, timeval *after )
{
    timeval res;
    timersub( after, before, &res );
    return 1000 * res.tv_sec + res.tv_usec / 1000;
}

#define LENGTH_LIMIT 10000000

int main(int na, char **arg)
{
    // The number of elements must be used as program argument
    if (na != 2)
    {
        printf("Specify number of elements, at least %d.\n", LENGTH_LIMIT);
        return 0;
    }
    int N = atoi(arg[1]);
    /*if (N < LENGTH_LIMIT)
    {
        printf("The number of elements must be at least %d.\n", LENGTH_LIMIT);
        return 0;
    }*/

    // array allocation
    TYPE *pole = new TYPE[N];
    if (!pole)
    {
        printf("Not enought memory for array!\n");
        return 1;
    }

    // Initialization of random number generator
    srand((int)time(NULL));

    printf("Random numbers generetion started...");
    for (int i = 0; i < N; i++)
    {
        pole[i] = rand() % (N * 10);
        if (!(i % LENGTH_LIMIT))
        {
            printf(".");
            fflush(stdout);
        }
    }
    printArray(pole, N);

    printf("\nSort using 4 threads...\n");

    pthread_t thread1, thread2, thread3, thread4;
    task_part part1(1, 0, N / 4, pole);
    task_part part2(2, N / 4, (N / 4) * 2, pole);
    task_part part3(3, (N / 4) * 2, (N / 4) * 3, pole);
    task_part part4(4, (N / 4) * 3, (N / 4) * 4, pole);
    timeval time_before, time_after;

    // Time recording before searching
    gettimeofday(&time_before, NULL);

    // Threads starting
    pthread_create(&thread1, NULL, my_thread, &part1);
    pthread_create(&thread2, NULL, my_thread, &part2);
    pthread_create(&thread3, NULL, my_thread, &part3);
    pthread_create(&thread4, NULL, my_thread, &part4);

    // Waiting for threads completion
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);
    pthread_join(thread4, NULL);

    // Time recording after searching
    gettimeofday(&time_after, NULL);

    TYPE* sorted1 = mergeArrays(part1.result, part2.result, part1.N, part2.N);
    TYPE* sorted2 = mergeArrays(part3.result, part4.result, part3.N, part4.N);
    pole = mergeArrays(sorted1, sorted2, part1.N + part2.N, part3.N + part4.N);
    printf("Result:\n");
    printArray(pole, N);
    printf("The search time: %d [ms]\n", timeval_to_ms(&time_before, &time_after));

    /*printf("\nMaximum number search using two threads...\n");
    pthread_t pt1, pt2;
    task_part part1(1, 0, N / 2, pole);
    task_part part2(2, N / 2, N, pole);
    timeval time_before, time_after;

    // Time recording before searching
    gettimeofday(&time_before, NULL);

    // Threads starting
    pthread_create(&pt1, NULL, my_thread, &part1);
    pthread_create(&pt2, NULL, my_thread, &part2);

    // Waiting for threads completion 
    pthread_join(pt1, NULL);
    pthread_join(pt2, NULL);

    // Time recording after searching
    gettimeofday(&time_after, NULL);

    printf("The found maximum: %d\n", MAX(part1.get_max(), part2.get_max()));
    printf("The search time: %d [ms]\n", timeval_to_ms(&time_before, &time_after));

    printf("\nMaximum number search using one thread...\n");

    gettimeofday(&time_before, NULL);

    // Searching in single thread
    task_part single(333, 0, N, pole);
    TYPE res = single.search_max();

    gettimeofday(&time_after, NULL);

    printf("The found maximum: %d\n", res);
    printf("The search time: %d [ms]\n", timeval_to_ms(&time_before, &time_after));*/
}

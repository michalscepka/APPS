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
#include <vector>

#define LENGTH_LIMIT 10000000
#define TYPE int

bool vypis = 0;

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
        this->result = nullptr;
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
            result[i] = rand() % (100);
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
    if(vypis)
    {
        for (int i = 0; i < N; i++)
        {
            printf("%d\t", pole[i]);
        }
        printf("\n");
    }
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

// Thread will fill array from element arg->first to arg->last.
// Result will be stored to arg->result*.
void *thread_fill(void *void_arg)
{
    task_part *ptr_task = (task_part*)void_arg;
    //printf("Thread %d filling started from %d to %d...\n", ptr_task->id, ptr_task->first, ptr_task->last);

    ptr_task->result = ptr_task->fillArray();

    //printf("Filled in thread %d:\n", ptr_task->id);
    printArray(ptr_task->result, ptr_task->N);

    return NULL;
}

// Thread will sort array from element arg->first to arg->last.
// Result will be stored to arg->result*.
void *thread_sort(void *void_arg)
{
    task_part *ptr_task = (task_part*)void_arg;
    //printf("Thread %d sorting started from %d to %d...\n", ptr_task->id, ptr_task->first, ptr_task->last);

    //ptr_task->result = ptr_task->bubbleSort();
    //ptr_task->result = ptr_task->selectionSort();
    ptr_task->result = ptr_task->insertionSort();

    //printf("Sorted in thread %d:\n", ptr_task->id);
    printArray(ptr_task->result, ptr_task->N);

    return NULL;
}

// Time interval between two measurements
int timeval_to_ms(timeval *before, timeval *after)
{
    timeval res;
    timersub( after, before, &res );
    return 1000 * res.tv_sec + res.tv_usec / 1000;
}

int main(int na, char** arg)
{
    // The number of elements must be used as program argument
    if (na < 2)
    {
        printf("Specify number of elements and optionaly number of threads.\n");
        return 0;
    }
    int N = atoi(arg[1]);

    // array allocation
    TYPE *pole = new TYPE[N];
    if (!pole)
    {
        printf("Not enought memory for array!\n");
        return 1;
    }

    // Initialization of random number generator
    srand((int)time(NULL));

    //vytvoreni threadu a task_partu
    int th_count = 2;
    if(na == 3)
        th_count = atoi(arg[2]);
        
    pthread_t threads[th_count];
    std::vector<task_part> parts;
    for (int i = 0; i < th_count; ++i)
    {
        parts.push_back(task_part(i + 1, i * (N / th_count), (i + 1) * (N / th_count), pole));
    }
    printf("\nUsing %d threads...\n", th_count);

    timeval time_before, time_after, time_all_start, time_all_stop;

    gettimeofday(&time_all_start, NULL);

    //FILLING
    // Time recording before filling
    gettimeofday(&time_before, NULL);
    printf("\nRandom numbers generating started...\n");
    for (int i = 0; i < th_count; ++i)
    {
        pthread_create(&threads[i], NULL, thread_fill, &parts[i]);
    }
    for (int i = 0; i < th_count; ++i)
    {
        pthread_join(threads[i], NULL);
    }
    // Time recording after filling
    gettimeofday(&time_after, NULL);
    printf("The filling time: %d [ms]\n", timeval_to_ms(&time_before, &time_after));

    //SORTING
    // Time recording before sorting
    gettimeofday(&time_before, NULL);
    printf("\nSorting started...\n");
    for (int i = 0; i < th_count; ++i)
    {
        pthread_create(&threads[i], NULL, thread_sort, &parts[i]);
    }
    for (int i = 0; i < th_count; ++i)
    {
        pthread_join(threads[i], NULL);
    }
    // Time recording after sorting
    gettimeofday(&time_after, NULL);
    printf("The sorting time: %d [ms]\n", timeval_to_ms(&time_before, &time_after));

    //MERGE
    // Time recording before merging
    gettimeofday(&time_before, NULL);
    printf("\nMerging started...\n");
    //printf("Result:\n");
    pole = nullptr;
    for(int i = 0; i < th_count - 1; i++)
    {
        if(pole == nullptr)
        {
            pole = mergeArrays(parts[i].result, parts[i + 1].result, parts[i].N, parts[i + 1].N);
            N = parts[i].N + parts[i].N;
        }
        else
        {
            pole = mergeArrays(pole, parts[i + 1].result, N, parts[i + 1].N);
            N += parts[i + 1].N;
        }
    }
    printArray(pole, N);
    // Time recording after sorting
    gettimeofday(&time_after, NULL);
    printf("The merging time: %d [ms]\n", timeval_to_ms(&time_before, &time_after));

    gettimeofday(&time_all_stop, NULL);
    printf("The time all: %d [ms]\n", timeval_to_ms(&time_all_start, &time_all_stop));
}

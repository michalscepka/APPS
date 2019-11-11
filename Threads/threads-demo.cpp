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
//#define TYPE __uint32_t
#define TYPE int

bool vypis = 1;

class task_part
{
public:
    int id;                 // user identification
    int first, last;     	// data range
    TYPE* data;             // array

    task_part(int myid, int first, int last, TYPE* ptr)
    {
        this->id = myid;
        this->first = first;
        this->last = last;
        this->data = ptr;
    }

    void bubbleSort()
    {
        for (int i = first; i < last - 1; i++)
        {
            for (int j = first; j < last - 1; j++)
            {
                if (data[j + 1] < data[j])
                {
                    TYPE tmp = data[j + 1];
                    data[j + 1] = data[j];
                    data[j] = tmp;
                }
            }
        }
    }

    void selectionSort()
    {
        for (int i = first; i < last - 1; i++)
        {
            int minIndex = i;
            for (int j = i + 1; j < last; j++)
            {
                if (data[minIndex] > data[j])
                    minIndex = j;
            }
            TYPE tmp = data[i];
            data[i] = data[minIndex];
            data[minIndex] = tmp;
        }
    }

    void insertionSort()
    {
        for (int i = first; i < last - 1; i++)
        {
            int j = i + 1;
            TYPE tmp = data[j];
            while (j - first > 0 && tmp < data[j - 1])
            {
            	data[j] = data[j - 1];
                j--;
            }
            data[j] = tmp;
        }
    }

    //zkontroluje ze je pole setrizene
    bool check()
    {
        for(int i = first; i < last - 1; i++)
        {
            if(data[i] > data[i + 1])
            {
                return false;
            }
        }
        return true;
    }
};

//vypise pole
void printArray(TYPE* pole, int pole_start, int pole_stop)
{
    if(vypis)
    {
        for (int i = pole_start; i < pole_stop; i++)
        {
            printf("%d\t", pole[i]);
        }
        printf("\n");
    }
}

                                                                                                    //PREDAT JESTE ODKUD KAM SE TO BUDE SORTIT V NOVEM POLI
void mergeArrays(TYPE* arr1, int arr1_start, int arr1_stop, TYPE* arr2, int arr2_start, int arr2_stop, TYPE* pole2)
{
    int i = arr1_start, j = arr2_start, k = 0;

    while (i < arr1_stop && j < arr2_stop)
    {
    	if (arr1[i] < arr2[j])
			pole2[k++] = arr1[i++];
		else
			pole2[k++] = arr2[j++];
    }

    while (i < arr1_stop)
    	pole2[k++] = arr1[i++];

    while (j < arr2_stop)
    	pole2[k++] = arr2[j++];
}

// Thread will sort array from element arg->first to arg->last.
// Result will be stored to arg->result*.
void *thread_sort(void *void_arg)
{
    task_part *ptr_task = (task_part*)void_arg;
    //printf("Thread %d sorting started from %d to %d...\n", ptr_task->id, ptr_task->first, ptr_task->last);

    //ptr_task->bubbleSort();
    ptr_task->selectionSort();
    //ptr_task->insertionSort();

    //printf("Sorted in thread %d:\n", ptr_task->id);
    printArray(ptr_task->data, ptr_task->first, ptr_task->last);
    
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

    for (int i = 0; i < N; i++)
    {
        pole[i] = rand() % (100);
    }
    printArray(pole, 0, N);

    //vytvoreni threadu a task_partu
    int th_count = 2;
    if(na == 3)
        th_count = atoi(arg[2]);
        
    /*pthread_t threads[th_count];
    std::vector<task_part> parts;
    for (int i = 0; i < th_count; ++i)
    {
        parts.push_back(task_part(i + 1, i * (N / th_count), (i + 1) * (N / th_count), pole));
    }
    printf("\nUsing %d threads...\n", th_count);*/

    pthread_t thread1, thread2;
    task_part part1(1, 0, N / 2, pole);
    task_part part2(2, N / 2, N, pole);

    timeval time_before, time_after, time_all_start, time_all_stop;

    gettimeofday(&time_all_start, NULL);

    printf("Sorting started...\n");
    pthread_create(&thread1, NULL, thread_sort, &part1);
    pthread_create(&thread2, NULL, thread_sort, &part2);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    if(part1.check() && part2.check())
    {
    	printf("sorted\n");
    }

    printArray(pole, 0, N);

    TYPE *pole2 = new TYPE[N];
    mergeArrays(part1.data, part1.first, part1.last, part2.data, part2.first, part2.last, pole2);
    printArray(pole2, 0, N);

    /*//FILLING
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
    printf("The filling time: %d [ms]\n", timeval_to_ms(&time_before, &time_after));*/

    /*//SORTING
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
    printf("The sorting time: %d [ms]\n", timeval_to_ms(&time_before, &time_after));*/

    /*//MERGE
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
    printf("The merging time: %d [ms]\n", timeval_to_ms(&time_before, &time_after));*/

    gettimeofday(&time_all_stop, NULL);
    printf("The time all: %d [ms]\n", timeval_to_ms(&time_all_start, &time_all_stop));
}






























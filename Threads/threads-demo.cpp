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
#define TYPE __uint32_t
//#define TYPE int

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
                if (data[j + 1] > data[j])
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
                if (data[minIndex] < data[j])
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
            while (j - first > 0 && tmp > data[j - 1])
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
            if(data[i] < data[i + 1])
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

void mergeArrays(TYPE* data, TYPE* result, int first1, int last1, int first2, int last2, int N)
{
    int i = 0, j = first2, k = 0;

    while (i < last1 && j < last2)
    {
    	if (data[i] > data[j])
			result[k++] = data[i++];
		else
			result[k++] = data[j++];
    }

    while (i < last1)
    	result[k++] = data[i++];

    while (j < last2)
    	result[k++] = data[j++];

    for(int a = last2; a < N; a++)
    	result[a] = data[a];
}

// Thread will sort array from element arg->first to arg->last.
void *thread_sort(void *void_arg)
{
    task_part *ptr_task = (task_part*)void_arg;
    //printf("Thread %d sorting started from %d to %d...\n", ptr_task->id, ptr_task->first, ptr_task->last);

    //ptr_task->bubbleSort();
    //ptr_task->selectionSort();
    ptr_task->insertionSort();

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
        printf("Specify number of elements and optionally number of threads.\n");
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

    pthread_t threads[th_count];
    std::vector<task_part> parts;
    for (int i = 0; i < th_count; i++)
    {
    	if(i == th_count - 1)
    	{
    		parts.push_back(task_part(i + 1, i * (N / th_count), (i + 1) * (N / th_count) + (N % th_count), pole));
    	}
    	else
    	{
    		parts.push_back(task_part(i + 1, i * (N / th_count), (i + 1) * (N / th_count), pole));
    	}
    }
    printf("\nUsing %d threads...\n", th_count);

    //SORT
    timeval time_before, time_after, time_all_start, time_all_stop;
    gettimeofday(&time_all_start, NULL);
    gettimeofday(&time_before, NULL);
    printf("\nSorting started...\n");
    for (int i = 0; i < th_count; i++)
    {
        pthread_create(&threads[i], NULL, thread_sort, &parts[i]);
    }
    for (int i = 0; i < th_count; ++i)
    {
        pthread_join(threads[i], NULL);
    }
    gettimeofday(&time_after, NULL);
    printf("The SORTING time: %d [ms]\n", timeval_to_ms(&time_before, &time_after));

    //MERGE
    gettimeofday(&time_before, NULL);
    TYPE *temporary = new TYPE[N];
	for(int i = 0; i < th_count - 1; i++)
	{
		mergeArrays(pole, temporary, 0, parts[i].last, parts[i + 1].first, parts[i + 1].last, N);

		TYPE* tmp = pole;
		pole = temporary;
		temporary = tmp;
	}
	gettimeofday(&time_after, NULL);
	printf("The MERGING time: %d [ms]\n", timeval_to_ms(&time_before, &time_after));

	printf("Result:\n");
	printArray(pole, 0, N);

    gettimeofday(&time_all_stop, NULL);
    printf("The time SUM: %d [ms]\n", timeval_to_ms(&time_all_start, &time_all_stop));
}






























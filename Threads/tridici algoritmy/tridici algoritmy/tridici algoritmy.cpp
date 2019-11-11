#include <iostream>
#include <time.h>

#define TYPE int

bool vypis = 1;

using namespace std;

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
		for (int i = first; i < last - 1; i++)
		{
			if (data[i] > data[i + 1])
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
	if (vypis)
	{
		for (int i = pole_start; i < pole_stop; i++)
		{
			printf("%d\t", pole[i]);
		}
		printf("\n");
	}
}

void mergeArrays(TYPE* arr1, int arr1_start, int arr1_stop, TYPE* arr2, int arr2_start, int arr2_stop, TYPE* pole2)
{
	int i = arr1_start, j = arr2_start, k = 0;

	while (i < arr1_stop && j < arr2_stop)
	{
		if (arr1[i] > arr2[j])
			pole2[k++] = arr1[i++];
		else
			pole2[k++] = arr2[j++];
	}

	while (i < arr1_stop)
		pole2[k++] = arr1[i++];

	while (j < arr2_stop)
		pole2[k++] = arr2[j++];
}

int main(void)
{
	const int N = 10;
	TYPE* pole = new TYPE[N];

	srand((unsigned int)time(nullptr));

	for (int i = 0; i < N; i++)
	{
		pole[i] = rand() % 100;
	}

	printArray(pole, N);
	insertionSort(pole, 0, N / 2);
	insertionSort(pole, N / 2, N);
	printArray(pole, N);
	if (check(pole, 0, N / 2))
		printf("sorted1\n");
	if (check(pole, N / 2, N))
		printf("sorted2\n");
	//mergeArrays(sorted1, sorted2, N / 2, N / 2);
	//printArray(sorted_final, N);

	return 0;
}

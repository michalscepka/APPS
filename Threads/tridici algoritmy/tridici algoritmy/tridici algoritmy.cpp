#include <iostream>
#include <time.h>

#define TYPE int

using namespace std;

class task_part
{
public:
	int id;                 // user identification
	int first, last, N;     // data range
	TYPE* data;             // array
	TYPE max = 0;           // result
	TYPE* result;   	    // result*

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
		cout << i << ":" << '\t';
	}
	cout << endl;

	for (int i = 0; i < N; i++)
	{
		printf("%d\t", pole[i]);
	}
	cout << endl;
	cout << endl;
}

TYPE* fillArray(TYPE* array, int first, int last, int length)
{
	TYPE* result = new TYPE[length];
	for (int i = 0; i < length; i++)
	{
		result[i] = array[first + i];
	}
	return result;
}

TYPE* bubbleSort(TYPE* array, int first, int last)
{
	int N = last - first;
	TYPE* result = fillArray(array, first, last, N);

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

TYPE* selectionSort(TYPE* array, int first, int last)
{
	int N = last - first;
	TYPE* result = fillArray(array, first, last, N);

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

TYPE* insertionSort(TYPE* array, int first, int last)
{
	int N = last - first;
	TYPE* result = fillArray(array, first, last, N);

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

int main(void)
{
	const int N = 1000000;
	TYPE* pole = new TYPE[N];
	//TYPE pole[N] = { 9.2, 4.5, 4.2, 2.3, 0.1, 0.002, 9.9, 2.31, 15.3, 11.12 };

	srand((unsigned int)time(nullptr));

	for (int i = 0; i < N; i++)
	{
		pole[i] = rand() % 100;
	}

	/*int pole1[] = { 1, 3, 5, 7, 9 };
	int pole2[] = { 2, 4, 8, 10, 11, 12, 16 };
	printArray(pole1, 5);
	printArray(pole2, 7);
	int* pole3 = mergeArrays(pole1, pole2, 5, 7);
	printArray(pole3, 5 + 7);*/

	/*printArray(pole, N);
	TYPE* sorted1 = bubbleSort(pole, 0, N / 2);
	TYPE* sorted2 = bubbleSort(pole, N / 2, N);
	printArray(sorted1, N / 2);
	printArray(sorted2, N / 2);
	TYPE* sorted_final = mergeArrays(sorted1, sorted2, N / 2, N / 2);
	printArray(sorted_final, N);*/

	//printArray(pole, N);
	TYPE* sorted1 = insertionSort(pole, 0, N / 2);
	TYPE* sorted2 = insertionSort(pole, N / 2, N);
	//printArray(sorted1, N / 2);
	//printArray(sorted2, N / 2);
	TYPE* sorted_final = mergeArrays(sorted1, sorted2, N / 2, N / 2);
	//printArray(sorted_final, N);

	return 0;
}

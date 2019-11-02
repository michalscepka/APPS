#include <iostream>
#include <time.h>


/**
 * Buble sort (od nejmensiho)
 * @param array pole k serazeni
 * @param from od ktereho indexu
 * @param length kolik indexu
 */
template <class T>
void bubbleSort(T* array, int from, int length)
{
	for (int i = 0; i < length - 1; i++)
	{
		for (int j = 0; j < length - i - 1; j++)
		{
			if (array[from + j + 1] < array[from + j])
			{
				T tmp = array[from + j + 1];
				array[from + j + 1] = array[from + j];
				array[from + j] = tmp;
			}
		}
	}
}

void selectSort(int* pole, int N)
{
	for (int i = 0; i < N - 1; i++)
	{
		int minIndex = i;

		for (int j = i + 1; j < N; j++)
		{
			if (pole[minIndex] > pole[j])
			{
				minIndex = j;
			}
		}
		std::swap(pole[minIndex], pole[i]);
	}
}

template <class T>
void print(T* pole, int N)
{
	for (int i = 0; i < N; i++)
	{
		std::cout << i << ":" << '\t';
	}
	std::cout << std::endl;

	for (int i = 0; i < N; i++)
	{
		std::cout << pole[i] << '\t';
	}
	std::cout << std::endl;
	std::cout << std::endl;
}

int main(void)
{
	const int N = 10;
	char* pole = new char[N];
	//double pole[N] = { 9.2, 4.5, 4.2, 2.3, 0.1 };

	srand((unsigned int)time(nullptr));

	for (int i = 0; i < N; i++)
	{
		pole[i] = 33 + rand() % 93;
	}

	print(pole, N);
	bubbleSort(pole, 0, N / 2);
	print(pole, N);
	bubbleSort(pole, N / 2, N - (N / 2));
	print(pole, N);

	return 0;
}

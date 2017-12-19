#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <string>
#include <Query.h>
#include <Windows.h>
#include <iomanip>
#include <limits>
#include <cstddef>
#undef max

using namespace std;

double PCFreq = 0.0;
__int64 CounterStart = 0;
int ** Dane;
int StartVertex = 0;
int Score = numeric_limits<int>::max();
int *BestSolution;
int Iters = 0;

int Size = 0;
int Tabu_time = 0;
int **Times_tab;
int BestScore = numeric_limits<int>::max();
int Counter = 0;
int Limit = 0; //limit dywersyfikiacji

int *currentSolution;
int ** tabuList;

void StartCounter()
{
	LARGE_INTEGER li;
	if (!QueryPerformanceFrequency(&li))
		cout << "QueryPerformanceFrequency failed!\n";

	PCFreq = double(li.QuadPart) / 1000.0;

	QueryPerformanceCounter(&li);
	CounterStart = li.QuadPart;
}
double GetCounter()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return double(li.QuadPart - CounterStart) / PCFreq;
}

void First()
{
	Dane = new int *[Size];
	for (int i = 0; i < Size; i++)
		Dane[i] = new int[Size];
}
void deleteAll()
{
	for (int i = 0; i < Size; i++)
	{
		delete[] Dane[i];
	}

	delete[] Dane;


	for (int i = 0; i < Size; i++)
	{
		delete[] tabuList[i];
	}
	delete[] tabuList;

	for (int i = 0; i < Size; i++)
	{
		delete[] Times_tab[i];
	}
	delete[] Times_tab;

	delete[] BestSolution;
	delete[] currentSolution;

}

void initTabs()
{
	currentSolution = new int[Size + 1];
	tabuList = new int*[Size];
	for (int i = 0; i < Size; ++i)
		tabuList[i] = new int[Size];

	Times_tab = new int*[Size];
	for (int i = 0; i < Size; ++i)
		Times_tab[i] = new int[Size];


	for (int i = 0; i < Size; i++)	//zerowanie tablic
	{
		for (int j = 0; j < Size; j++)
		{
			tabuList[i][j] = 0;
			Times_tab[i][j] = 0;
		}
	}

	for (int i = 0; i < Size; i++)	//inicjacja
	{
		currentSolution[Size - i] = i;
		//currentSolution[i] = i;
	}
	currentSolution[Size] = 0;

	currentSolution[0] = 0;
}

void OpenFile(string Name)
{
	string str;
	fstream Plik(Name);
	while (Plik.good() == 1)
	{
		Plik >> str;
		if (str == "DIMENSION:")
		{
			Plik >> Size;
			First();
		}
		if (str == "EDGE_WEIGHT_SECTION")
		{
			for (int i = 0; i < Size; i++)
			{
				for (int k = 0; k < Size; k++)
				{
					Plik >> (int)Dane[i][k];
				}

			}
		}
	}
	Plik.close();
	initTabs();
}



int * swap(int city1, int city2, int *solution) {
	int temp = solution[city1];
	solution[city1] = solution[city2];
	solution[city2] = temp;
	return solution;
}

int getCost(int *solution)
{
	int cost = 0;
	int a, b;
	for (int i = 0; i < Size; i++)
	{
		a = solution[i];
		b = solution[i + 1];
		cost += Dane[a][b];
	}
	return cost;
}

void GetCurrentPath(int * solution) {
	for (int i = 0; i < Size; i++) {
		cout << solution[i] << "->";
	}
	cout << solution[Size];
}

void GetRandomPath()
{
	int rand1, rand2;
	srand(static_cast <unsigned int> (time(NULL)));
	int *pomoc = new int[Size + 1];
	for (int i = 0; i < Size; i++)
	{
		rand1 = rand() % (Size - 1) + 1;
		rand2 = rand() % (Size - 1) + 1;
		currentSolution = swap(rand1, rand2, currentSolution);	//generuje n rozwiazan i wybieram najlepsze jako poczatkowe
		if (Score > getCost(currentSolution))
		{
			Score = getCost(currentSolution);
			memcpy(pomoc, currentSolution, sizeof(int)*(Size + 1));
		}

	}
	memcpy(currentSolution, pomoc, sizeof(int)*(Size + 1));
	delete[] pomoc;

	BestSolution = new int[Size + 1];
	memcpy(BestSolution, currentSolution, sizeof(int)*(Size + 1));	//pierwsze rozwiazanie jest aktualnie najlepsze

	BestScore = getCost(BestSolution);	//licze koszt
}

void GetStraightPath()
{
	for (int i = 0; i < Size; i++)	//inicjacja
	{
		currentSolution[Size - i] = i;
		//currentSolution[i] = i;
	}
	currentSolution[Size] = 0;


	currentSolution[0] = 0;
	BestSolution = new int[Size + 1];
	memcpy(BestSolution, currentSolution, sizeof(int)*(Size + 1));
	BestScore = getCost(BestSolution);
}

void tabuUpdate(int city1, int city2, int ** tabu)
{
	tabu[city1][city2] += Tabu_time;
	tabu[city2][city1] += Tabu_time;
}

void freqUpdate(int city1, int city2, int ** tabu)
{
	tabu[city1][city2] += 1;
	tabu[city2][city1] += 1;
}

void tabuDecrement(int ** tabu) {
	for (int i = 0; i<Size; i++) {
		for (int j = 0; j<Size; j++) {
			if (tabu[i][j] > 0)
			{
				tabu[i][j] -= 1;
			}
		}
	}
}


int * getBestNeighbour(int ** tabuList, int * initalSolution) {
	int * bestSolution = new int[Size + 1];
	memcpy(bestSolution, initalSolution, sizeof(int)*(Size + 1));

	int bestCost = getCost(initalSolution);
	int city1 = 0;
	int city2 = 0;
	bool firstNeighbor = true;

	for (int i = 1; i < Size; i++) {
		for (int j = i + 1; j < Size; j++)
		{
			if (i == j)
			{
				continue;
			}

			int * newBestSolution = new int[Size + 1];

			memcpy(newBestSolution, initalSolution, sizeof(int)*(Size + 1));
			newBestSolution = swap(i, j, newBestSolution);
			int newBestCost = getCost(newBestSolution);


			if (((newBestCost < bestCost || firstNeighbor) && tabuList[i][j] == 0) || (newBestCost < BestScore && (newBestCost < bestCost || firstNeighbor))) {
				firstNeighbor = false;
				city1 = i;
				city2 = j;
				memcpy(bestSolution, newBestSolution, sizeof(int)*(Size + 1));
				bestCost = newBestCost;
			}

			delete[] newBestSolution;
		}

	}

	if (city1 != 0)
	{
		tabuDecrement(tabuList);
		tabuUpdate(city1, city2, tabuList);
		freqUpdate(city1, city2, Times_tab);
	}
	return bestSolution;
}


void TS_TSP()
{
	for (int i = 0; i < Iters; i++)
	{
		if (Counter == Limit)
		{
			int city1 = 0;
			int city2 = 0;
			int value = numeric_limits<int>::max();
			for (int i = 1; i < Size; i++) {
				for (int j = i + 1; j < Size; j++) {
					if (i == j)
					{
						continue;
					}
					if (Times_tab[i][j] < value)
					{
						value = Times_tab[i][j];
						city1 = i;
						city2 = j;
					}
				}
			}



			currentSolution = swap(city1, city2, currentSolution);
			freqUpdate(city1, city2, Times_tab);

			Counter = 0;

		}
		else
		{
			currentSolution = getBestNeighbour(tabuList, currentSolution);
		}

		int currentCost = getCost(currentSolution);

		if (currentCost < BestScore) {
			cout << "\niteracja: " << setw(4) << left << i << " Best: " << setw(4) << left << BestScore << " Current: " << currentCost;
			memcpy(BestSolution, currentSolution, sizeof(int)*(Size + 1));
			BestScore = currentCost;
			Counter = 0;
			for (int i = 0; i < Size; i++)
			{
				for (int j = 0; j < Size; j++)
				{
					Times_tab[i][j] = 0;
				}
			}

		}
		else {
			Counter++;		//inkrementuje ilosc iteracji bez rozwiazania
		}

	}
	cout << endl;
}


int main()
{

	//cout << Score;
	/*cin.get();
	OpenFile("C:/Users/barto/Desktop/PEA2Pliki/ftv70.atsp");*/
	/*
	for (int i = 0; i < Size; i++)
	{
	for (int j = 0; j < Size; j++)
	{

	cout << setw(4) << Dane[i][j] << "  ";
	}
	cout << endl;
	}

	cin.get();*/

	//GetStraightPath();
	//cin.get();
	//GetCurrentPath(currentSolution);
	//cout << endl << BestScore << endl;
	//cin.get();
	//Tabu_time = 50;	//10
	//Iters = 100000;	//10000
	//Limit = Iters / 100;	//100
	//StartCounter();
	//TS_TSP();
	//cout<<"czas: "<<GetCounter();
	//cin.get();
	//GetCurrentPath(currentSolution);
	//cin.get();
	//cout << endl << BestScore << endl;

	cout << "\n\n1.ftv33.atsp\n2.ftv47.atsp\n3.ftv70.atsp\n4.fri26.tsp\n5.gr48.tsp\n6.gr120.tsp\n";
	int w;
	cin >> w;
	switch (w) {
	case 1: OpenFile("C:/Users/barto/Desktop/PEA2Pliki/ftv33.atsp");
		GetRandomPath();
		cout << "Poczatkowy Score: " << getCost(currentSolution) << endl;
		Tabu_time = 25;
		Iters = 10000;
		Limit = Iters / 10;
		StartCounter();
		TS_TSP();
		cout << "czas: " << GetCounter() << endl;
		cin.get();
		GetCurrentPath(currentSolution);
		cin.get();
		cout << endl << BestScore << endl;
		deleteAll();
		cin.get();
		return 0;
		break;
	case 2: OpenFile("C:/Users/barto/Desktop/PEA2Pliki/ftv47.atsp");
		GetRandomPath();
		cout << "Poczatkowy Score: " << getCost(BestSolution) << endl;
		Tabu_time = 40;
		Iters = 10000;
		Limit = Iters / 10;
		StartCounter();
		TS_TSP();
		cout << "czas: " << GetCounter() << endl;
		cin.get();
		GetCurrentPath(currentSolution);
		cin.get();
		cout << endl << BestScore << endl;
		deleteAll();
		cin.get();
		return 0;
		break;
	case 3: OpenFile("C:/Users/barto/Desktop/PEA2Pliki/ftv70.atsp");
		GetRandomPath();
		cout << "Poczatkowy Score: " << getCost(BestSolution) << endl;
		Tabu_time = 50;
		Iters = 100000;
		Limit = Iters / 100;
		StartCounter();
		TS_TSP();
		cout << "czas: " << GetCounter() << endl;
		cin.get();
		GetCurrentPath(currentSolution);
		cin.get();
		cout << endl << BestScore << endl;
		deleteAll();
		cin.get();
		return 0;
		break;
	case 4: OpenFile("C:/Users/barto/Desktop/PEA2Pliki/fri26.tsp");
		GetRandomPath();
		cout << "Poczatkowy Score: " << getCost(BestSolution) << endl;
		Tabu_time = 14;
		Iters = 500;
		Limit = Iters / 10;
		StartCounter();
		TS_TSP();
		cout << "czas: " << GetCounter() << endl;
		cin.get();
		GetCurrentPath(currentSolution);
		cin.get();
		cout << endl << BestScore << endl;
		deleteAll();
		cin.get();
		return 0;
		break;
	case 5: OpenFile("C:/Users/barto/Desktop/PEA2Pliki/gr48.tsp");
		GetRandomPath();
		cout << "Poczatkowy Score: " << getCost(BestSolution) << endl;
		Tabu_time = 40;
		Iters = 10000;
		Limit = Iters / 10;
		StartCounter();
		TS_TSP();
		cout << "czas: " << GetCounter() << endl;
		cin.get();
		GetCurrentPath(currentSolution);
		cin.get();
		cout << endl << BestScore << endl;
		deleteAll();
		cin.get();
		return 0;
		break;
	case 6: OpenFile("C:/Users/barto/Desktop/PEA2Pliki/gr120.tsp");
		GetRandomPath();
		cout << "Poczatkowy Score: " << getCost(BestSolution) << endl;
		Tabu_time = 110;
		Iters = 100000;
		Limit = Iters / 10;
		StartCounter();
		TS_TSP();
		cout << "czas: " << GetCounter() << endl;
		cin.get();
		GetCurrentPath(currentSolution);
		cin.get();
		cout << endl << BestScore << endl;
		deleteAll();
		cin.get();
		return 0;
		break;
	}




	deleteAll();

	cin.get();
	return 0;
}
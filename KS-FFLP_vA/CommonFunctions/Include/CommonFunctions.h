#ifndef _COMMONFUNCTIONS_	//TO AVOID THAT: The compiler parses it as being two definition of struct/class members
#define _COMMONFUNCTIONS_	//(as it will visit header.h twice and read the def of member struct).

#ifdef DLL_EXPORT_CF
#define EXPORT_CF __declspec(dllexport)
#else
#define EXPORT_CF __declspec(dllimport)
#endif

#pragma warning(disable:4996) //evitar warnings por templates de stl

#include <vector>
#include <list>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <time.h>
#include <algorithm>
#include <tuple>

#define   INITIAL_SEED       1			
#define   A1                  16807
#define   C1                  2836
#define   B1                  127773
#define   M1                  2147483647
#define   FLOAT_M            2147483647.0
const long max_rand = 1000000L;

static unsigned long int seedCF = INITIAL_SEED;

using namespace std;

namespace CommonFunctions{

	EXPORT_CF std::vector<std::string> SplitString(std::string str, const std::string &delim);
	EXPORT_CF std::string removeExtension(std::string sNameFile);
	EXPORT_CF std::string getFileExtension(std::string sNameFile);
	EXPORT_CF int getLengthFile(std::string sNameFile);
	EXPORT_CF unsigned long get_seed();
	EXPORT_CF std::vector<int> digits(int x);
	EXPORT_CF std::string itos(int x);
	EXPORT_CF void mergeVectorsAB(std::vector<tuple<int,int,double>> &A, const std::vector<tuple<int,int,double>> &B);

	//Matheuristics
	EXPORT_CF void quicksort_doublearray(double *&a,int inf, int sup);
	EXPORT_CF void floydWarshall(double **graph, int iDim);
	EXPORT_CF void orderVectorList(std::vector<tuple<int, int, double>> &vSavingsList, int iIni, int iFin, bool bIncrementOrder);
	EXPORT_CF void removeDuplicateTuples(vector<tuple<int,int,double>> &vComplete, vector<tuple<int,int,double>> &vBase);
	EXPORT_CF void DELETEMATRIXDOUBLE(double **&pdMatrix, int iDimension);
};
#endif
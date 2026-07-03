#include "MySources.h"
#include "SmartEnums.h"
#include<iostream>
#include <tuple>

using namespace std;
using namespace CommonFunctions;

#define EPS 0.0001
#define BIGNUM 1e50
typedef std::vector<tuple<int,int,double>> TUPLE3;


struct StrSolution{
	std::string sNameInst;
	int iKSIter;
	IloAlgorithm::Status sStatusOpt;
	double dSolutionValue;
	double dLowerBound;
	double dOptGap;
	double dComputingTime;
	int iKernelSize_ini;
	int iKernelSize_fin;
	int iBucketSize;
	int iNumSelBucket;
};

enum PROB_TYPE{
	PMEDIAN,		//P-Median problem (BILP) objective function: sum of distances from customers to their closest facility
	PCENTER,		//P-Center problem (BILP) objective function: maximum distance from a customer to its closest facility
	PMINIMAX,		//P-Minimax problem (BILP) objective function: maximum distance from a customer to its closest facility (cvar)
	PBIOBJ_CVAR,	//P-Minimax Bi-objective using epsilon constraint - impact especially on obj val and constraints
	PBIOBJ_PMED		//P-Minimax Bi-objective using epsilon constraint - impact especially on obj val and constraints
};

enum POINT_TYPE{
	NONE,
	IDEAL,		//P-Median problem (BILP)
	NADIR,		//P-Center problem (BILP)
	EFFICIENT	//P-Minimax Bi-objective using epsilon constraint
};

//Defining list of constraints definitions. 
//Takes a macro m as a parameter. Then it passes each one to m.
#define CONSTLIST(m)				\
	m(CONSTID, Objective)			\
    m(CONSTID, UniqueAssigment)		\
    m(CONSTID, NoAssignment)		\
    m(CONSTID, OpenFacilities)		\
	m(CONSTID, MaxDistance)			\
	m(CONSTID, MaxDistance2)		\
	m(CONSTID, MaxDistance3)		

class CKSCVAR{

	public:	
		CKSCVAR(std::string sNameFile);				// Construct a model of the class FPVRP
		~CKSCVAR(){Destroy();};

		void Destroy();

		//Original problems
		void solveFacilityLocationProb(PROB_TYPE tTypeProblemE, double dBeta, double dTimeLimit, int iApplyKS=0);

		//Kernel Search
		void applyKernelSearch(TUPLE3 &vCandidateList, int iKernelSizeY, int iBucketSizeY, double dTimeLimit, double &dMILPTime, double &dTotalTime, double &dBestUB);
		void fixKernel(std::vector<tuple<int,int,double>> vKernel, IloNumVarArray2D &x_E, IloNumVarArray &y_E);
		void FacilityLocationModel();
		void createKernelBucket(std::vector<tuple<int,int,double>> &vYsolutionRelax1, std::vector<tuple<int,int,double>> &vKernel, int iKernelSizeY, int iIsInitialKernel);

		IloAlgorithm::Status solveModel(IloCplex & cplex, double dMILPTime, double &dBestUB, double &dTotalTime, IloAlgorithm::Status &IloIncumbentStatus, int iIter=0, int iApplyKS=0);
		IloAlgorithm::Status setLPRelaxation(TUPLE3 &vSortedCandidateList, std::vector<tuple<int,int,double>> vYsolution, double &dTotalTime, double dMILPTime, bool bSetMILPValues, int &iKernelSize);
		
		void getSolutionVars(IloCplex cplexE);
		void saveSolution(IloCplex cplexE, int iKSIter, IloAlgorithm::Status iIloStatus, double dTotalTime, int iKernelElemIni);

		//Infeasibilites
		void evalFeasibility(IloCplex cplexAux);
		void printSolution();

		//Best solution
		double getBestSolutionValue();
		TUPLE3 getKernel();
		TUPLE3 getInitialRankedList();
		void getReducedCostsX(double **pdRedCostsE);
		int getNumCustomers(){return N;}

		void setEpsilon(double dEpsilonE){dEpsilon = dEpsilonE;}
		void setReducedCostsX(double **dRedCostsE);
		void setKernel(TUPLE3 vKernel_E) {vKernel = vKernel_E;}
		void setInitialRankedList(TUPLE3 vInitialRankedList_E){ vInitialRankedList = vInitialRankedList_E;}
		

	protected:

		IloEnv env;
		IloModel model;
		IloCplex cplex;
	
		IloNumVar Z;			//Maximum distance from a customer to its closest facility - For Objective function
		IloNumVarArray y, u;	//Facility j
		IloNumVarArray2D x;		//x= Arc traverse from customer i to facility j, u = for objective function pmedian(beta)

		std::string sNameOfInstance;
		std::vector<StrSolution> vSolutionPool;
		TUPLE3 vBestSolution;
		std::vector<double> vBestSolutionMultipliers;

		//Kernel Search
		TUPLE3	vInitialRankedList,
				vSelectedItems, 
				vSolution, 
				vArcsSelectedBucket, 
				vKernel;

		//Parameters ============================================
		int N;				//	Number of nodes/customers
		int P;				//	Number of facilities
		int Pmax;			//	Number of potencial facilities
		double **dDist;		//Distance matrix
		double dBeta;
		PROB_TYPE tTypeProblem;

		//LP solution
		double **dRedCostsX;		//Distance matrix

		//Best solution 
		double dBestSolutionValue;
		double dEpsilon;

};

//Funtions that are not part of the class
void applyComparisonSolutions(std::string sNameFile, std::string sNameFileOut);


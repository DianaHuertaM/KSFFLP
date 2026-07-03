#include "KSCVAR.h"


int main(int argc, char **argv){

	//Directly from terminal, use: KSVAR.exe pmed3.txt 2 0.10 7200 1 
	//Directly from Visual Studio: pmed3.txt 2 0.10 7200 1

	if (argc == 6) { //Apply solution method exact/KS

		//Read Parameters //example: pmed3.txt 2 0.10 7200 1
		std::string sNameFile = string(argv[1]);	//Input file
		int iProblemType = stoi(argv[2]);			//PROB_TYPE: 0-PMEDIAN, 1-PCENTER, 2-PBETA...
		double dBeta = stod(argv[3]);				//Beta value [0-1]
		double dMaxTime = stod(argv[4]);			//Max time of optimization (seconds)
		int iAppliedMethod = stoi(argv[5]);			//0- (Exact), 1- (create Kernel/Buckets + apply KS), 2- TEST (NON create Kernel/Bucket + apply KS) 
		//Note: Bucket size = Kernel size

		//Create a CKSCVAR instance 
		CKSCVAR myProblem1(sNameFile);

		//If exact or kernel search
		if (iAppliedMethod == 0 || iAppliedMethod == 1) { //Main solution method

			myProblem1.solveFacilityLocationProb((enum PROB_TYPE)iProblemType, dBeta, dMaxTime, iAppliedMethod);

		}else if (iAppliedMethod == 2) { //Extra Test

			//TESTS-DH  =======================================================================
			//Create kernel and buckets iun another way
			
			//=========================================================//
			//First - Apply exact or KS to obtain ideal/nadir points
			//=========================================================//
			iAppliedMethod = 0; //or iAppliedMethod = 1; 
			CKSCVAR idealPointX(sNameFile), idealPointY(sNameFile),
					nadirPointX(sNameFile), nadirPointY(sNameFile);

			int iEP = 10; // e-constraint problems generated
			double dStep = 1e5;
		
			//Ideal Points
			idealPointX.solveFacilityLocationProb(PBIOBJ_CVAR, dBeta, dMaxTime*0.25, iAppliedMethod);
			double dPXideal = idealPointX.getBestSolutionValue();

			idealPointY.solveFacilityLocationProb(PBIOBJ_PMED, dBeta, dMaxTime*0.25, iAppliedMethod);
			double dPYideal = idealPointY.getBestSolutionValue();

			//Nadir Points
			nadirPointX.setEpsilon(dPYideal);
			nadirPointX.solveFacilityLocationProb(PBIOBJ_CVAR, dBeta, dMaxTime*0.25, iAppliedMethod);
			double dPXnadir = nadirPointX.getBestSolutionValue();

			//=========================================================//
			//Second - Obtain kernel and sorted list of elements and matrix of reduced costs
			//=========================================================//
			TUPLE3 prevKernel = nadirPointX.getKernel();
			TUPLE3 prevSortedList = nadirPointX.getInitialRankedList();
			double **dReducedCostsX = NULL;
			nadirPointX.getReducedCostsX(dReducedCostsX); 
			int iNumCustomers = nadirPointX.getNumCustomers();

			nadirPointY.setEpsilon(dPXideal);
			nadirPointY.solveFacilityLocationProb(PBIOBJ_PMED, dBeta, dMaxTime*0.25, iAppliedMethod);
			double dPYnadir = nadirPointY.getBestSolutionValue();
		

			cout<<"(ideal,nadir) points: ("<<dPXideal<<","<<dPYnadir<<")  and  ("<<dPYideal<<","<<dPXnadir<<")"<<endl;		
			dStep = (dPXnadir - dPXideal)/(iEP + 1);


			//=========================================================//
			//Third - Apply KS using the previous kernel
			//=========================================================//
			iAppliedMethod = 2;

			double dEpsilon = dPYideal + dStep;
			CKSCVAR efficientPoint(sNameFile);

			for(int i=0; i < iEP; i++){
			
				efficientPoint.setEpsilon(dEpsilon);
				efficientPoint.setReducedCostsX(dReducedCostsX);
				efficientPoint.setKernel(prevKernel);
				efficientPoint.setInitialRankedList(prevSortedList);

				efficientPoint.solveFacilityLocationProb(PBIOBJ_CVAR, dBeta, dMaxTime*0.25, iAppliedMethod);

				double dPefficient = efficientPoint.getBestSolutionValue();
				cout<<"Efficient point ["<<i+1<<"] = ("<<dPYideal + dStep<<","<<dPefficient<<")"<<endl;

				//Update the new kernel
				prevKernel = efficientPoint.getKernel();
				dEpsilon = efficientPoint.getBestSolutionValue() + dStep;

				efficientPoint.Destroy();

			}

			DELETEMATRIXDOUBLE(dReducedCostsX, iNumCustomers);			
			}

		cout << "== End of the optimization ==" << endl;

	}else if(argc == 3){ //Get evaluation of solution

		std::string sNameFile = string(argv[1]);	//Solution.txt
		std::string sNameFileOut = string(argv[2]);	//Output.txt
		applyComparisonSolutions(sNameFile, sNameFileOut);

	}else {
		//Wrong number of parameters, display valid options
		cout << "Wrong number of parameters. Please, use one of the following options: " << endl;
		//for exact method, use 3 arguments: ===================================
		cout << "1) For applying exact method (CPLEX): " << endl;
		cout << "\t\t./KSCVAR <input_file> <problem_type> <beta_value> <max_time> <apply_KS>" << endl;
		cout << "\t\t\t- problem_type: 0-PMEDIAN, 1-PCENTER, 2-PBETA" << endl;
		cout << "\t\t\t- apply_KS: 0- (non apply KS), 1- (create Kernel/Buckets + apply KS), 2- (NON create Kernel/Bucket + apply KS)" << endl;
		//For kernel search, use 7 arguments: ==================================	
		cout << "2) For applying kernel search: " << endl; 
		//example: pmed3.txt 2 0.10 7200 1 15 0.05
		cout << "\t\t./KSCVAR <input_file> <problem_type> <beta_value> <max_time> <apply_KS> <bucket_size> <percentage_iterations_KS>" << endl; 
		//Input file
		cout << "\t\t\t- Param2 <input_file>: File with the problem data (ex. pmed3.txt)" << endl;
		//PROB_TYPE: 0-PMEDIAN, 1-PCENTER, 2-PBETA...
		cout << "\t\t\t- Param3 <problem_type>: 0-PMEDIAN, 1-PCENTER, 2-PBETA, 3-PBIOBJ_CVAR, 4-PBIOBJ_PMED" << endl;
		//Beta value
		cout << "\t\t\t- Param4 <beta_value>: Value of the beta parameter for the problem" << endl;
		//Max time of optimization
		cout << "\t\t\t- Param5 <max_time>: Maximum time allowed for the optimization process (Seconds)" << endl;
		//Apply exact, ks or ks without creating kernel/bucket
		cout << "\t\t\t- Param6 <apply_KS>: 0- (Exact), 1- (create Kernel/Buckets + apply KS), 2-(NON create Kernel/Bucket + apply KS)" << endl; //Beta value
		//Size of bucket of Y vars (facilities)
		cout << "\t\t\t- Param7 <bucket_size>: Size of the bucket of Y variables (facilities) to be created in each iteration of the KS" << endl; 
		//Maximum number of iterations for the KS
		cout << "\t\t\t- Param8 <percentage_iterations_KS>: Maximum percentage of iterations for applying the KS" << endl;

	}
	
	cin.get();
	return 0;
}


///////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	applyComparisonSolutions()
//	Developed by	:	Diana Lucia Huerta Munoz		     
//	Description		:   This function make the comparison between problem solutions
//	Date			:   Jan 2019
///////////////////////////////////////////////////////////////////////////////////
void applyComparisonSolutions(std::string sNameFile, std::string sNameFileOut){

	ifstream input(sNameFile);

	cout<<"Reading solution file..."<<sNameFile<<endl;
	bool bStart = false;
	std::string sLine ="";
	TUPLE3 vBestSolution;
	while(!input.eof()){

		//start to read the file once we are in the line of distances-assignment
		getline(input, sLine);

		if(sLine.find("Multipliers") != std::string::npos || sLine.find("Total") != std::string::npos){
			bStart = false;
		}

		if(bStart){
			std::vector<string> vVector = SplitString(sLine,"\t");
			//cout<<"Line = "<<stoi(vVector[0])<<", "<<stoi(vVector[1])<<", "<<stod(vVector[2])<<endl;
			vBestSolution.push_back(make_tuple(stoi(vVector[0]), stoi(vVector[1]), stod(vVector[2])));
		}

		if(sLine.find("Distance") != std::string::npos){
			bStart = true; //Next iteration the line is obtained
		}
			
	}

	input.close();

	//Here the vector must be order in decreasing order
	bStart = false;
	if(vBestSolution.size() > 0){
		orderVectorList(vBestSolution, 0, (int)vBestSolution.size()-1, true);

		int N = (int)vBestSolution.size();
		double dBeta1 = 0.05, dBeta2 = 0.10;
		int K1 = (int)ceil((double)(dBeta1*N));
		int K2 = (int)ceil((double)(dBeta2*N));
		double dDK1 = std::get<2>(vBestSolution.at(K1));
		double dDK2 = std::get<2>(vBestSolution.at(K2));

		if(CommonFunctions::getLengthFile(sNameFileOut) == 0){
			ofstream output(sNameFileOut, ios::app);
			output	<<"Instance\t"
					<<"Pmedian\t"
					<<"Pcenter\t"
					<<"PBeta5\t"
					<<"PBeta10\n";
			output.close();
		}

		cout<<"Writing comparison file... "<<sNameFileOut<<endl;
		double dPmedianSol =0.0;
		double dPCenterSol =0.0;
		double dPBeta5Sol =dDK1;
		double dPBeta10Sol =dDK2;
		for(TUPLE3::iterator it=vBestSolution.begin(); it!=vBestSolution.end(); it++){
			dPmedianSol += std::get<2>(*it);
			if(it <= vBestSolution.begin() + K1){
				dPBeta5Sol+= max(((std::get<2>(*it) - dDK1)/(double)K1),0.0);
			}
			if(it <= vBestSolution.begin() + K2){
				dPBeta10Sol+= max(((std::get<2>(*it) - dDK2)/(double)K2),0.0);
			}
		}
		dPCenterSol = std::get<2>(*vBestSolution.begin());
		
		ofstream output(sNameFileOut, ios::app);
		output<<sNameFile<<"\t"<<dPmedianSol<<"\t"<<dPCenterSol<<"\t"<<dPBeta5Sol<<"\t"<<dPBeta10Sol<<endl;
		output.close();

	}
	
	return;
}
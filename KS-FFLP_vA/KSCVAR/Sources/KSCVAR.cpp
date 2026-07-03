#include "KSCVAR.h"


////Takes the enum name and the list of values as parameters and 
////defines an enum for us. It also appends the total count.
SMARTENUM_DEFINE_ENUM(CONSTID, CONSTLIST)
////Accesses the array at the right position to return the 
////corresponding string literal for a given enum value.
SMARTENUM_DEFINE_NAMES(CONSTID, CONSTLIST)
SMARTENUM_DEFINE_GET_VALUE_FROM_STRING(CONSTID, CONSTLIST)

///////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	CKSCVAR()
//	Developed by	:	Diana Lucia Huerta Munoz		     
//	Description		:   Class constructor for CVAR Kernel Search implementation
//	Date			:   Dic 2018
///////////////////////////////////////////////////////////////////////////////////
CKSCVAR::CKSCVAR(std::string sNameFile_E):model(env), cplex(env){

	//Default Parameters ============================================
	N = 0;					//	Number of nodes/customers
	P = 0;					//	Number of facilities
	Pmax = 0;				//	Number of potencial facilities
	dDist = NULL;			//Distance matrix
	dBeta = 1;				//Parameter for p-minimax problem (cvar), deafault value is 1, which corresponds to p-median problem
	tTypeProblem = PMEDIAN;	//p-median
	sNameOfInstance = removeExtension(sNameFile_E);

	//Read data and initialize parameters
	std::ifstream InputFile(sNameFile_E);
	int iNumNodes=0, iNumEdges = 0;

	if(!InputFile.good()){
		cout<<"File does not exists or is corrupted."<<endl;
		exit(0);
	}

	//Reading input file ====================================
	cout<<"Reading input file..."<<endl;
	InputFile>>N;
	InputFile>>iNumEdges;
	InputFile>>P;

	Pmax = N;

	//---Initialize matrix of distances
	dDist = new double*[N];
	for(int i=0; i<N; i++){
		dDist[i] = new double[Pmax];
		for(int j=0; j<Pmax; j++){
			if(i == j){
				dDist[i][j] = 0.0;
			}else{
				dDist[i][j] = 1e35;
			}
		}
	}

	//Reduced costs matrix
	dRedCostsX = new double*[N];
	for(int i= 0; i<N; i++){
		dRedCostsX[i] = new double[Pmax];
		for(int p= 0; p<Pmax; p++){
			dRedCostsX[i][p] = 0.0;
		}
	}

	//Replace cost of edges for those in the input file
	int a = 0, b = 0;
	double dEdgeCost = 0.0;
	for(int e = 0; e < iNumEdges; e++){
		InputFile>>a;
		InputFile>>b;
		InputFile>>dEdgeCost;
		dDist[a-1][b-1] = dEdgeCost;
		dDist[b-1][a-1] = dDist[a-1][b-1];
	}

	//apply Floyd algorithm to obtain the shortest path between all pairs of nodes, 
	//in case the input file does not have the direct distance from each node to each potential facility
	cout << "Applying Floyd Algorithm to initial matrix" << endl;
	floydWarshall(dDist, N);

	InputFile.close();

	//Define decision variables
	Z = IloNumVar(env, 0, IloInfinity, ILOFLOAT);
	y = IloNumVarArray(env, Pmax, 0, 1, ILOBOOL);
	u = IloNumVarArray(env, N, 0, IloInfinity, ILOFLOAT);
	CreateArray2D(x, env, N, Pmax, ILOBOOL);

	//Best solution 
	dBestSolutionValue = 1e50;
	dEpsilon = 1e50; 

	return;
}


///////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	~CKSCVAR()
//	Developed by	:	Diana Lucia Huerta Munoz		     
//	Description		:   Destructor for CVAR Kernel Search class
//	Date			:   Dic 2018
///////////////////////////////////////////////////////////////////////////////////
void CKSCVAR::Destroy(){

	for(int i=0; i<N; i++){
		delete [] dRedCostsX[i];
		delete [] dDist[i];
	}
	delete [] dRedCostsX;
	delete [] dDist;

	model.end();
	cplex.end();
	env.end();

	return;
}


///////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	saveSolution()
//	Developed by	:	Diana Lucia Huerta Munoz		     
//	Description		:   Save the information of a given KS iteration
//	Date			:   Dic 2018
///////////////////////////////////////////////////////////////////////////////////
void CKSCVAR::saveSolution(IloCplex cplexE, int iKSIter, IloAlgorithm::Status iIloStatus, double dTotalTime, int iKernelElemIni){

	StrSolution strSolution;				//For solution data

	strSolution.sNameInst = sNameOfInstance;
	strSolution.iKSIter = iKSIter;
	strSolution.sStatusOpt = iIloStatus;
	strSolution.dSolutionValue = (double)cplexE.getObjValue();
	strSolution.dLowerBound = (double)cplexE.getBestObjValue();
	strSolution.dOptGap = (double)cplexE.getMIPRelativeGap() * 100;
	strSolution.dComputingTime = dTotalTime;
	strSolution.iKernelSize_ini = iKernelElemIni;		//To have a feasible solution at least P facilities must be included in Kernel
	strSolution.iKernelSize_fin = (int)vKernel.size();
	strSolution.iBucketSize = (int)vArcsSelectedBucket.size();
	strSolution.iNumSelBucket = (int)vArcsSelectedBucket.size() - (int)vSelectedItems.size();

	vSolutionPool.push_back(strSolution);				

	cout<<iKSIter<<"\t"<<iIloStatus<<"\t"<<cplexE.getObjValue()<<"\t"
		<<(double)cplexE.getBestObjValue()<<"\t"<<cplexE.getMIPRelativeGap() * 100
		<<"\t"<<dTotalTime<<"\t"<<iKernelElemIni<<"\t"<<vKernel.size()
		<<"\t"<<(int)vArcsSelectedBucket.size() - (int)vSelectedItems.size()<<endl;
	
	return;

}

///////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	solveFacilityLocationProb()
//	Developed by	:	Diana Lucia Huerta Munoz		     
//	Description		:   Function to solve KS 
//	Date			:   Dic 2018
///////////////////////////////////////////////////////////////////////////////////
void CKSCVAR::solveFacilityLocationProb(PROB_TYPE tTypeProblemE, double dBetaE, double dTimeLimit, int iApplyKS){

	//Local parameters =============================================	
	tTypeProblem = tTypeProblemE;
	dBeta = dBetaE;
	double	dBestUB = 1e35,					//For minimization problem
			dTime = 0.0, dTotalTime = 0.0,	// Times for KS
			dMILPTime = dTimeLimit;			// The first MILP to solve will last at most 10 mins, then it changes to each KS MILP time 
	
	clock_t cTime = 0;
	IloAlgorithm::Status IloIncumbentStatus =  IloAlgorithm::Status::Unknown, 
						 iIloStatus = IloIncumbentStatus;

	int iKernelSizeY = P;		//For Facility Location Problems Kernel Size AT LEAST P

	int iKernelElemIni = 0;	
	int iKSIter= 0;

	
	try{
		//Define the model
		FacilityLocationModel();

		cout<<"Optimizing..."<<endl;
		cout<<"KSIter\tStatus\tBUB\tSolution\tGAP\tTime\tKIni\tKFin\tBucket"<<endl;

		//If we are not applying KS, just solve the MILP and get the solution, otherwise apply KS
		if(iApplyKS == 0){

			//Solve MILP
			cplex.extract(model);	
			iIloStatus = solveModel(cplex, dMILPTime, dBestUB, dTotalTime, IloIncumbentStatus, 0, iApplyKS);
			
			if(iIloStatus == IloAlgorithm::Status::Optimal || iIloStatus == IloAlgorithm::Status::Feasible){
				
				//addSolution =======================================================
				cout<<"Saving MILP solution... "<<endl;
				saveSolution(cplex, iKSIter, iIloStatus, dTotalTime, iKernelElemIni);
				cout<<"Getting initial MILP solution... "<<endl;
				//getSolutionVars(cplex);
			
				//If solution is optimal and integer, STOP, this is the optimal of the MILP
				printSolution();
				cout<<"Best found... ending Optimization"<<endl;
			}else{
				////Infeasible solution
				cout<<iKSIter<<"\t"<<iIloStatus<<"\t"<<dBestUB<<"\t"
				<<"---\t"<<cplex.getMIPRelativeGap() * 100
				<<"\t"<<dTotalTime<<"\t"<<iKernelElemIni<<"\t"<<vKernel.size()
				<<"\t"<<(int)vArcsSelectedBucket.size() - (int)vSelectedItems.size()<<endl;
			}

		}else if(iApplyKS >= 1){ 
			
			//==========================================	KS	============================================//
			TUPLE3 vCandidateList;	
			std::vector<tuple<int,int,double>> vYsolution; //<node, solution value, reduced costs>

			if (iApplyKS == 1) {// [1] Solving Linear Relaxion to obtain kernel ==============================================

				//Solve the linear relaxation directly 
				iIloStatus = setLPRelaxation(vCandidateList, vYsolution, dTotalTime, dMILPTime*0.75, false, iKernelSizeY);

			}else{
				//Otherwise: we are using a predefined kernel and buckets previously created in another way (both are sorted)
				removeDuplicateTuples(vInitialRankedList, vKernel);	//Remove the kernel items from the complete list to obtain buckets and size
				vCandidateList = vInitialRankedList;
			}
			
			//If relaxation is feasible or optimal or we have used a predefined kernel, then apply KS, otherwise the problem is infeasible and we cannot apply KS
			if(iIloStatus == IloAlgorithm::Status::Optimal || iIloStatus == IloAlgorithm::Status::Feasible || iApplyKS == 2){
				
				dMILPTime = max(0.0, (dTimeLimit - dTotalTime));
				
				//The bucket size will be the same as the kernel size
				cout<<"Initial K-Facilities Size = "<<iKernelSizeY<<endl;
				int iBucketSizeY = iKernelSizeY;
				applyKernelSearch(vCandidateList, iKernelSizeY, iBucketSizeY, dTimeLimit, dMILPTime, dTotalTime, dBestUB);
				
				//Print the best solution found at the end of the optimization
				printSolution();
			}
		}

	}catch (IloException& e) {
		cerr << "Concert exception caught: " << e << endl;
	}catch (...) {
		cerr << "Unknown exception caught" << endl;
	}

	return;
}

///////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	applyKernelSearch()
//	Developed by	:	Diana Lucia Huerta Munoz		     
//	Description		:   Kernel Search algorithm
//	Date			:   Dic 2018
///////////////////////////////////////////////////////////////////////////////////
void CKSCVAR::applyKernelSearch(TUPLE3 &vCandidateList, int iKernelSizeY, int iBucketSizeY, double dTimeLimit, double &dMILPTime, double &dTotalTime, double &dBestUB){

	IloConstraintArray IloConst(env);		//For restricting the main MILP
	IloAlgorithm::Status IloIncumbentStatus =  IloAlgorithm::Status::Unknown,
						 IloStatus = IloAlgorithm::Status::Unknown;

	int iKSIterMax = (int)ceil((Pmax-iKernelSizeY)/(double)iBucketSizeY) + 1,
		iKSIter = 0;

	//Create Kernel and Bucket lists =====================================
	if(vKernel.size()==0){
		createKernelBucket(vCandidateList, vKernel, iKernelSizeY, 1);
	}

	int iKernelElemIni = (int)vKernel.size();

	//Here solve the MILP only with the initial Kernel
	IloModel modelKS(env); 
	modelKS.add(model);	

	//Just in case I will make the convertion to bool again
	for(int i = 0; i<N; i++){
		modelKS.add(IloConversion(env, x[i], ILOBOOL));
	}modelKS.add(IloConversion(env, y, ILOBOOL));

	//Here it starts the Kernel Search =====================================
	//At least give 60 sec to solve the first iteration of the KS
	dMILPTime = max(60.0, ceil((dMILPTime - dTotalTime)/iKSIterMax)); //Compute time for each restricted MILP
		
	while(iKSIter < iKSIterMax){

		if(iKSIter == 0){
			fixKernel(vKernel, x, y);
		}else{

			//Here are added the buckets
			vArcsSelectedBucket.resize(0);
			createKernelBucket(vCandidateList, vSelectedItems, iBucketSizeY, 0);

			if(vSelectedItems.size() == 0){
				iKSIter = iKSIterMax;
				break; // could be applied? - TEST
			}

			mergeVectorsAB(vKernel, vSelectedItems); //union(A + B)
			fixKernel(vKernel, x, y);
			vArcsSelectedBucket = vSelectedItems;								
					
			////========== ADDITIONAL SET OF CONSTRAINTS ==========////
			//In this part we need to add a new set of constraints,
			//First remove any other constraint in the model
			if(IloConst.getSize()>0){
				for (int i = 0; i < IloConst.getSize(); i++){
					modelKS.remove(IloConst[i]);
				}
				IloConst.clear();
			}

			bool bIsConstraint = false;
			IloExpr InvEqL(env);
			if(IloIncumbentStatus == IloAlgorithm::Status::Optimal){
				//cout<<"Adding OPTIMALITY constraint... "<<endl;
				for(auto it = vArcsSelectedBucket.begin(); it != vArcsSelectedBucket.end(); it++){
					InvEqL += x[std::get<0>(*it)][std::get<1>(*it)]; 
					bIsConstraint = true;
				}
				if(bIsConstraint){
					IloConstraint cons = InvEqL >= 1;
					modelKS.add(cons).setName("NewC");
					IloConst.add(cons);
				}

			}else if(IloIncumbentStatus == IloAlgorithm::Status::Feasible){

				//cout<<"Adding FEASIBILITY constraint... "<<endl;
				//Arcs in kernel
				for(auto it = vKernel.begin(); it != vKernel.end(); it++){
					InvEqL += x[std::get<0>(*it)][std::get<1>(*it)];
					bIsConstraint = true;
				}
				//Minus Arcs in solution
				for(auto it = vSolution.begin(); it != vSolution.end(); it++){
					InvEqL -= x[std::get<0>(*it)][std::get<1>(*it)];
					bIsConstraint = true;
				}					

				//if(bIsConstraint) model.add(InvEqL >= 1);
				if(bIsConstraint){
					IloConstraint cons = InvEqL >= 1;
					modelKS.add(cons).setName("NewC");
					IloConst.add(cons);
				}		
			}

			InvEqL.end();
		}

		////========== ADDITIONAL SET OF CONSTRAINTS ==========////
		//Solving model ====================================
		IloCplex cplexKS(modelKS);
		cplexKS.extract(modelKS);
		IloStatus = solveModel(cplexKS, dMILPTime, dBestUB, dTotalTime, IloIncumbentStatus, iKSIter+1, 1);

		if (IloStatus == IloAlgorithm::Status::Optimal || IloStatus == IloAlgorithm::Status::Feasible) { //if the problem is feasible, then get the solution and update the best upper bound

			dBestUB = cplexKS.getObjValue();

			//Get X and Y variables from the new solution
			vSolution.resize(0);
			for(int i= 0; i<N; i++){
				for(int p= 0; p<Pmax; p++){
					if(cplexKS.getValue(x[i][p]) > 1 - EPS){
						vSolution.push_back(make_tuple(i,p,cplexKS.getValue(x[i][p])));
					}
				}
			}

			//Keep only values from Bucket that were selected in the final solution
			removeDuplicateTuples(vSelectedItems, vSolution); //Remove from selected bucket the items in solution
			removeDuplicateTuples(vKernel, vSelectedItems);	//Remove the non-selected items from kernel

			//addSolution =======================================================
			saveSolution(cplexKS, iKSIter+1, IloStatus, dTotalTime, iKernelElemIni);
			getSolutionVars(cplexKS);

		}else{
			
			//If the problem is infeasible, then we need to update the kernel and the buckets for the next iteration
			//and recompute the time for the next MILP to solve considering the time already spent and the remaining iterations to do
			removeDuplicateTuples(vKernel, vSelectedItems);
			dMILPTime = max(0.0, ceil((dTimeLimit - dTotalTime)/(iKSIterMax - iKSIter + 1)));
			if(dMILPTime < EPS){
				cout<<"MILP time is too small -> end optimization."<<endl;	
				iKSIter = iKSIterMax;
			}
	
			//Infeasible - Print in terminal
			cout<<iKSIter+1<<"\t"<<IloStatus<<"\t"<<dBestUB<<"\t"
			<<"---\t"<<"---\t"
			<<"\t"<<dTotalTime<<"\t"<<iKernelElemIni<<"\t"<<vKernel.size()
			<<"\t"<<(int)vArcsSelectedBucket.size() - (int)vSelectedItems.size()<<endl;

		}
		++iKSIter;
		cplexKS.end();
	}
	modelKS.end();
	IloConst.end();
				
	return;

}

///////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	getSolutionVars()
//	Developed by	:	Diana Lucia Huerta Munoz		     
//	Description		:   Obtain the solution values of the variables
//	Date			:   Dic 2018
///////////////////////////////////////////////////////////////////////////////////
void CKSCVAR::getSolutionVars(IloCplex cplexE){

	vBestSolution.resize(0);

	//First P places are for facilities
	for(int p=0; p < Pmax; p++){
		if(cplexE.getValue(y[p]) >= 0.5){
			vBestSolution.push_back(make_tuple(p,-1,0));
		}
	}

	//The following are for assignment
	for(int p=0; p < Pmax; p++){
		for(int i=0; i < N; i++){		
			if(cplexE.getValue(x[i][p]) >= 0.5){
				vBestSolution.push_back(make_tuple(i, p, dDist[i][p]));
			}
		}
	}

	if(tTypeProblem == PMINIMAX){
		vBestSolutionMultipliers.resize(0);
		for(int i=0; i < N; i++){
			vBestSolutionMultipliers.push_back(cplexE.getValue(u[i]));
		}
	}

	return;
}

///////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	printSolution()
//	Developed by	:	Diana Lucia Huerta Munoz		     
//	Description		:   
//	Date			:   Dic 2018
///////////////////////////////////////////////////////////////////////////////////
void CKSCVAR::printSolution(){

	//=========================================================================//
	//				First print all the obtained solutions					   //
	//=========================================================================//
	string sNameFO = sNameOfInstance + "_Results.txt";
	if(CommonFunctions::getLengthFile(sNameFO) == 0){
		ofstream output(sNameFO, ios::app);
		output	<<"Instance\t"
				<<"KSIter\t"
				<<"Status\t"
				<<"BestSol\t"
				<<"BestLB\t"
				<<"MIPGap\t"
				<<"AccumTime\t"
				<<"KernelIni\t"
				<<"KernelFin\t"
				<<"BucketSize\t"
				<<"Selected\n";
		output.close();
	}
	ofstream output(sNameFO, ios::app);

	int iTotalItemsBucket = 0;
	for(std::vector<StrSolution>::iterator Sol = vSolutionPool.begin(); Sol != vSolutionPool.end(); Sol++){
		output<<"\n"<<Sol->sNameInst<<"\t"
				  <<Sol->iKSIter<<"\t"
				  <<Sol->sStatusOpt<<"\t"
				  <<Sol->dSolutionValue<<"\t"
				  <<Sol->dLowerBound<<"\t"
				  <<Sol->dOptGap<<"\t"
				  <<Sol->dComputingTime<<"\t"
				  <<Sol->iKernelSize_ini<<"\t"
				  <<Sol->iKernelSize_fin<<"\t"
				  <<Sol->iBucketSize<<"\t"
				  <<Sol->iNumSelBucket<<"\t";

		//compute selected_items
		iTotalItemsBucket += Sol->iNumSelBucket;

	}	
	output.close();
	
	//=========================================================================//
	//					Print solution BEST solution						   //
	//=========================================================================//
	sNameFO = "Final_Results.txt";
	if(CommonFunctions::getLengthFile(sNameFO) == 0){
		ofstream output(sNameFO, ios::app);
		output	<<"Instance\t"
				<<"KSIter\t"
				<<"Status\t"
				<<"BestSol\t"
				<<"BestLB\t"
				<<"MIPGap\t"
				<<"AccumTime\t"
				<<"KernelIni\t"
				<<"KernelFin\t"
				<<"BucketSize\t"
				<<"Selected\n";
		output.close();
	}

	ofstream outputBest(sNameFO, ios::app);
	std::vector<StrSolution>::iterator Sol = vSolutionPool.end() - 1;
	
	outputBest<<"\n"<<Sol->sNameInst<<"\t"
		<<Sol->iKSIter<<"\t"
		<<Sol->sStatusOpt<<"\t"
		<<Sol->dSolutionValue<<"\t"
		<<Sol->dLowerBound<<"\t"
		<<Sol->dOptGap<<"\t"
		<<Sol->dComputingTime<<"\t"
		<<Sol->iKernelSize_ini<<"\t"
		<<Sol->iKernelSize_fin<<"\t"
		<<Sol->iBucketSize<<"\t"
		<<iTotalItemsBucket<<"\t"
		<<(Sol->iKernelSize_fin - Sol->iKernelSize_ini)<<"\t";

	outputBest.close();

	//=========================================================================//
	//							Print solution values						   //
	//=========================================================================//
	sNameFO = sNameOfInstance + "_Sol.txt";
	ofstream outputSol(sNameFO, ios::app);

	outputSol<<"Instance:"<<sNameOfInstance<<endl;
	outputSol<<"P = "<<P<<endl;
	outputSol<<"n = "<<N<<endl;
	outputSol<<"Beta = "<<dBeta<<endl;
	outputSol<<"k = "<<ceil(dBeta * N)<<endl;
	outputSol<<"Object value = "<<(vSolutionPool.end() - 1)->dSolutionValue<<endl;
	
	int iCountAux = 0;
	double dTotalDistance = 0.0;
	for(TUPLE3::iterator Sol = vBestSolution.begin(); Sol != vBestSolution.end(); Sol++){
		if(iCountAux < P){ 
			//Facilities
			if(iCountAux == 0) outputSol<<"Selected warehouses: "<<endl;
			outputSol<<std::get<0>(*Sol)<<endl;
		}else{
			//Assignment
			if(iCountAux == P) outputSol<<"Distance distribution (Customer, Warehouse, Distance): "<<endl;
			outputSol<<std::get<0>(*Sol)<<"\t"<<std::get<1>(*Sol)<<"\t"<<std::get<2>(*Sol)<<endl;
			dTotalDistance += std::get<2>(*Sol);
		}	
		++iCountAux;
	}

	if(vBestSolutionMultipliers.size() > 0) outputSol<<"Multipliers (node,value): "<<endl;
	iCountAux = 0;
	for(std::vector<double>::iterator Sol = vBestSolutionMultipliers.begin(); Sol != vBestSolutionMultipliers.end(); Sol++){
		if(*Sol != 0.0){
			outputSol<<iCountAux<<"\t"<<*Sol<<endl;
		}
		++iCountAux;
	}

	outputSol<<"Total distance = "<<dTotalDistance<<endl;
	outputSol.close();

	return;
}



///////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	setLPRelaxation()
//	Developed by	:	Diana Lucia Huerta Munoz		     
//	Description		:   Solve the linear relaxion of <model> and return the list
//						of the most prominent elements in vSortedCandidateList
//	Date			:   Dic 2018
///////////////////////////////////////////////////////////////////////////////////
IloAlgorithm::Status CKSCVAR::setLPRelaxation(TUPLE3 &vSortedCandidateList, std::vector<tuple<int,int,double>> vYsolution, double &dTotalTime, double dMILPTime, bool bSetMILPValues, int &iKernelSize){

	double dDummyUB = 1e5;
	IloAlgorithm::Status dDummyStatus;
	vSortedCandidateList.resize(0);

	//==========================================================//
	//2.- Create LR <model>: convert Xs and Ys vars float, fix values
	//==========================================================//
	IloModel modelRelax(env);
	modelRelax.add(model);

	for(int i = 0; i<N; i++){
		modelRelax.add(IloConversion(env, x[i], ILOFLOAT));
	}modelRelax.add(IloConversion(env, y, ILOFLOAT));
			

	//==============================================================//
	//3.- Solve the LP relaxation <model> - dMILPTime/2 sec at most//
	//==============================================================//
	IloCplex cplexRelax(modelRelax);	
	cplexRelax.extract(modelRelax);
	IloAlgorithm::Status iIloStatus = solveModel(cplexRelax, dMILPTime, dDummyUB, dTotalTime, dDummyStatus, -1,0); 

	//TEST-Diana
	int iCountYpositive = 0, iCountYZero=0;
	int iCountXpositive = 0, iCountXZero=0;

	if(iIloStatus == IloAlgorithm::Status::Optimal || iIloStatus == IloAlgorithm::Status::Feasible){

		//Get the reduced costs of Ys depending on its solution value
		TUPLE3 vYsolutionRelax1, vYsolutionRelax0; //ID node, solution value, reduced costs
		for(int p=0; p<Pmax; p++){
			if(cplexRelax.getValue(y[p]) >= 0.0 +  EPS){
				vYsolutionRelax1.push_back(make_tuple(p,cplexRelax.getValue(y[p]), (double)cplexRelax.getReducedCost(y[p])));	//y = 1
				++iCountYpositive;
			}else{
				vYsolutionRelax0.push_back(make_tuple(p,cplexRelax.getValue(y[p]), (double)cplexRelax.getReducedCost(y[p])));	//y = 0
				++iCountYZero;
			}
		}

		//TEST-Diana: P is equal to the num of Y with positive value
		iKernelSize = min(iCountYpositive, Pmax);

		//Sort Y variable: create 1 vector per Ys=1 and 1 vector per Ys=0
		//According to the sorting of Y variables, create kernel and buckets
		if(vYsolutionRelax1.size() > 0) orderVectorList(vYsolutionRelax1, 0, (int)vYsolutionRelax1.size()-1, true); //Increasing order
		if(vYsolutionRelax0.size() > 0) orderVectorList(vYsolutionRelax0, 0, (int)vYsolutionRelax0.size()-1, false);//Decreasing order
		mergeVectorsAB(vYsolutionRelax1, vYsolutionRelax0);							//union(A + B)
		vSortedCandidateList = vYsolutionRelax1;		
		
		//This is the complete list of elements sorted by a certain criterion
		vInitialRankedList = vSortedCandidateList;//Copy complete list

		//Then obtain the value of Xs
		for(int i= 0; i<N; i++){
			for(int p= 0; p<Pmax; p++){	
				dRedCostsX[i][p] = (double)cplexRelax.getReducedCost(x[i][p]);

				//Count the number of Xs with positive value & zero
				if(cplexRelax.getValue(x[i][p]) >= 0.0 + EPS){
					++iCountXpositive;
				}else{
					++iCountXZero;
				}
			}
		}

	}else{
		cout<<"Non LP solution found. Impossible to obtain information to create the initial Kernel"<<endl;
	}

	//TEST-Diana Print values	
	if(CommonFunctions::getLengthFile("LP-Info.dat") == 0){
		ofstream fOutput("LP-Info.dat", ios::app);
		fOutput<<"Instance\tPval\tYp\tY0\tXp\tX0\n";
		fOutput.close();
	}
	ofstream fOutput("LP-Info.dat", ios::app);
	fOutput<<sNameOfInstance<<"\t"<<iKernelSize<<"\t"<<iCountYpositive<<"\t"<<iCountYZero<<"\t"<<iCountXpositive<<"\t"<<iCountXZero<<"\n";
	fOutput.close();

	modelRelax.end();
	cplexRelax.end();

	return iIloStatus;
}


///////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	solveModel()
//	Developed by	:	Diana Lucia Huerta Munoz		     
//	Description		:   Solve the model by applying CPLEX
//	Date			:   Dic 2018
///////////////////////////////////////////////////////////////////////////////////
IloAlgorithm::Status CKSCVAR::solveModel(IloCplex &cplexE, double dMILPTime, double &dBestUB, double &dTotalTime, IloAlgorithm::Status &IloIncumbentStatus, int iIter, int iApplyKS){
	
	//cout<<"Solving problem - iter "<<iIter<<endl;

	IloAlgorithm::Status iloStatus = IloAlgorithm::Status::Unknown;

	////Iteration 0 is for the MILP solved to obtain the initial solution
	if(iApplyKS == 1 && iIter <= 0){//I have to do this to reproduce tests and check possible issues
		//dMILPTime = 20700 * (dMILPTime/60.0); //20700 ticks are more or less 1 min 
		//cplexE.setParam(IloCplex::DetTiLim, dMILPTime);
		cplexE.setParam(IloCplex::TiLim ,dMILPTime);
	}else{
		cplexE.setParam(IloCplex::TiLim ,dMILPTime);
	}
	cplexE.setParam(IloCplex::Threads,1);
	cplexE.setParam(IloCplex::ParallelMode,1);

	if(iIter > 1) cplexE.setParam(cplexE.CutUp,dBestUB - EPS);
	//cplexE.readMIPStart("pmed19SolCplex.mst");

	string sNameLPFile = sNameOfInstance + itos(iIter) + ".lp";
	string sNameLogFile = sNameOfInstance + itos(iIter) + ".log";		

	ofstream filenameLog(sNameLogFile.c_str());
	cplexE.exportModel(sNameLPFile.c_str());

	cplexE.setOut(filenameLog);

	clock_t cTime = clock();
	if (!cplexE.solve()) {
		filenameLog.close();
		remove(sNameLogFile.c_str());
		remove(sNameLPFile.c_str());
	}

	//cplexE.writeMIPStart(sNameMSTFile.c_str()); //write MST file for the solution obtained, this can be used to analyze feasibility
	//evalFeasibility(sNameLPFile, cplexE, PCENTER);
	double dTime = (clock() - (double)cTime)/(double)CLOCKS_PER_SEC;
	dTotalTime += dTime;

	filenameLog.close();
	iloStatus = cplexE.getStatus();

	//If solution is OPTIMAL or FEASIBLE
	if(iloStatus == IloAlgorithm::Status::Optimal || iloStatus == IloAlgorithm::Status::Feasible){
		if(cplexE.getObjValue() < dBestUB){
			dBestUB = cplexE.getObjValue();
			IloIncumbentStatus = iloStatus;

			//Update the best solution value (<=BestUB)
			dBestSolutionValue = (double)cplexE.getObjValue();
		}
	}

	return iloStatus;

}


///////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	createKernelBucket()
//	Developed by	:	Diana Lucia Huerta Munoz		     
//	Description		:   Obtain the main iKernelSizeY/X elements to consider and add them 
//						to vYsolutionRelax1
//	Date			:   Dic 2018
///////////////////////////////////////////////////////////////////////////////////
void CKSCVAR::createKernelBucket(std::vector<tuple<int,int,double>> &vYsolutionRelax1, std::vector<tuple<int,int,double>> &vKernel, int iKernelSizeY, int iIsInitialKernel){

	int iCount = 0;
	vKernel.resize(0);

	//1.- For having a control of the nodes assigned to facilities
	int *iNodeAssign = new int[N];
	for(int i=0; i<N; i++){
		iNodeAssign[i] = 0; //Non assigned yet
	}

	//2.- Initialize the threshold for each new kernel element (Y)
	//	  This is for X vars once a Y var is selected
	double *dThreshold = new double[N]; 
	for(int i=0; i<N; i++){
		dThreshold[i] = 0.0;
	}

	//3.- Obtain the next Kernels Y and X
	std::vector<int> vKernelY;
	while(iCount < iKernelSizeY && vYsolutionRelax1.size()>0){

		//For a given facility add Xs <= Threshold
		auto it = vYsolutionRelax1.begin();
		int iValueY = std::get<0>(*it);
		double dMedian = 1e35;

		if(iIsInitialKernel != 1){
			for(int i=0; i<N; i++){
				dThreshold[i] = dRedCostsX[i][iValueY];
			}

			//Order the vector in inscreasing order
			quicksort_doublearray(dThreshold, 0, N-1);
			
			//Compute the median Threshold_x for each kernel_y		
			if(N % 2 > 0.0){//Odd number
				dMedian = dThreshold[(int)(N*0.5)]; //abs(dThreshold[iCount])/(double)N;
			}else{//Even number
				int iMiddle = (int)ceil(N*0.5);
				dMedian = (dThreshold[iMiddle] + dThreshold[iMiddle + 1])*0.5; //abs(dThreshold[iCount])/(double)N;
			}
		}

		//cout<<"Average Threshold["<<iCount<<"] = "<<dThreshold[iCount]<<endl;
		//Found all Xs <= Threshold of y[p]
		for(int i=0; i<N; i++){
			if(i != iValueY && (dRedCostsX[i][iValueY] <= dMedian + EPS)){ //abs(dRedCostsX[i][iValueY])
				vKernel.push_back(make_tuple(i, iValueY, dRedCostsX[i][iValueY]));
				iNodeAssign[i] = 1;
			}else if(i == iValueY){
				vKernel.push_back(make_tuple(i, iValueY, dRedCostsX[i][iValueY]));
				iNodeAssign[i] = 1;
			}
		}

		//Delete selected Y and count it.
		vKernelY.push_back(iValueY);
		vYsolutionRelax1.erase(it);
		++iCount;
	}

	//Check if all customers are assigned (to obtain a feasible solution)
	if(vKernelY.size() > 0 && iIsInitialKernel == 1){
		int iCountAux = 0;
		for(int i=0; i<N; i++){
			if(iNodeAssign[i] == 0){//Assign the customers to a selected Y facility, whose distance is the shortest
				double dCostMin = 1e35;
				int iCustBest = 0, iYBest = 0;
				for(auto itKY = vKernelY.begin(); itKY != vKernelY.end(); itKY++){
					if(*itKY != i && dDist[i][*itKY] < dCostMin){
						dCostMin = dDist[i][*itKY];
						iCustBest = i;
						iYBest = *itKY;
					}else if(*itKY == i){
						dCostMin = 0.0;
						iCustBest = i;
						iYBest = *itKY;
						break;
					}
				}
				vKernel.push_back(make_tuple(iCustBest, iYBest, dRedCostsX[i][iYBest]));
				iNodeAssign[i] = 1;
				++iCountAux;
			}
		}
		cout<<"Extra-elements =  "<<iCountAux<<endl;
	}
	
	//Here, kernel is Ok, the remaining ones conform the bucket = vYsolutionRelax1

	delete [] iNodeAssign;
	delete [] dThreshold;

	return;

}


///////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	FacilityLocationModel()
//	Developed by	:	Diana Lucia Huerta Munoz		     
//	Description		:   Obtain the MILP formulation to analyze
//	Date			:   Dic 2018
///////////////////////////////////////////////////////////////////////////////////
void CKSCVAR::FacilityLocationModel(){

	double dK = ceil(dBeta * N);

	//Formulation p-center ==================================
	cout<<"Getting Facility Location formulation: "<<endl;		
		
	////========== Giving names to variables ==========////
	Z.setName("z");
	for(int p=0; p<Pmax; p++){
		stringstream nameY;
		nameY << "y" << p;
		y[p].setName(nameY.str().c_str());

		for(int i=0; i<N; i++){
			stringstream nameX;
			nameX << "x" << i <<"_"<< p;
			x[i][p].setName(nameX.str().c_str());	
		}
	}

	for(int i=0; i<N; i++){
		stringstream nameX;
		nameX << "u" << i ;
		u[i].setName(nameX.str().c_str());	
	}
	
	//======================================================//
	////========== Defining Objective Function ===========////
	//======================================================//

	//C5: Objective function - Depending on the type of problem to evaluate, the objective function is different
	IloExpr OF(env);
	if(tTypeProblem == PMEDIAN || tTypeProblem == PBIOBJ_PMED){		//======== Obj Pmedian 

		for(int p=0; p<Pmax; p++){
			for(int i=0; i<N; i++){
				OF += dDist[i][p] * x[i][p];											
			}
		}

	}else if(tTypeProblem == PCENTER){									//======= Pcenter

		OF =  Z;

	}else if(tTypeProblem == PMINIMAX || tTypeProblem == PBIOBJ_CVAR){//======= Pminimax - CVaR

		OF =  (dK * Z);
		for(int i=0; i<N; i++){
			OF += (u[i]);
		}

	}

	model.add(IloMinimize(env,OF)).setName(getStringFromEnumValue(CONSTID,0));
	OF.end();

	//====================================================================//
	////==================== Defining Constraints =======================///
	//====================================================================//

	//C6 are located at the end
	
	//C7: Each customers must be assigned to one facility
	for(IloInt i = 0;  i < N; i++){
		IloExpr InvEqL(env);
		for(int p=0; p<Pmax; p++){	
			InvEqL += x[i][p];
		}
		model.add(InvEqL == 1).setName(getStringFromEnumValue(CONSTID,1));
		InvEqL.end();
	}

	//C8:The number of opened facilities must be P
	IloExpr InvEqL(env);
	for (int p = 0; p < Pmax; p++) {
		InvEqL += y[p];
	}
	model.add(InvEqL == P).setName(getStringFromEnumValue(CONSTID, 3));
	InvEqL.end();

	//C9: If facility p is not opened, no customer must be assigned
	for(IloInt i = 0;  i < N; i++){			
		for(int p=0; p<Pmax; p++){		
			model.add(x[i][p] <= y[p]).setName(getStringFromEnumValue(CONSTID,2));
		}
	}

	//========================================================================================//
	//C6: Maximum distance between a customer and its closest facility - Depending on the type of problem, the constraints are different
	//========================================================================================//
	if (tTypeProblem == PCENTER) {	//Pcenter case

		//Maximum distance between a customer and its closest facility
		for (IloInt i = 0; i < N; i++) {
			IloExpr InvEqL(env);
			for (int p = 0; p < Pmax; p++) {
				InvEqL += dDist[i][p] * x[i][p];
			}
			model.add(Z >= InvEqL).setName(getStringFromEnumValue(CONSTID, 4));
			InvEqL.end();
		}

	}
	else if (tTypeProblem == PMINIMAX || tTypeProblem == PBIOBJ_CVAR) {//PMinimax 

		//Maximum distance between a customer and its closest facility
		for (IloInt i = 0; i < N; i++) {
			IloExpr InvEqR(env);
			for (int p = 0; p < Pmax; p++) {
				InvEqR += dDist[i][p] * x[i][p];
			}
			model.add((dK * Z + dK * u[i]) >= InvEqR).setName(getStringFromEnumValue(CONSTID, 4));
			InvEqR.end();
		}

	}

	//========================================================================================//
	//Extra Constraints for the CVaR and Pmedian problems
	//========================================================================================//
	if (tTypeProblem == PBIOBJ_CVAR) {

		//f(x) < epsilon
		IloExpr InvEq(env);
		for (int p = 0; p < Pmax; p++) {
			for (int i = 0; i < N; i++) {
				InvEq += dDist[i][p] * x[i][p];
			}
		}
		model.add(InvEq <= dEpsilon).setName(getStringFromEnumValue(CONSTID, 5));
		InvEq.end();

	}else if (tTypeProblem == PBIOBJ_PMED) {

		//g(x) < epsilon
		IloExpr InvEq(env);
		InvEq = (dK * Z);
		for (int i = 0; i < N; i++) {
			InvEq += (u[i]);
		}
		model.add(InvEq <= dEpsilon).setName(getStringFromEnumValue(CONSTID, 5));
		InvEq.end();

		//Maximum distance between a customer and its closest facility
		for (IloInt i = 0; i < N; i++) {
			IloExpr InvEqR(env);
			for (int p = 0; p < Pmax; p++) {
				InvEqR += dDist[i][p] * x[i][p];
			}
			model.add((dK * Z + dK * u[i]) >= InvEqR).setName(getStringFromEnumValue(CONSTID, 6));
			InvEqR.end();
		}
	}

	return;
}


///////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	fixKernel()
//	Developed by	:	Diana Lucia Huerta Munoz		     
//	Description		:   Fix all variables to 0 (except the ones that belongs toK)
//	Date			:   Dic 2018
///////////////////////////////////////////////////////////////////////////////////
void CKSCVAR::fixKernel(std::vector<tuple<int,int,double>> vKernel, IloNumVarArray2D &x_E, IloNumVarArray &y_E){

	//First initialize all in 0
	for(int p=0; p<Pmax; p++){
		y_E[p].setBounds(0.0, 0.0);
		for(int i=0; i<N; i++){
			x_E[i][p].setBounds(0.0, 0.0);
		}
	}

	int i=0, j=0, indexAux = -1;
	for(auto it = vKernel.begin(); it != vKernel.end(); it++){
		i = std::get<0>(*it); //Customer
		j = std::get<1>(*it); //Facility
		x_E[i][j].setUB(1.0 + EPS); //Initially, all upper bounds are set to 0 for X variables
		if(indexAux != j){
			indexAux = j;
			y_E[j].setUB(1.0 + EPS);
		}
	}

	return;
}

///////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	evalConstraints(std::string sNameFile, IloExpr _Expr1)
//	Developed by	:	Diana Lucia Huerta Munoz		     
//	Description		:   Get constraints of a LP/SAV model and evaluates infeasibilities
//	Date			:   March 2018
///////////////////////////////////////////////////////////////////////////////////
void CKSCVAR::evalFeasibility(IloCplex cplexAux){
	
	//Each customers must be assigned to one facility
	for(IloInt i = 0;  i < N; i++){
		double InvEqL = 0.0;
		for(int p=0; p<Pmax; p++){	
			InvEqL += cplexAux.getValue(x[i][p]);
		}
		if(InvEqL != 1.0) cout<<"Constraint 1 is INFEASIBLE: (InvEqL == 1) -> valueExpr = "<<InvEqL<<endl;
	}

	//If facility p is not opened, no customer must be assigned
	for(IloInt i = 0;  i < N; i++){			
		for(int p=0; p<Pmax; p++){		
			if(cplexAux.getValue(x[i][p]) > cplexAux.getValue(y[p])){
				cout<<"Constraint 2 is INFEASIBLE: (x[i][p] <= y[p]) -> valueExpr = "<<cplexAux.getValue(x[i][p])<<" > "<<cplexAux.getValue(y[p])<<endl;
			}
		}
	}

	//The number of opened facilities must be P
	double InvEqL2 = 0.0;
	for(int p=0; p<Pmax; p++){			
		InvEqL2 += cplexAux.getValue(y[p]);
	}
	if(InvEqL2 != P){
		cout<<"Constraint 3 is INFEASIBLE: (InvEqL == P) -> valueExpr = "<<InvEqL2<<" != "<<P<<endl;
	}

	if(tTypeProblem == PCENTER){	//Pcenter
		//Maximum distance between a customer and its closest facility
		for(IloInt i = 0;  i < N; i++){	
			double InvEqL1=0.0;
			for(int p=0; p<Pmax; p++){			
				InvEqL1 += dDist[i][p] * cplexAux.getValue(x[i][p]);
			}
			double dObj = cplexAux.getObjValue();
			if(dObj < InvEqL1){
				cout<<"Constraint 4 is INFEASIBLE: (Z >= SumX*Dist) -> valueExpr = "<<cplexAux.getObjective()<<" < "<<InvEqL1<<endl;
			}
		}
	}else if(tTypeProblem == PMINIMAX){	//PMinimax
		//Maximum distance between a customer and its closest facility
		for(IloInt i = 0;  i < N; i++){	
			double InvEqR=0.0;
			for(int p=0; p<Pmax; p++){			
				InvEqR += dDist[i][p] * cplexAux.getValue(x[i][p]);
			}
			if((1*cplexAux.getObjValue() + 1*cplexAux.getValue(u[i])) < InvEqR){
				cout<<"Constraint 4 is INFEASIBLE: (Z*dK >= SumX*Dist) -> valueExpr = "<<cplexAux.getObjective()<<" < "<<InvEqR<<endl;
			}
		}
	}

	////Non-negative constraints
	for(int p=0; p<Pmax; p++){	
		if(cplexAux.getValue(y[p]) < 0.0){
			cout<<"Constraint 5 is INFEASIBLE: (y[p]) >= 0) -> valueExpr = "<<cplexAux.getValue(y[p])<<" < "<<0.0<<endl;
		}
		for(IloInt i = 0;  i < N; i++){	
			if(cplexAux.getValue(x[i][p]) < 0.0){
				cout<<"Constraint 5 is INFEASIBLE: (x[i][p] >= 0.0) -> valueExpr = "<<cplexAux.getValue(x[i][p])<<" < "<<0.0<<endl;
			}
		}
	}

	return;
}

///////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	getBestSolutionValue
//	Developed by	:	Diana Lucia Huerta Munoz		     
//	Description		:   Get private value of the best solution
//	Date			:   March 2018
///////////////////////////////////////////////////////////////////////////////////
double CKSCVAR::getBestSolutionValue(){

	return dBestSolutionValue;
}

///////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	getKernel
//	Developed by	:	Diana Lucia Huerta Munoz		     
//	Description		:   Get the current kernel
//	Date			:   March 2018
///////////////////////////////////////////////////////////////////////////////////
TUPLE3 CKSCVAR::getKernel(){

	return vKernel;
}		

TUPLE3 CKSCVAR::getInitialRankedList(){

	return vInitialRankedList;

}

///////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	setReducedCostsX
//	Developed by	:	Diana Lucia Huerta Munoz		     
//	Description		:   Set the reduced costs of x in the private array
//	Date			:   March 2018
///////////////////////////////////////////////////////////////////////////////////
void CKSCVAR::setReducedCostsX(double **dRedCostsE){

	//Then obtain the value of Xs
	dRedCostsX = new double*[N];
	for(int i= 0; i<N; i++){
		dRedCostsX[i] = new double[Pmax];
		for(int p= 0; p<Pmax; p++){	
			dRedCostsX[i][p] = dRedCostsE[i][p];
		}
	}

}

///////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	getReducedCostsX
//	Developed by	:	Diana Lucia Huerta Munoz		     
//	Description		:   Copy the reduced costs of x vars
//	Date			:   March 2018
///////////////////////////////////////////////////////////////////////////////////
void CKSCVAR::getReducedCostsX(double** pdRedCostsE) {

	pdRedCostsE = new double* [N];
	for (int i = 0; i < N; i++) {
		pdRedCostsE[i] = new double[Pmax];
		for (int p = 0; p < Pmax; p++) {
			pdRedCostsE[i][p] = dRedCostsX[i][p];
		}
	}

	return;
}
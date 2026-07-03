#include "CommonFunctions.h"
#include <iostream>


namespace CommonFunctions{
//////////////////////////////////////////////////////////////////////
//Function:	SplitString												//
//Date:		April 29th 2014											//
//Authors:	Jessica Rodriguez and Diana Huerta						//
//Brief:	Splits a string and saves elements in a vector			//		
//////////////////////////////////////////////////////////////////////
EXPORT_CF std::vector<std::string> SplitString(std::string str, const std::string &delim){
    
	size_t found = 0;
	std::string token;
	std::vector<std::string> result;
	while ((found = str.find(delim)) != std::string::npos) {
		token = str.substr(0, found);
		result.push_back(token);
		//std::cout << token << std::endl;
		str.erase(0, found + delim.length());
	}
	result.push_back(str);
	std::vector<std::string> FinalResult;

	//Only apply this when the delimiter is not a tabulator
	//If we only want to obtain the words separated by -tabular- then this is not needed
	if(delim != "\t"){		
		if(found != std::string::npos){
			result.push_back(str.substr(0, found));
	
			//Delete blank spaces
			for(vector<string>::const_iterator i = result.begin(); i != result.end(); ++i) {
				if(*i != "") FinalResult.push_back(*i);
			}
		}else{
			FinalResult.push_back("-");
		}
	}else{
		FinalResult = result;
	}
   
    return FinalResult;
}

//////////////////////////////////////////////////////////////////////
//Function:	removeExtension											//
//Date:		Feb 18th 2015											//
//Authors:	Diana Huerta											//
//Brief:	remove extension from a file name						//		
//////////////////////////////////////////////////////////////////////
EXPORT_CF std::string removeExtension(std::string sNameFile){

	int iLastindex = (int)(sNameFile.find_last_of(".")); 
	string sNameWExt = sNameFile.substr(0, iLastindex); 
	return sNameWExt;
	
}

//////////////////////////////////////////////////////////////////////
//Function:	getFileExtension											//
//Date:		March 13th 2015											//
//Authors:	Diana Huerta											//
//Brief:	Get the extension of a file								//		
//////////////////////////////////////////////////////////////////////
EXPORT_CF std::string getFileExtension(std::string sNameFile){

	int iLastindex = (int)(sNameFile.find_last_of(".")); 
	string sNameExt = sNameFile.substr(iLastindex, iLastindex+5); 
	return sNameExt;
	
}

//////////////////////////////////////////////////////////////////////
//Function:	getFileExtension											//
//Date:		March 13th 2015											//
//Authors:	Diana Huerta											//
//Brief:	Get the extension of a file								//		
//////////////////////////////////////////////////////////////////////
EXPORT_CF int getLengthFile(std::string sNameFile){

	ofstream File(sNameFile.c_str(), ios::app);
	int iLenghtFile = 0;
	if(File){
		File.seekp(0, File.end);
		iLenghtFile = (int)File.tellp();
		File.close();
	}
	
	return iLenghtFile;	
}

//////////////////////////////////////////////////////////////////////
//Ordena valores en el vector a
//////////////////////////////////////////////////////////////////////
EXPORT_CF void quicksort_doublearray(double *&a,int inf, int sup){

    int i=inf;
    int j=sup; 
    int p = (inf+sup)/2;
    double distancia = a[p];
    do {    
        while (a[i] < distancia) ++i; 
        while (a[j] > distancia) --j;
        if (i<=j){
            double h=a[i]; 
			a[i]=a[j]; 
			a[j]=h;
            ++i; --j;
        }
    }while (i<=j);   //  recursion
    if (inf<j) quicksort_doublearray(a, inf, j);
    if (i<sup) quicksort_doublearray(a, i, sup);
}

//////////////////////////////////////////////////////////////////////
//Function:	reduce_decimals											//
//Date:		Feb 10th 2017											//
//Authors:	Diana Lucia Huerta Munoz								//
//Brief:	Reduce the number of decimals of a double				//		
//////////////////////////////////////////////////////////////////////
EXPORT_CF std::string itos(int x){

    stringstream ss;
    ss << x;
	return ss.str();

}


//////////////////////////////////////////////////////////////////////
//Function:	floydWarshall											//
//Date:		Nov 2018												//
//Authors:	Diana L. Huerta Munoz									//
//Brief:	Solves the all-pairs shortest path problem using Floyd	//
//			Warshall algorithm										//
////////////////////////////////////////////////////////////////////// 
EXPORT_CF void floydWarshall(double **graph, int iDim){ 

    for (int k = 0; k < iDim; k++){// Pick all vertices as source one by one 
        for (int i = 0; i < iDim; i++){// Pick all vertices as destination for the above picked source 
            for (int j = 0; j < iDim; j++){ 
                // If vertex k is on the shortest path from 
                // i to j, then update the value of dist[i][j] 
                if (graph[i][k] + graph[k][j] < graph[i][j]) 
                    graph[i][j] = graph[i][k] + graph[k][j]; 
            } 
        } 
    } 

	return;

}

//////////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	orderVectorList												//
//	Developed by	:	Diana Lucia Huerta Munoz									//
//	Description		:   Re-order a vector of pairs in bIncrementOrder				//
//	Date			:   Oct 2016													//	
//////////////////////////////////////////////////////////////////////////////////////
EXPORT_CF void orderVectorList(std::vector<tuple<int, int, double>> &vSavingsList, int iIni, int iFin, bool bIncrementOrder){

	int iMiddleList = (iFin + iIni)/2;

	double x= std::get<2>(vSavingsList[iMiddleList]);	//Order by the double value
    int i = iIni;
    int j = iFin;

	if(bIncrementOrder){
		do {    
			while (std::get<2>(vSavingsList[i]) > x) ++i; 
			while (std::get<2>(vSavingsList[j]) < x) --j;
			if (i<=j){

				swap(vSavingsList[i],vSavingsList[j]);
				++i; --j;
			}
		}while (i<=j);   //  recursion
	}else{
		do {    
			while (std::get<2>(vSavingsList[i]) < x) ++i; 
			while (std::get<2>(vSavingsList[j]) > x) --j;
			if (i<=j){
				swap(vSavingsList[i],vSavingsList[j]);
				++i; --j;
			}
		}while (i<=j);   //  recursion
	}

    if (iIni<j)  orderVectorList(vSavingsList, iIni, j, bIncrementOrder);  
    if (i<iFin)  orderVectorList(vSavingsList, i, iFin, bIncrementOrder);  

	return;
}

//////////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	mergeVectorsAB												//
//	Developed by	:	Diana Lucia Huerta Munoz									//
//	Description		:   Merge two vectors of tuples									//
//	Date			:   Oct 2016													//	
//////////////////////////////////////////////////////////////////////////////////////
EXPORT_CF void mergeVectorsAB(std::vector<tuple<int,int,double>> &A, const std::vector<tuple<int,int,double>> &B){
    A.reserve( A.size() + B.size() );                // preallocate memory without erase original data
    A.insert( A.end(), B.begin(), B.end() );         // add B;
    return;                                        // here A could be named AB
}

//////////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	removeDuplicateTuples												//
//	Developed by	:	Diana Lucia Huerta Munoz									//
//	Description		:   Remove duplicate tuples from a vector						//
//	Date			:   Oct 2016													//	
//////////////////////////////////////////////////////////////////////////////////////
EXPORT_CF void removeDuplicateTuples(vector<tuple<int,int,double>> &vComplete, vector<tuple<int,int,double>> &vBase){

	////Order vectors to make it more efficient
	//std::sort (vComplete.begin(),vComplete.end());   
	if(vComplete.size()>0 && vBase.size() > 0){
		std::sort (vBase.begin(),vBase.end()); //order the base list

		for(std::vector<tuple<int,int,double>>::iterator it = vBase.begin(); it != vBase.end(); it++){
			for(std::vector<tuple<int,int,double>>::iterator it2 = vComplete.begin(); it2 != vComplete.end(); it2++){
				if(std::get<0>(*it2) == std::get<0>(*it) && (std::get<1>(*it2) == std::get<1>(*it))){
					//cout<<"--> "<<std::get<1>(*it2) <<" - "<< std::get<1>(*it)<<endl;
					//vBase.erase(it);
					std::get<2>(*it) = -1; //Change to -1 to identify the selected element
					vComplete.erase(it2);
					break;
				}
			}
		}

		vComplete.reserve(vComplete.size()); //use only the capacity that is used actively
	}
	return;

}

//////////////////////////////////////////////////////////////////////
//Function:	DELETEMATRIXDOUBLE										//
//Date:		March 02 2017											//
//Authors:	Diana L. Huerta Munoz									//
//Brief:	Delete a Matrix (memory free)							//		
//////////////////////////////////////////////////////////////////////
EXPORT_CF void DELETEMATRIXDOUBLE(double **&pdMatrix, int iDimension){

	if(sizeof(pdMatrix) == NULL) delete [] pdMatrix;
	else{
		for(int i=0; i < iDimension; i++){
		
			delete [] pdMatrix [i];

		}delete [] pdMatrix;
	}
}

}


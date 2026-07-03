#include "MySources.h"

ILOSTLBEGIN


///////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	CreateArray2D
//	Developed by	:	Diana Lucia Huerta Munoz		     
//	Description		:   Initialize a 2 dimension IloNumVarArray
//	Date			:   Jul 2018
///////////////////////////////////////////////////////////////////////////////////
void CreateArray2D(IloNumVarArray2D &tArray, IloEnv tEnv, int iDim1, int iDim2,IloNumVar::Type Type){
	
	tArray =  IloNumVarArray2D(tEnv, iDim1);
	for(int i=0; i<iDim1; i++){
		tArray[i] =  IloNumVarArray(tEnv, iDim2, 0, IloInfinity, Type);
	}

}

///////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	CreateArray2D
//	Developed by	:	Diana Lucia Huerta Munoz		     
//	Description		:   Initialize a 2 dimension IloNumVarArray when index 0 is INT, otherwise Type
//	Date			:   Jul 2018
///////////////////////////////////////////////////////////////////////////////////
void CreateArray2DX(IloNumVarArray2D &tArray, IloEnv tEnv, int iDim1, int iDim2,IloNumVar::Type Type){
	
	tArray =  IloNumVarArray2D(tEnv, iDim1);
	for(int i=0; i<iDim1; i++){
		if(i==0){
			tArray[i] =  IloNumVarArray(tEnv, iDim2, 0, IloInfinity, IloNumVar::Type::Int);
		}else{
			tArray[i] =  IloNumVarArray(tEnv, iDim2, 0, 1, Type);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	CreateArray2D
//	Developed by	:	Diana Lucia Huerta Munoz		     
//	Description		:   Initialize a 2 dimension IloBoolVarArray 
//	Date			:   Jul 2018
///////////////////////////////////////////////////////////////////////////////////
void CreateArray2D(IloBoolVarArray2D &tArray, IloEnv tEnv, int iDim1, int iDim2){
	
	tArray =  IloBoolVarArray2D(tEnv, iDim1);
	for(int i=0; i<iDim1; i++){
		tArray[i] =  IloBoolVarArray(tEnv, iDim2);
	}

}

///////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	CreateArray2D
//	Developed by	:	Diana Lucia Huerta Munoz		     
//	Description		:   Initialize a 2 dimension IloNumArray 
//	Date			:   Jul 2018
///////////////////////////////////////////////////////////////////////////////////
void CreateArray2D(IloNumArray2D &tArray, IloEnv tEnv, int iDim1, int iDim2){
	
	tArray =  IloNumArray2D(tEnv, iDim1);
	for(int i=0; i<iDim1; i++){
		tArray[i] = IloNumArray(tEnv, iDim2);
	}

}

///////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	CreateArray3D
//	Developed by	:	Diana Lucia Huerta Munoz		     
//	Description		:   Initialize a 3 dimension IloBoolVarArray 
//	Date			:   Jul 2018
///////////////////////////////////////////////////////////////////////////////////
void CreateArray3D(IloBoolVarArray3D &tArray, IloEnv tEnv, int iDim1, int iDim2, int iDim3){
	
	tArray =  IloBoolVarArray3D(tEnv, iDim1);
	for(int i=0; i<iDim1; i++){
		tArray[i] =  IloBoolVarArray2D(tEnv, iDim2);
		for(int j=0; j<iDim2; j++){
			tArray[i][j] = IloBoolVarArray(tEnv, iDim3);
		}
	}

}

///////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	CreateArray3D
//	Developed by	:	Diana Lucia Huerta Munoz		     
//	Description		:   Initialize a 3 dimension IloNumVarArray 
//	Date			:   Jul 2018
///////////////////////////////////////////////////////////////////////////////////
void CreateArray3D(IloNumVarArray3D &tArray, IloEnv tEnv, int iDim1, int iDim2, int iDim3, IloNumVar::Type Type){
	
	tArray =  IloNumVarArray3D(tEnv, iDim1);
	for(int i=0; i<iDim1; i++){
		tArray[i] =  IloNumVarArray2D(tEnv, iDim2);
		for(int j=0; j<iDim2; j++){
			tArray[i][j] = IloNumVarArray(tEnv, iDim3, 0, IloInfinity, Type);
		}
	}

}

///////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	CreateArray3D
//	Developed by	:	Diana Lucia Huerta Munoz		     
//	Description		:   Initialize a 3 dimension IloNumArray 
//	Date			:   Jul 2018
///////////////////////////////////////////////////////////////////////////////////
void CreateArray3D(IloNumArray3D &tArray, IloEnv tEnv, int iDim1, int iDim2, int iDim3){
	
	tArray =  IloNumArray3D(tEnv, iDim1);
	for(int i=0; i<iDim1; i++){
		tArray[i] =  IloNumArray2D(tEnv, iDim2);
		for(int j=0; j<iDim2; j++){
			tArray[i][j] = IloNumArray(tEnv, iDim3);
		}
	}

}

///////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	CreateArray4D
//	Developed by	:	Diana Lucia Huerta Munoz		     
//	Description		:   Initialize a 4 dimension IloBoolVarArray
//	Date			:   Jul 2018
///////////////////////////////////////////////////////////////////////////////////
void CreateArray4D(IloBoolVarArray4D &tArray, IloEnv tEnv, int iDim1, int iDim2, int iDim3, int iDim4){
	
	tArray =  IloBoolVarArray4D(tEnv, iDim1);
	for(int i=0; i<iDim1; i++){
		tArray[i] =  IloBoolVarArray3D(tEnv, iDim2);
		for(int j=0; j<iDim2; j++){
			tArray[i][j] = IloBoolVarArray2D(tEnv, iDim3);
			for(int k=0; k<iDim3; k++){
				tArray[i][j][k] = IloBoolVarArray(tEnv, iDim4);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	CreateArray4D
//	Developed by	:	Diana Lucia Huerta Munoz		     
//	Description		:   Initialize a 4 dimension IloNumVarArray
//	Date			:   Jul 2018
///////////////////////////////////////////////////////////////////////////////////
void CreateArray4D(IloNumVarArray4D &tArray, IloEnv tEnv, int iDim1, int iDim2, int iDim3, int iDim4, IloNumVar::Type Type){
	
	tArray =  IloNumVarArray4D(tEnv, iDim1);
	for(int i=0; i<iDim1; i++){
		tArray[i] =  IloNumVarArray3D(tEnv, iDim2);
		for(int j=0; j<iDim2; j++){
			tArray[i][j] = IloNumVarArray2D(tEnv, iDim3);
			for(int k=0; k<iDim3; k++){
				tArray[i][j][k] = IloNumVarArray(tEnv, iDim4, 0, IloInfinity, Type);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
//	FunctionName	:	CreateArray4D
//	Developed by	:	Diana Lucia Huerta Munoz		     
//	Description		:   Initialize a 4 dimension IloNumArray
//	Date			:   Jul 2018
///////////////////////////////////////////////////////////////////////////////////
void CreateArray4D(IloNumArray4D &tArray, IloEnv tEnv, int iDim1, int iDim2, int iDim3, int iDim4){
	
	tArray =  IloNumArray4D(tEnv, iDim1);
	for(int i=0; i<iDim1; i++){
		tArray[i] =  IloNumArray3D(tEnv, iDim2);
		for(int j=0; j<iDim2; j++){
			tArray[i][j] = IloNumArray2D(tEnv, iDim3);
			for(int k=0; k<iDim3; k++){
				tArray[i][j][k] = IloNumArray(tEnv, iDim4);
			}
		}
	}
}

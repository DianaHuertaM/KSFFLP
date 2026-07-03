#include <ilcplex/ilocplex.h>
#include<ilconcert\ilotupleset.h>
#include<ilcplex/ilocplexi.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#include "CommonFunctions.h"

//For Bool Multidimension Variables
typedef IloArray<IloBoolVarArray> IloBoolVarArray2D; 
typedef IloArray<IloArray<IloBoolVarArray>> IloBoolVarArray3D;
typedef IloArray<IloArray<IloArray<IloBoolVarArray>>> IloBoolVarArray4D;

//For Int Multidimension Variables
typedef IloArray<IloIntVarArray> IloIntVarArray2D; 
typedef IloArray<IloArray<IloIntVarArray>> IloIntVarArray3D;
typedef IloArray<IloArray<IloArray<IloIntVarArray>>> IloIntVarArray4D;

//For Num Multidimension Variables
typedef IloArray<IloNumVarArray> IloNumVarArray2D; 
typedef IloArray<IloArray<IloNumVarArray>> IloNumVarArray3D;
typedef IloArray<IloArray<IloArray<IloNumVarArray>>> IloNumVarArray4D;

//For Float Multidimension Variables
typedef IloArray<IloFloatArray> IloFloatArray2D; 
typedef IloArray<IloArray<IloFloatArray>> IloFloatArray3D;
typedef IloArray<IloArray<IloArray<IloFloatArray>>> IloFloatArray4D;

//For Int Multidimension Variables
typedef IloArray<IloNumArray> IloNumArray2D;
typedef IloArray<IloArray<IloNumArray>> IloNumArray3D;
typedef IloArray<IloArray<IloArray<IloNumArray>>> IloNumArray4D;

//For Int Multidimension Variables
typedef IloArray<IloBoolArray> IloBoolArray2D;
typedef IloArray<IloArray<IloBoolArray>> IloBoolArray3D;
typedef IloArray<IloArray<IloArray<IloBoolArray>>> IloBoolArray4D;

void CreateArray2D(IloNumVarArray2D &tArray, IloEnv tEnv, int iDim1, int iDim2, IloNumVar::Type Type);
void CreateArray2D(IloBoolVarArray2D &tArray, IloEnv tEnv, int iDim1, int iDim2);
void CreateArray2D(IloNumArray2D &tArray, IloEnv tEnv, int iDim1, int iDim2);
void CreateArray2D(IloBoolArray2D &tArray, IloEnv tEnv, int iDim1, int iDim2);
void CreateArray2DX(IloNumVarArray2D &tArray, IloEnv tEnv, int iDim1, int iDim2,IloNumVar::Type Type);

void CreateArray3D(IloBoolVarArray3D &tArray, IloEnv tEnv, int iDim1, int iDim2, int iDim3);
void CreateArray3D(IloNumVarArray3D &tArray, IloEnv tEnv, int iDim1, int iDim2, int iDim3, IloNumVar::Type Type);
void CreateArray3D(IloNumArray3D &tArray, IloEnv tEnv, int iDim1, int iDim2, int iDim3);
//void CreateArray3D(IloBoolArray3D &tArray, IloEnv tEnv, int iDim1, int iDim2, int iDim3);

void CreateArray4D(IloBoolVarArray4D &tArray, IloEnv tEnv, int iDim1, int iDim2, int iDim3, int iDim4);
void CreateArray4D(IloNumVarArray4D &tArray, IloEnv tEnv, int iDim1, int iDim2, int iDim3, int iDim4, IloNumVar::Type Type);
//void CreateArray4D(IloNumArray4D &tArray, IloEnv tEnv, int iDim1, int iDim2, int iDim3, int iDim4);
//void CreateArray4D(IloBoolArray4D &tArray, IloEnv tEnv, int iDim1, int iDim2, int iDim3, int iDim4);

//void CreateArray2D(IloIntVarArray2D &tArray, IloEnv tEnv, int iDim1, int iDim2);
//void CreateArray3D(IloIntVarArray3D &tArray, IloEnv tEnv, int iDim1, int iDim2, int iDim3);
//void CreateArray4D(IloIntVarArray4D &tArray, IloEnv tEnv, int iDim1, int iDim2, int iDim3, int iDim4);



void readInstance(std::string sFileName);
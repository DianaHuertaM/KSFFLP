# KSFFLP
Paper Source Code: A kernel search heuristic for a fair facility location problem

#Basic considerations

- Current CPLEX version: CPLEX_Studio2211. 
- Run the algorithm using the following command in terminal:

  KSVAR.exe pmed3.txt 2 0.10 7200 1

where:

Parameter 1: Executable file.

Parameter 2: Instance file (e.g. pmed3.txt located in the bin folder).

Parameter 3: Problem type:
- 0 = p-median
- 1 = p-center
- 2 = proposed FFLP (CVaR, beta)
  
Parameter 4: Value of beta - interval (0,1] (near 0 is equivalent to p-center, near 1 to p-median).

Parameter 5: Maximum computing time allowed 

Parameter 6: Solution method:
- 0 = Exact method (CPLEX)
- 1 = Proposed Kernel Search heuristic
- 2 = Testing option, where the kernel is generated separately and then the Kernel Search is solved using that predefined kernel.

The first two solution methods (0 and 1), for the last parameter, are the most relevant. The third option was mainly implemented for testing purposes and can safely be ignored.

If you have any questions or comments regarding the source code or the paper, please feel free to contact me at diana.huertam@uanl.mx

Diana Huerta

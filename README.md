## Lab-1 Rowhammer Mitigations:
     
In this lab assignment, you will implement rowhammer mitigations. First you will implement two previous rowhammer mitigations, Graphene (MICRO'20), that uses algorithmic tracking of aggressor rows with few counters stored in SRAM, and CRA (CAL'14) which uses exact tracking of aggressors with in-DRAM counters tracking all row activations. Further, you will design CRA++, an improvement on CRA to reduce its overheads. Overall, You will learn how to model and simulate these hardware defenses, understand the performance trade-offs of different design decisions, and potentially create a more efficient solution than previous work.
  

### Task-A - Graphene [2 points]
The first task is to implement the Misra-Gries tracker from [Graphene](https://www.microarch.org/micro53/papers/738300a001.pdf). A basic framework of the Misra-Gries tracker is available in `mgries.h/c`. You need to implement functions to check the tracker on a memory access, reset the tracker at appropriate times, and perform the mitigative action (refresh neighboring rows), when the time is right. We have implemented a function `memsys_rh_mitigate` in `memsys.c` for you to use for invoking the mitigative action.

You will have to complete the following specific tasks: 
1. Instantiate Misra-Gries tracker in `memsys.c` (`mgries_t`) 
  - Think: How large should the tracker be? How many trackers do you need? What should be the threshold? 
2. Implement the function to access and update the tracker; `mgries_access`.
3. Reset the tracker every 64ms using `mgries_reset` (assume a 4Ghz Clock).
4. Use the previous memory access information in `dram_acc_info` to appropriately invoke `mgries_access`.
5. Invoke the mitigation `memsys_rh_mitigate` when needed.

#### **Grading Criteria:**  
We will use the value of the statistics MGRIES-*_NUM_INSTALL and MSYS_RH_TOT_MITIGATE` to award points. +2 points if the values match the correct answer (+-5% error is acceptable), otherwise 0.
  

### Task-B - CRA [4 points]
The second task is to implement [CRA](http://memlab.ece.gatech.edu/papers/CAL_2014_1.pdf). A basic framework for the in-DRAM counters is available in `cra_ctr.h/c`. For this task, we will assume that out of the 16GB memory, the first 15GB stores the data and the last 1GB only stores the counters. For the simulation, you will store the counter values in a `CraCtr` structure within `memsys.c` and appropriately insert accesses to the DRAM to simulate the extra memory activity for accessing the in-DRAM counters. The first part of the task involves implementing the counters functionally, and the second part involves adding a counter-cache (structure `Ctrcache` provided in `ctrcache.c/h`) to reduce the number of DRAM accesses for accessing the counters.

The first sub-part has the following tasks:
1. Instantiate the CRA counters in `memsys.c` (`cra_t`).
2. Implement the functions to read and write to the in-DRAM counters in `cra_ctr.c`.
   - Think: How should the rows in memory map to the counters in DRAM?
3. Use the previous memory access information in `dram_acc_info` to appropriately invoke `cra_ctr_read/write`.
4. Invoke the mitigation `memsys_rh_mitigate` when needed.

The second sub-part has the following tasks:
1. Instantiate the ctr-cache in `cra` (`ctr_cache`).
2. Use `ctrcache_access` in `indramctr_read/write` to check if a counter is present in the cache and `ctrcache_install` to install if not present. 
   - Think: How do you handle reads and writes to the cache? When will writes propagate to memory? 

#### **Grading Criteria:**  
We will use the value of the statistics CRA_NUM_READS, CRA_NUM_WRITES and MSYS_RH_TOT_MITIGATE` to award points. +3 points if the values match the correct answer (+-5% error is acceptable), otherwise 0.
  

### Task-C - CRA++ [3 points + bonus]
The third task is to implement CRA++, your technique to further reduce the overheads of CRA. You will focus on two sources of overheads: (a) number of reads and writes to the counters in DRAM, (b) number of times the mitigation is invoked. For instance, this task is intentionally open-ended to encourage you to use your creativity aad come up with a new solution!  

To help you reduce these overheaads, you can think about the following two questions orthogonally:
1. How do we reduce number of reads and writes for the counters, while keeping the number of mitigations the same?
2. How can we reduce the number of mitigations, while still maintaining the security guarantees?

You are not allowed to change the number of sets and associativity for the counter-cache. But you are allowed to repurpose the counters or modify the operation of the counter cache in any other manner.
  

### Files for Submission.
You will submit one tar with three folders inside the tar: src_lab1_a, src_lab1_b, src_lab1_c, each containing the entire source code (`src` folder) for the particular task. Please make sure that your code compiles and runs on either the Ubuntu or Red-Hat machines provided by [ECE](https://help.ece.gatech.edu/labs/names)  or [CoC](https://support.cc.gatech.edu/facilities/general-access-servers). We will compile, run and evaluate your code on similar machines.
  

### Collaboration / Plagiarism Policy
- You are allowed to discuss the problem and potential solution directions with your peers.
- You are not allowed to discuss or share any code with your peers, or copy code from any source on the internet. All work you submit MUST be your own.
- You are also not allowed to post the problems or your solutions on the online.
- All submitted code will be checked for plagiarism against each other submissions (and other sources from the internet).


## DRAM RH SIM v1.0
A fast DRAM simulator for evaluating Rowhammer defenses.

### Instructions to use the Simulator
- **Compile**: To compile, use `cd src/; make all`. This will download the traces from Dropbox (almost 800MB), and compile the source code to create `sim` executable.
- **Run**; To run the simulator with a workload's execution trace (e.g. `../TRACES/bwaves_06.mat.gz`) running on 4 Cores of a CPU, use `./sim ../TRACES/bwaves_06.mat.gz ../TRACES/bwaves_06.mat.gz ../TRACES/bwaves_06.mat.gz ../TRACES/bwaves_06.mat.gz > ../RESULTS/BASELINE.4C.8MB/bwaves_06.res``. 
  - You can run the simulator with any of the traces in the `../TRACES/` folder with the same trace or different traces running on the 4 CPU cores. 
  - `../scripts/runall.sh` provides the command for running one benchmark and also all benchmarks in parallel.
- **Get Statistics**: To get statistics, you can check the output of the simulator. For the bwaves_06 run  above, you can see `../RESULTS/BASELINE.4C.8MB/bwaves_06.res` for statistics like `SYS_CYCLES : 2155238427`. 
  - To report a statistic (SYS_CYCLES) for all benchmarks and several results folders, use `cd ../scripts;  perl getdata.pl -w lab1 -amean -s SYS_CYCLES  -d ../RESULTS/BASELINE.4C.8MB <ANOTHER RESULTS FOLDER>`. 
  - `getdata.pl` has several other useful flags including `-gmean` for geometric mean, `-n` for normalizing results of one folder with another, `-mstat` to multiply all stats with a constant value and more.
  


#ifndef INDRAMCTR_H
#define INDRAMCTR_H

#include "global_types.h"

typedef struct InDramCtr InDramCtr;
#define NUM_CTRS_PER_CL (16)  // **Q**

///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////


struct InDramCtr{
  uns64         *counts;
  uns           num_ctrs;
  Addr          saturation_val; //--after which counter is saturated.
  uns           threshold; //-- after which mitigation is to be issued 

  uns64         s_num_reset;        //-- how many times was the tracker reset
  //---- Update below statistics in indramctr_reads() and indramctr_writes() ----
  uns64         s_num_reads;  //-- how many times was the tracker called
  uns64         s_num_writes; //-- how many times did the tracker install rowIDs 
  uns64         s_mitigations; //-- how many times did the tracker issue mitigation

};

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

InDramCtr *indramctr_new(uns num_ctrs,  uns saturation_val, uns threshold);
void    indramctr_reset(InDramCtr *m);

Flag   indramctr_read(InDramCtr *m, Addr rowAddr, uns64 in_cycle);
void    indramctr_write(InDramCtr *m, Addr rowAddr, uns64 in_cycle, uns64 counter_val);

void    indramctr_print_stats(InDramCtr *m);

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

#endif // INDRAMCTR_H

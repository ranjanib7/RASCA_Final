#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "indramctr.h"

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

InDramCtr *indramctr_new(uns num_ctrs, uns saturation_val, uns threshold){
  InDramCtr *m = (InDramCtr *) calloc (1, sizeof (InDramCtr));
  m->counts  = (uns64 *) calloc (num_ctrs, sizeof(uns64));
  m->num_ctrs = num_ctrs;
  m->saturation_val = saturation_val; //**Q**
  m->threshold = threshold;
  return m;
}

// **Q**: How do we restrict the memory pages & reserve pages for ctrs?
// **Q**: WHen will counters be reset ? Overhead?
// **Q**: Two counter solution?  

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

void    indramctr_reset(InDramCtr *m){
  uns ii;

  //------ update global stats ------
  m->s_num_reset++;

  //----- reset the structures ------  
  for(ii=0; ii < m->num_ctrs; ii++){
    m->counts[ii] = 0;

    //-- TODO: Call indramctr_write()?? **Q**
  }  
}

////////////////////////////////////////////////////////////////////
// The rowAddr field is the row to be updated in the tracker
// returns TRUE if mitigation must be issued for the row
////////////////////////////////////////////////////////////////////

Flag  indramctr_read(InDramCtr *m, Addr rowAddr, uns64 in_cycle){
  Flag retval = FALSE;
  m->s_num_reads++;

  //---- TODO: Calculate the indramctr to be accessed
  
  //---- TODO: Issue memory access using memsys_dram_accesss()
  
  //---- TODO: Read counter and decide if mitigation is to be issued (retval)
  
  if(retval==TRUE){
    m->s_mitigations++;
  }
  
  return retval;
}

void  indramctr_write(InDramCtr *m, Addr rowAddr, uns64 in_cycle, uns64 counter_val){
  m->s_num_writes++;

  //---- TODO: Calculate the indramctr to be accessed
  
  //---- TODO: Issue memory access using memsys_dram_accesss()
  
  //---- TODO: Update counter.
  
  return ;
}

////////////////////////////////////////////////////////////////////
// print stats for your tracker here
////////////////////////////////////////////////////////////////////

void    indramctr_print_stats(InDramCtr *m){
    char header[256];
    sprintf(header, "INDRAMCTR");

    printf("\n%s_NUM_RESET      \t : %llu",    header, m->s_num_reset);
    printf("\n%s_NUM_READS     \t : %llu",     header, m->s_num_reads);
    printf("\n%s_NUM_WRITES    \t : %llu",     header, m->s_num_writes);
    printf("\n%s_NUM_MITIGATE   \t : %llu",    header, m->s_mitigations);

    printf("\n"); 
}

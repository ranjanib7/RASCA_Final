#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "mgries.h"

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

MGries *mgries_new(uns num_entries, uns threshold, Addr bankID){
  MGries *m = (MGries *) calloc (1, sizeof (MGries));
  m->entries  = (MGries_Entry *) calloc (num_entries, sizeof(MGries_Entry));
  m->threshold = threshold;
  m->num_entries = num_entries;
  m->bankID = bankID;
  return m;
}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

void    mgries_reset(MGries *m){
  uns ii;

  //------ update global stats ------
  m->s_num_reset++;
  m->s_glob_spill_count += m->spill_count;

  //----- reset the structures ------
  m->spill_count       = 0;
  
  for(ii=0; ii < m->num_entries; ii++){
    m->entries[ii].valid = FALSE;
    m->entries[ii].addr  = 0;
    m->entries[ii].count = 0;
  }  
}

////////////////////////////////////////////////////////////////////
// The rowAddr field is the row to be updated in the tracker
// returns TRUE if mitigation must be issued for the row
////////////////////////////////////////////////////////////////////

Flag  mgries_access(MGries *m, Addr rowAddr){
  Flag retval = FALSE;
  m->s_num_access++;

  //---- TODO: Access the tracker and install entry (update stats) if needed

  //---- TODO: Decide if mitigation is to be issued (retval)
  
  if(retval==TRUE){
    m->s_mitigations++;
  }
  
  return retval;
}

////////////////////////////////////////////////////////////////////
// print stats for the tracker
// DO NOT CHANGE THIS FUNCTION
////////////////////////////////////////////////////////////////////

void    mgries_print_stats(MGries *m){
    char header[256];
    sprintf(header, "MGRIES-%llu",m->bankID);

    printf("\n%s_NUM_RESET      \t : %llu",    header, m->s_num_reset);
    printf("\n%s_GLOB_SPILL_CT  \t : %llu",    header, m->s_glob_spill_count);
    printf("\n%s_NUM_ACCESS     \t : %llu",    header, m->s_num_access);
    printf("\n%s_NUM_INSTALL    \t : %llu",    header, m->s_num_install);
    printf("\n%s_NUM_MITIGATE   \t : %llu",    header, m->s_mitigations);

    printf("\n"); 
}

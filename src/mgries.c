#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "mgries.h"

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

MGries *mgries_new(uns entries, uns threshold){
  MGries *m = (MGries *) calloc (1, sizeof (MGries));
  m->entries  = (MGries_Entry *) calloc (entries, sizeof(MGries_Entry));
  m->threshold = threshold;
  m->num_entries = entries;
  return m;
}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

void    mgries_reset(MGries *m){
  uns ii;

  //------ update global stats ------
  m->s_num_reset++;
  m->s_spill_count += m->spill_count;

  //----- reset the structures ------
  m->spill_count       = 0;
  
  for(ii=0; ii < m->num_entries; ii++){
    m->entries[ii].valid = FALSE;
    m->entries[ii].addr  = 0;
    m->entries[ii].count = 0;
  }
  
}

////////////////////////////////////////////////////////////////////
// The addr field is the row address
// returns TRUE if mitigation must be issued
////////////////////////////////////////////////////////////////////

Flag  mgries_access(MGries *m, Addr addr){
  Flag retval = FALSE;
  m->s_num_access++;

  //---- TODO: access the tracker and install entry if needed


  
  if(retval==TRUE){
    m->s_mitigations++;
  }
  
  return retval;
}

////////////////////////////////////////////////////////////////////
// print stats for your tracker here
////////////////////////////////////////////////////////////////////

void    mgries_print_stats(MGries *m){
 

}

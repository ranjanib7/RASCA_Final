#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "mgries.h"

using namespace std;

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

  Flag hit = FALSE, miss_spill_match = FALSE;
  Flag retval = FALSE;

  m->s_num_access++;

  //---- TODO: Access the tracker and install entry (update stats) if needed
  // Is it already in the table?
  for(uns i = 0; i < m->num_entries; i++) {
    if((m->entries[i].addr == rowAddr) && m->entries[i].valid) {
      m->entries[i].count++;
      hit = TRUE;
      // Set is_near_aggressor flag if this row was accessed as a near aggressor
      m->entries[i].secondary = (m->entries[i].count/58);
      m->entries[i].count += 2*(m->entries[i].secondary);
      break;
    }
  }

  // any entry with the same value as the spillover count
  if(!hit) {
    // if it isn't in there, it may need to be installed
    for(uns i = 0; i < m->num_entries; i++) {
      if(m->entries[i].valid == 0) {
        // install the new entry
	m->entries[i].valid = TRUE;
	m->entries[i].addr = rowAddr;
	m->entries[i].count++;
    	hit = TRUE;
	m->s_num_install++;
	m->entries[i].secondary = (m->entries[i].count/58);
        m->entries[i].count += 2*(m->entries[i].secondary);
	break;
      }
    }
    if(!hit) {
      // any entry with the same value as spillover count
      for(uns j = 0; j < m->num_entries; j++) {
        if((m->entries[j].count == m->spill_count) && m->entries[j].valid) {
          // install a new item id in that entry
	  m->entries[j].addr = rowAddr;
	  m->entries[j].count++;
	  miss_spill_match = TRUE;
	  m->s_num_install++;
	  m->entries[j].secondary = (m->entries[j].count/58);
          m->entries[j].count += 2*(m->entries[j].secondary);
	  break;
        }
      }

      // no entry has the same spillover count value so increment
      if(miss_spill_match == FALSE) {
        m->spill_count++;
      }
    }
  }

  //---- TODO: Decide if mitigation is to be issued (retval)

  for(uns k = 0; k < m->num_entries; k++) {
    if((m->entries[k].count%m->threshold == 0) && (m->entries[k].count >= m->threshold) && (m->entries[k].valid) && (m->entries[k].addr == rowAddr)) {
      retval = TRUE;
      break;
    }
  }

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

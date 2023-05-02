#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>

#include "externs.h"
#include "memsys.h"
#include "mcore.h"

using namespace std;

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

MemSys *memsys_new(uns num_threads, uns64 rh_threshold){
    MemSys *m = (MemSys *) calloc (1, sizeof (MemSys));
    m->num_threads    = num_threads;
    m->rh_threshold   = rh_threshold/2;

    // init main memory DRAM
    m->mainmem = dram_new(MEM_SIZE_MB*1024*1024, MEM_CHANNELS, MEM_BANKS, MEM_PAGESIZE, 
			  MEM_T_ACT, MEM_T_CAS, MEM_T_RAS, MEM_T_RP, MEM_T_BURST);
    sprintf(m->mainmem->name, "MEM");
    m->lines_in_mainmem_rbuf = MEM_PAGESIZE/LINESIZE; // static

    //-- TASK A --
    //-- TODO: Initialize Misra Gries Tracker

    //-- Think? How many trackers do you need? 
    //HINT: Look at mgries_new(..) func or memsys_print_stats func.    
    /* int num_mg_trackers = X; */
    /* m->mgries_t = (MGries**) calloc(num_mg_trackers,sizeof(MGries*)); */

    /* for(int i=0; i<num_mg_trackers; i++){ */
    //-- Think? What should be the threshold for mgries_new? 
    /*   m->mgries_t[i] = mgries_new(...);   */
    /* } */
    int num_mg_trackers = MEM_BANKS;
    m->mgries_t = (MGries**)calloc(num_mg_trackers, sizeof(MGries*));

    uns w = (0.064 * 4 * 1000000000) / (MEM_T_RAS + MEM_T_RP);
    uns num_entries = (w / m->rh_threshold);
    for (int i = 0; i < num_mg_trackers; i++) {
        //-- Think? What should be the threshold for mgries_new?
        m->mgries_t[i] = mgries_new(num_entries, m->rh_threshold, i);
    }

    //-- TASK B --
    //-- TODO: Initialize CRA Counters in DRAM

    //-- Think? How many counters needed? What should be the threshold for cra_ctr_new()?
    /* m->cra_t = (CraCtr*)  calloc(1,sizeof(CraCtr)); */
    /* m->cra_t = cra_ctr_new(...);    */

    // AQUA
    //int num_rows = 52182;  // assuming that A = rh/2 = 512 and time taken to move a quarantine row = 1 read + 1 write = 234 cycles
    uns64 rqa_rows = 23053;
    m->rqa = rqa_new(rqa_rows, rh_threshold/2);
    uns64 fpt_rows = 2097152;
    m->fpt = ptrtables_new(fpt_rows);
    uns64 rpt_rows = 23053;
    m->rpt = ptrtables_new(rpt_rows);
    return m;
}


//////////////////////////////////////////////////////////////////////////
// NOTE: ACCESSES TO THE MEMORY USE LINEADDR OF THE CACHELINE ACCESSED
//////////////////////////////////////////////////////////////////////////

uns64 memsys_access(MemSys *m, Addr lineaddr,  uns tid, uns64 in_cycle){
  uns64 total_delay=0;
  uns64 memdelay=0;

  memdelay=memsys_dram_access(m, lineaddr, in_cycle, &(m->dram_acc_info));
  total_delay=memdelay;

  // stat collection happens below this line
  m->s_totaccess++;
  m->s_totdelaysum+=total_delay;

  
  // return delay
  return total_delay;
}



////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

void memsys_print_stats(MemSys *m)
{
    char header[256];
    sprintf(header, "MSYS");

    uns64 avg_delay=0;
    if(m->s_totaccess){
      avg_delay=m->s_totdelaysum/m->s_totaccess;
    }

    dram_print_stats(m->mainmem);

    printf("\n%s_TOT_ACCESS      \t : %llu",    header, m->s_totaccess);
    printf("\n%s_AVG_DELAY       \t : %llu",    header, avg_delay);
    printf("\n%s_RH_TOT_MITIGATE \t : %llu",    header, m->s_tot_mitigate);
    printf("\n");
    
    if(m->mgries_t)
      for(uns i=0; i< m->mainmem->num_banks; i++){
	mgries_print_stats(m->mgries_t[i]);
      }

    if(m->cra_t)
      cra_ctr_print_stats(m->cra_t);

    if(m->rqa)
      rqa_print_stats(m->rqa);
}


//////////////////////////////////////////////////////////////////////////
// NOTE: ACCESSES TO THE MEMORY USE LINEADDR OF THE CACHELINE ACCESSED
//-- for input ACTinfo* act_info: if the pointer is not NULL, then *act_info is updated, else not 
////////////////////////////////////////////////////////////////////
uns64  memsys_dram_access(MemSys *m, Addr lineaddr, uns64 in_cycle, ACTinfo *act_info){
  DRAM_ReqType type=DRAM_REQ_RD;
  double burst_size=1.0; // one cache line
  uns64 delay=0;

  if((in_cycle - m->rqa->last_reset)/4000000 >= 64) {
    m->rqa->last_reset = in_cycle;
    rqa_reset(m->rqa);
  }

  delay += rqa_check_delay();
  Flag outcome = mgries_access(m->mgries_t[act_info->bankID], lineaddr/(m->mainmem->lines_in_rowbuf));
  if(outcome == TRUE) {
    delay += rqa_migrate(m->rqa, lineaddr/(m->mainmem->lines_in_rowbuf), m->fpt, m->rpt);
  }
  else {
    delay += dram_service(m->mainmem, lineaddr/(m->mainmem->lines_in_rowbuf), type, burst_size, in_cycle, act_info);
    rqa_drain_row(m->rqa, m->rpt, m->fpt);
  }

  /*if(act_info != NULL && act_info->isACT) {
    Flag outcome = mgries_access(m->mgries_t[act_info->bankID], lineaddr/(m->mainmem->lines_in_rowbuf));
    if(outcome == TRUE) {
      memsys_rh_mitigate(m, lineaddr/(m->mainmem->lines_in_rowbuf), in_cycle);
    }
  }*/


  return delay;
}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
void  memsys_rh_mitigate(MemSys *m, Addr rowID, uns64 in_cycle){

  DRAM_ReqType type=DRAM_REQ_RD;
  double burst_size=1.0; // one cache line
  uns64 delay=0;

  //-- neighbors --
  Addr row_prev_lineaddr, row_next_lineaddr ; 
  dram_get_neighbor_lineaddr(m->mainmem, rowID, &row_prev_lineaddr, &row_next_lineaddr);

  //-- Activate neighbors --
  if(row_prev_lineaddr != (Addr)-1){
    delay += dram_service(m->mainmem, row_prev_lineaddr, type, burst_size, in_cycle, NULL);
  }
  if(row_next_lineaddr != (Addr)-1){
    delay += dram_service(m->mainmem, row_next_lineaddr, type, burst_size, in_cycle, NULL);
  }

  //Update statistic
  m->s_tot_mitigate++;
  return ;
}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////


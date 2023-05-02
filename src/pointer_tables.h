#ifndef POINTER_TABLES_H
#define POINTER_TABLES_H

#include "global_types.h"

typedef struct PtrTable_Entry PtrTable_Entry;
typedef struct PtrTable PtrTable;

///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

struct PtrTable_Entry {
  Flag			valid;
  Addr			addr;		// Mapped address
};

struct PtrTable {
  uns64			numRows;
  PtrTable_Entry	*entries;

};

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

PtrTable *ptrtables_new(uns64 entries);
Addr fpt_check(PtrTable *pt, Addr rowAddr);
Addr rpt_access(PtrTable *pt, Addr rowAddr);
void fpt_update(PtrTable *pt, Addr new_addr, Addr old_addr);
void rpt_update(PtrTable *pt, Addr old_addr, Addr new_addr);

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
#endif // POINTER_TABLES_H

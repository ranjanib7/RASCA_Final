#include "mitigation.h"

void MurmurHash3_x64_128(const void* key, const int len, const uint32_t seed, void* out);

void countingBloomFilter(uns num_entries, uns max_count, uns num_hashes) {
	CountingBloomFilter* cbf = (CountingBloomFilter*)calloc(1, sizeof(CountingBloomFilter));
	cbf->numHashes = num_hashes;
	cbf->maxCount = max_count;
	cbf->numEntries = num_entries;
	cbf->entries = (uns*)calloc(numEntries, sizeof(uns));
}

inline uns64 nthHash(uint8_t n, uns64 hashA, uns64 hashB, uns64 filterSize) {
	return (hashA + n * hashB) % filterSize;
}

void cbf_add(CountingBloomFilter *cbf, uns rowAddr) {
	uns data[4] = { uns(rowAddr * 0xff), uns((rowAddr >> 8) & 0xff), uns((rowAddr >> 16) & 0xff), uns((rowAddr >> 24) & 0xff };
	uns64 hashValues[2];
	MurmurHash3_x64_128(data, 4, 0xdeadbeef, hashValues);
	for (int n = 0; n < cbf->numHashes; n++) {
		uns64 index = (hashValues[0] + n * hashValues[1]) % cbf->numEntries;
		cbf->entries[index]++;
	}
}

bool cbf_query(countingBloomFilter* cbf, uns rowAddr) {
	uns data[4] = { uint8_t(rowAddr & 0xff), uint8_t((rowAddr >> 8) & 0xff),
						uint8_t((rowAddr >> 16) & 0xff), uint8_t((rowAddr >> 24) & 0xff) };
	uns64 hashValues[2];
	MurmurHash3_x64_128(data, 4, 0xdeadbeef, hashValues);
	for (int n = 0; n < cbf->numHashes; n++) {
		if (!cbf->entries[nthHash(n, hashValues[0], hashValues[1], cbf->numEntries)]) {
			return false;
		}
	}
	return true;
}

void del(countingBloomFilter *cbf, uns rowAddr) {
	uint8_t data[4] = { uint8_t(rowAddr & 0xff), uint8_t((rowAddr >> 8) & 0xff),
						uint8_t((rowAddr >> 16) & 0xff), uint8_t((rowAddr >> 24) & 0xff) };
	uns64 hashValues[2];
	MurmurHash3_x64_128(data, 4, 0xdeadbeef, hashValues);
	for (int n = 0; n < cbf->numHashes; n++) {
		if (cbf->entries[nthHash(n, hashValues[0], hashValues[1], cbf->numEntries)]) {
			cbf->entries[nthHash(n, hashValues[0], hashValues[1], cbf->numEntries)]--;
		}
	}
}

void aqua_new(uns numEntries, uns numRows, uns threshold, numRanks, uns numBanks) {

	RQA* rqa = (RQA*)calloc(1, sizeof(RQA));
	rqa->numQREntries = numEntries;
	rqa->threshold = threshold;
	rqa->indirection = (IndirectionTables*)calloc(numRanks, sizeof(IndirectionTables*));
	for (int i = 0; i < numRanks; i++) {
		indirection[i] = (IndirectionTables)calloc(numBanks, sizeof(IndirectionTables));
		for (int j = 0; j < numBanks; j++) {
			indirection[i][j].row_to_qr_t = (Entry*)calloc(numRows, sizeof(Entry));
			indirection[i][j].qr_to_row_t = (Entry*)calloc(rqa->numQREntries, sizeof(Entry));
			indirection[i][j].nextFreeQR_ID = 0;
			indirection[i][j].prevQRHead_ID = 0;
			indirection[i][j].QRTail_ID = 0;
		}
	}

	move_to_qr = 0;
	move_within_qr = 0;
	move_to_qr_remove = 0;
	move_within_qr_remove = 0;

	num_resets = 0;
	num_accesses = 0;
}

void decision_reset() {
	decision.validAddrs = 0;
	decision.remappedAddr = -1;
	decision.mitigate = false;
	decision.delay = 0;
	decision.MC_dela = 0;
	decision.SRAM_delay = 0;
	for (int i = 0; i < 6; i++) {
		decision.isActionSwap[i] = false;
		decision.addrs[i] = false;
	}
}

Decision& aqua_access(uns64 in_cycle, Addr addr) {

	num_accesses++;
	decision_reset();

	if (in_cycle - last_reset >= 64000000000) {
		last_reset = in_cycle;
		aqua_reset();
	}

	addQCheckDelay();

	if (decision.mitigate) {
		quarantineRow();
	}

	return decision;

}

void addQCheckDelay(Addr rowAddr, uns bankID, uns rankID) {
	IndirectionTables* indirectStructs = &(indirection[rankID][bankID]);
	decision.SRAM_delay += ;
}

bool aquaRemoveRow(Addr qrt_id, uns bankID, uns rankID) {
	IndirectionTables* indirStructs = &(indirection[rankID][bankID]);
	// remove row from quarantine region
	uns removed_rowID = indirStructs->qr_to_row_t[qrt_id].value;
	indirStructs->qr_to_row_t[qrt_id].valid = false;

	assert(indirStructs->row_to_qr_t[removed_rowID].valid);
	indirStructs->row_to_qr_t[removed_rowID].valid = false;
}

void aqua_quarantine_row(Addr rowAddr, uns bankID, uns rankID) {
	IndirectionTables* indirStructs = &(indirection[rankID][bankID]);
	uint8_t move_op = MOVE_TO_QR;

	// if the row is already in quarantine, the mapping only needs to be updates
	bool originalQuarantine = false;
	uns orig_row_qr_id;
	if (indirStructs->row_to_qr_t[rowAddr].valid) {
		move_op |= MOVE_WITHIN_QR;
		orig_row_qr_id = indirStructs->row_to_qr_t[rowAddr].value;
		originalQurantine = true;
	}
	else {
		// add entry
	}

	indirStructs->row_to_qr_t[rowAddr].valid = true;
	indirStructs->row_to_qr_t[rowAddr].value = indirStructs->nextFreeQR_ID;

	if (indirStructs->qr_to_row_t[indirStructs->nextFreeQR_ID].valid) {

		move_op |= REMOVE;
		uns removed_rowID = indirStructs->qr_to_row_t[indirStructs->nextFreeQR_ID].value;
	}

	indirStructs->qr_to_row_t[indirStructs->nextFreeQR_ID].valid = true;
	indirStructs->qr_to_row_t[indirStructs->nextFreeQR_ID].value = rowAddr;

	if (indirStructs->nextFreeQR_ID == indirStructs->QRTail_ID &&
		indirStructs->nextFreeQR_ID != indirStructs->prevQRHead_ID) {
		// Head wrapped around and reached tail, increment it
		indirStructs->QRTail_ID = (indirStructs->QRTail_ID + 1) % numQREntries;
	}

	// Increment next free QR ID
	indirStructs->nextFreeQR_ID = (indirStructs->nextFreeQR_ID + 1) % numQREntries;

	if (move_op == MOVE_TO_QR) {
		s_move_to_qr++;
	}
	else if (move_op == MOVE_WITHIN_QR) {
		s_move_within_qr++;
	}
	else if (move_op == (MOVE_TO_QR | REMOVE)) {
		s_move_to_qr_remove++;
	}
	else {
		s_move_within_qr_remove++;
	}
}

void aqua_reset() {
	detector->reset();
	num_resets++;
	for (int = 0; i < numRanks; i++) {
		for (int j = 0; j < numBanks; j++) {
			IndirectionTables* indirStructs - &(indirection[i][j]);
			indirStructs->prevQRHead_ID = indirStructs->nextFreeQR_ID;
		}
	}
}
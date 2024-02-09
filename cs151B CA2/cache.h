#include <iostream>
#include <bitset>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <cmath>
using namespace std;

#define L1_CACHE_SETS 16
#define L2_CACHE_SETS 16
#define L2_CACHE_WAYS 8
#define VICTIM_SIZE 4
#define MEM_SIZE 4096
#define BLOCK_SIZE 4 // bytes per block
#define DM 0
#define SA 1

struct cacheBlock
{
	int tag;		  // you need to compute offset and index to find the tag.
	int lru_position; // for SA only
	int data;		  // the actual data stored in the cache/memory
	bool valid;
	// add more things here if needed
};

struct Stat
{
	double missL1;
	double missL2;
	double accL1;
	double accL2;
	double accVic;	 // number of accesses (both reads and writes) to the Victim cache
	double missVic; // number of misses in the Victim cache
};

class cache
{
private:
	cacheBlock L1[L1_CACHE_SETS];				 // 1 set per row.
	cacheBlock L2[L2_CACHE_SETS][L2_CACHE_WAYS]; // x ways per row
	cacheBlock Vict[VICTIM_SIZE];
	Stat myStat;
	// add more things here
public:
	cache();
	bool controller(bool MemR, bool MemW, int *data, int adr, int *myMem);
	bool lookForL1(string adr);
	int lookForVict(string real_adr);
	int lookForL2(string adr);
	int findWayOFL2(int index);
	void BringtoL1(string binaryAddress, int newData);
	void BringToL2(int index, int newTag, int newData);
	void BringtoVict(int newTag, int newData);
	double getL1Misses() const { return myStat.missL1; }
    double getL1Accesses() const { return myStat.accL1; }
    double getL2Misses() const { return myStat.missL2; }
    double getL2Accesses() const { return myStat.accL2; }
	double getVictAccesses() const { return myStat.accVic; }
	double getVictMiss() const { return myStat.missVic; }
	// add more functions here ...
};




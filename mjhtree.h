#ifndef MJHTREE_H
#define MJHTREE_H

#include "mahjong.h"

#if NUM_TILE_IN_HAND < 0x100
typedef unsigned char mjtindex;
#elif NUM_TILE_IN_HAND < 0x10000
typedef unsigned short mjtindex;
#elif NUM_TILE_IN_HAND < 0x100000000
typedef unsigned long mjtindex;
#else
typedef unsigned long long mjtindex;
#endif

typedef char mjtcolor;

struct mjhtree {
	mjtile const*tiles;
	mjtindex parent[NUM_TILE_IN_HAND + 1];
	mjtindex child[NUM_TILE_IN_HAND + 1][2];
	mjtcolor color[NUM_TILE_IN_HAND + 1];
};

void MjhtreeInit(struct mjhtree*tree, mjtile const*tiles_new);
void MjhtreeDelete(struct mjhtree*tree, mjtindex index);
void MjhtreeUpdate(struct mjhtree*tree, mjtindex index);

mjtindex MjhtreeFindTile(struct mjhtree const*tree, mjtile t);
mjtindex MjhtreeFindKind(struct mjhtree const*tree, mjkind k);

char MjhtreeIsConsistent(struct mjhtree*tree, mjhand const hand);

#endif

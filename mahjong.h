#ifndef MAHJONG_H
#define MAHJONG_H

#ifndef NUM_TILE_EACHKIND
#define NUM_TILE_EACHKIND 4U
#endif
#ifndef NUM_SUIT
#define NUM_SUIT 3U
#endif
#ifndef NUM_KIND_EACHSUIT
#define NUM_KIND_EACHSUIT 9U
#endif
#ifndef NUM_KIND_CHAR
#define NUM_KIND_CHAR 7U
#endif
#ifndef NUM_TILE_FLOWER
#define NUM_TILE_FLOWER 8U
#endif

#ifndef NUM_MJPLAYER
#define NUM_MJPLAYER 4U
#endif
#ifndef NUM_GROUP_IN_HAND
#define NUM_GROUP_IN_HAND 4U
#endif
#ifndef NUM_TILE_DEAD
#define NUM_TILE_DEAD 0U
#endif

#define KIND_FIRSTCHAR (NUM_SUIT * NUM_KIND_EACHSUIT)
#define KIND_FIRSTFLOWER (KIND_FIRSTCHAR + NUM_KIND_CHAR)
#define KIND_FIRSTCHAR_BUFFED (NUM_SUIT * (NUM_KIND_EACHSUIT + 2U) + 2U)
#define KIND_FLOWER_BUFFED (KIND_FIRSTCHAR_BUFFED + NUM_KIND_CHAR * 3U)
#define KIND_LASTCHAR_BUFFED (KIND_FLOWER_BUFFED - 3U)
#define NUM_TILE_TOTAL (((NUM_KIND_EACHSUIT * NUM_SUIT) + NUM_KIND_CHAR) * NUM_TILE_EACHKIND + NUM_TILE_FLOWER)
#define NUM_KIND_BUFFED (KIND_FLOWER_BUFFED + 1U)
#define NUM_TILE_IN_HAND (NUM_GROUP_IN_HAND * 3U + 1U)

#if NUM_TILE_TOTAL < 0x100
typedef unsigned char mjtile;
#elif NUM_TILE_TOTAL < 0x10000
typedef unsigned short mjtile;
#elif NUM_TILE_TOTAL < 0x100000000
typedef unsigned long mjtile;
#else
typedef unsigned long long mjtile;
#endif

#if NUM_KIND_BUFFED < 0x100
typedef unsigned char mjkind;
#elif NUM_KIND_BUFFED < 0x10000
typedef unsigned short mjkind;
#elif NUM_KIND_BUFFED < 0x100000000
typedef unsigned long mjkind;
#else
typedef unsigned long long mjkind;
#endif

#if NUM_TILE_EACHKIND < 0x100
typedef unsigned char numkind;
#elif NUM_TILE_EACHKIND < 0x10000
typedef unsigned short numkind;
#elif NUM_TILE_EACHKIND < 0x100000000
typedef unsigned long numkind;
#else
typedef unsigned long long numkind;
#endif

typedef numkind mjhand[NUM_KIND_BUFFED * 6U + 2U];

static inline mjkind MjBuffKind(mjkind i) {
	if (i < KIND_FIRSTCHAR) return i + (i / NUM_KIND_EACHSUIT + 1U) * 2U;
	else if (i < KIND_FIRSTFLOWER) return (i - KIND_FIRSTCHAR) * 3U + KIND_FIRSTCHAR_BUFFED;
	else return KIND_FLOWER_BUFFED;
}

static inline mjkind MjUnbuffKind(mjkind i) {
	if (i < KIND_FIRSTCHAR_BUFFED)  return i - (i / (NUM_KIND_EACHSUIT + 2U) + 1U) * 2U;
	else if (i < KIND_FLOWER_BUFFED) return (i - KIND_FIRSTCHAR_BUFFED) / 3U + KIND_FIRSTCHAR;
	else return KIND_FIRSTFLOWER;
}

#endif
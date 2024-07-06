#ifndef MJTABLE_H
#define MJTABLE_H

#include"mahjong.h"

enum mjtablestate {
	MJS_N,
	MJS_DRAW,
	MJS_DISCARD,
	MJS_CHIPON,
	MJS_KAN_R,
	MJS_KAN_C,
};

struct mjtable {
	mjhand hands[NUM_MJPLAYER];
	char handtiles[NUM_MJPLAYER][NUM_TILE_TOTAL];
	char ready[NUM_MJPLAYER];
	unsigned char meld[NUM_MJPLAYER];
	unsigned char quad[NUM_MJPLAYER];
	unsigned char flower[NUM_MJPLAYER];

	mjtile handtileLinkF[NUM_MJPLAYER][NUM_TILE_TOTAL + 1];
	mjtile handtileLinkB[NUM_MJPLAYER][NUM_TILE_TOTAL + 1];

	enum mjtablestate state;
	mjtile posFront, posBack;
	unsigned char player;
	unsigned char playerDealer;

	struct {
		char win;
		char ready;
		unsigned char kan;
		char flower;

		mjkind kanCandidate[(NUM_TILE_IN_HAND + 1U) / 4U];
	} optionSelf;
	struct {
		char win[3];
		char kan[3];
		char pon[3];
		char chi[3];
	} optionOther;
	char(*predicateWin)(struct mjtable const*, unsigned char, void*);
	void*contextPredicateWin;
	char(*predicateReady)(struct mjtable const*, unsigned char, void*);
	void*contextPredicateReady;

	mjtile mount[NUM_TILE_TOTAL + NUM_TILE_TOTAL % 2U];
	mjkind kindFocus;
	mjtile tileFocus;
	mjtile tilesLastMeld[4];
};

void MjtableInit(
	struct mjtable*t,
	char(*predwin)(struct mjtable const*, unsigned char, void*),
	char(*predready)(struct mjtable const*, unsigned char, void*));

void MjtableDiscard(struct mjtable*t, mjkind kind);
void MjtablePass(struct mjtable*t);
void MjtableReady(struct mjtable*t);
void MjtableChi(struct mjtable*t, int type);
void MjtablePon(struct mjtable*t, int who);
void MjtableKanOther(struct mjtable*t, int who);
void MjtableKanSelf(struct mjtable*t, int which);
void MjtableFlower(struct mjtable*t);
/*Manually shuffle and set dealer before this*/
void MjtableNewround(struct mjtable*t);

void MjtableDiscardT(struct mjtable*t, mjtile tile);

#endif
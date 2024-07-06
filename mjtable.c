#include"mjtable.h"

#include<string.h>
#include<stddef.h>
#include<assert.h>

#if NUM_TILE_EACHKIND > 4
#error Ankan or kakan problem
#endif

static char CanWinDefault(struct mjtable const*t, unsigned char who, void*context) { return 0; }
static char CanReadyDefault(struct mjtable const*t, unsigned char who, void*context) { return 0; }

static mjtile FindTile(char const handtile[NUM_TILE_TOTAL], mjkind kind) {
	mjtile tile = MjUnbuffKind(kind) * NUM_TILE_EACHKIND;
	if (tile >= NUM_TILE_TOTAL) return assert(0), tile;
	while (!handtile[tile]) {
		++tile;
		if (tile >= NUM_TILE_TOTAL) return assert(0), MjUnbuffKind(kind) * NUM_TILE_EACHKIND;
	}
	assert(MjBuffKind(tile / NUM_TILE_EACHKIND) == kind);
	return tile;
}

static void RemoveTile(char handtile[NUM_TILE_TOTAL], mjtile*linkF, mjtile*linkB, mjkind kind, unsigned count, mjtile*tilesLastMeld) {
	if (count > 0) {
		mjtile tile = MjUnbuffKind(kind) * NUM_TILE_EACHKIND;
		if (tile >= NUM_TILE_TOTAL) return assert(0);
		if (handtile[tile]) {
			count -= 1;
			handtile[tile] = 0;
			linkF[linkB[linkF[tile]] = linkB[tile]] = linkF[tile];
			*tilesLastMeld = tile;
			++tilesLastMeld;
		}
		while (count > 0) {
			++tile;
			if (tile >= NUM_TILE_TOTAL) return assert(0);
			if (handtile[tile]) {
				count -= 1;
				handtile[tile] = 0;
				linkF[linkB[linkF[tile]] = linkB[tile]] = linkF[tile];
				*tilesLastMeld = tile;
				++tilesLastMeld;
			}
		}
		assert(MjBuffKind(tile / NUM_TILE_EACHKIND) == kind);
	}
}

void MjtableInit(
	struct mjtable*t,
	char(*predwin)(struct mjtable const*, unsigned char, void*),
	char(*predready)(struct mjtable const*, unsigned char, void*)) {
	t->predicateWin = predwin ? predwin : CanWinDefault;
	t->predicateReady = predready ? predready : CanReadyDefault;
	for (mjtile i = 0; i < (mjtile)NUM_TILE_TOTAL; ++i) t->mount[i] = i;
}

static void MjtableOptionSelf(struct mjtable*t) {
	numkind*const hand_player = t->hands[t->player];
	memset(&t->optionSelf, 0, sizeof(t->optionSelf));

	if (t->ready[t->player]) {
		if (hand_player[t->kindFocus + NUM_KIND_BUFFED] > 0)
			t->optionSelf.kanCandidate[t->optionSelf.kan++] = t->kindFocus;
		else if (hand_player[t->kindFocus] >= 3) {
			hand_player[t->kindFocus] -= 3;
			if (t->predicateReady(t, t->player, t->contextPredicateReady))
				t->optionSelf.kanCandidate[t->optionSelf.kan++] = t->kindFocus;
			hand_player[t->kindFocus] += 3;
		}
		hand_player[t->kindFocus] += 1;
		if (t->predicateWin(t, t->player, t->contextPredicateWin))
			t->optionSelf.win = 1;
		hand_player[t->kindFocus] -= 1;
		if (t->kindFocus == KIND_FLOWER_BUFFED) t->optionSelf.flower = 1;
	}
	else {
		switch (t->state) {
		default: break;
		case MJS_DRAW:
			hand_player[t->kindFocus] += 1;
			for (int i = 0; i < KIND_FIRSTFLOWER; ++i) {
				mjkind const k_kan = MjBuffKind(i);
				if (hand_player[k_kan] > 0) {
					if (hand_player[k_kan + NUM_KIND_BUFFED] > 0 || hand_player[k_kan] >= 4) {
						t->optionSelf.kanCandidate[t->optionSelf.kan++] = k_kan;
					}
				}
			}
			if (t->predicateReady(t, t->player, t->contextPredicateReady))
				t->optionSelf.ready = 1;
			if (t->predicateWin(t, t->player, t->contextPredicateWin))
				t->optionSelf.win = 1;
			if (hand_player[KIND_FLOWER_BUFFED] > 0) t->optionSelf.flower = 1;
			hand_player[t->kindFocus] -= 1;
			break;
		case MJS_CHIPON:
			for (int i = 0; i < KIND_FIRSTFLOWER; ++i) {
				mjkind const k_kan = MjBuffKind(i);
				if (hand_player[k_kan] > 0) {
					if (hand_player[k_kan + NUM_KIND_BUFFED] > 0 || hand_player[k_kan] >= 4) {
						t->optionSelf.kanCandidate[t->optionSelf.kan++] = k_kan;
					}
				}
			}
			if (t->predicateReady(t, t->player, t->contextPredicateReady))
				t->optionSelf.ready = 1;
			if (hand_player[KIND_FLOWER_BUFFED] > 0) t->optionSelf.flower = 1;
			break;
		}
	}
}

static void MjtableOptionOther(struct mjtable*t) {
	memset(&t->optionOther, 0, sizeof(t->optionOther));

	if (t->kindFocus != KIND_FLOWER_BUFFED) {
		switch (t->state) {
		default: break;
		case MJS_DISCARD:
			for (int i = 0; i < 3; ++i) {
				unsigned char const player = (t->player + i + 1) % NUM_MJPLAYER;
				numkind*const hand_player = t->hands[player];

				if (t->ready[player]) {
					if (hand_player[t->kindFocus] >= 3) {
						hand_player[t->kindFocus] -= 3;
						if (t->predicateReady(t, player, t->contextPredicateReady))
							t->optionOther.kan[i] = 1;
						hand_player[t->kindFocus] += 3;
					}
				}
				else {
					if (hand_player[t->kindFocus] >= 2) {
						t->optionOther.pon[i] = 1;
						if (hand_player[t->kindFocus] >= 3) t->optionOther.kan[i] = 1;
					}
				}

				hand_player[t->kindFocus] += 1;
				if (t->predicateWin(t, player, t->contextPredicateWin))
					t->optionOther.win[i] = 1;
				hand_player[t->kindFocus] -= 1;
			}

			{
				unsigned char const player = (t->player + 1) % NUM_MJPLAYER;
				if (!t->ready[player]) {
					numkind const*const hand_player = t->hands[(t->player + 1) % NUM_MJPLAYER];
					if (hand_player[t->kindFocus + 1] > 0 && hand_player[t->kindFocus + 2] > 0)
						t->optionOther.chi[0] = 1;
					if (hand_player[t->kindFocus - 1] > 0 && hand_player[t->kindFocus + 1] > 0)
						t->optionOther.chi[1] = 1;
					if (hand_player[t->kindFocus - 2] > 0 && hand_player[t->kindFocus - 1] > 0)
						t->optionOther.chi[2] = 1;
				}
			}
			break;
		case MJS_KAN_R:
			for (int i = 0; i < 3; ++i) {
				unsigned char const player = (t->player + i + 1) % NUM_MJPLAYER;
				numkind*const hand_player = t->hands[player];
				hand_player[t->kindFocus] += 1;
				if (t->predicateWin(t, player, t->contextPredicateWin))
					t->optionOther.win[i] = 1;
				hand_player[t->kindFocus] -= 1;
			}
			break;
		}
	}
}

void MjtableNewround(struct mjtable*t) {
	memset(t, 0, offsetof(struct mjtable, flower) + sizeof(t->flower));

	t->posFront = NUM_TILE_IN_HAND * NUM_MJPLAYER + 1;
	t->posBack = NUM_TILE_TOTAL - 1;
	t->player = t->playerDealer;
#if NUM_TILE_TOTAL % 2 == 1
		t->mount[NUM_TILE_TOTAL] = t->mount[NUM_TILE_TOTAL - 1];
#endif

	for (int i = 0; i < NUM_MJPLAYER; ++i) {
		unsigned char const player = (t->player + i) % NUM_MJPLAYER;
        mjtile*const linkF = t->handtileLinkF[player];
        mjtile*const linkB = t->handtileLinkB[player];
        linkF[NUM_TILE_TOTAL] = NUM_TILE_TOTAL;
        linkB[NUM_TILE_TOTAL] = NUM_TILE_TOTAL;
		for (int j = 0; j < NUM_TILE_IN_HAND; ++j) {
			mjtile const tile = t->mount[i * NUM_TILE_IN_HAND + j];
			t->hands[player][MjBuffKind(tile / NUM_TILE_EACHKIND)] += 1;
			t->handtiles[player][tile] = 1;
            linkB[NUM_TILE_TOTAL] = linkF[linkB[tile] = linkB[linkF[tile] = NUM_TILE_TOTAL]] = tile;
		}
	}
	t->kindFocus = MjBuffKind((t->tileFocus = t->mount[NUM_TILE_IN_HAND * NUM_MJPLAYER]) / NUM_TILE_EACHKIND);

	t->state = MJS_DRAW;
	MjtableOptionSelf(t);
}

void MjtableDiscard(struct mjtable*t, mjkind kind) {
	unsigned char const player = t->player;
	numkind*const hand_player = t->hands[player];
	if (hand_player[kind] > 0) {
		char*const handtile_player = t->handtiles[player];
		mjtile*const linkF = t->handtileLinkF[player];
		mjtile*const linkB = t->handtileLinkB[player];
		mjtile const tile = FindTile(handtile_player, kind);
		hand_player[kind] -= 1;
		handtile_player[tile] = 0;
        linkF[linkB[linkF[tile]] = linkB[tile]] = linkF[tile];
		if (t->state == MJS_DRAW) {
			hand_player[t->kindFocus] += 1;
			handtile_player[t->tileFocus] = 1;
            linkB[NUM_TILE_TOTAL] = linkF[linkB[t->tileFocus] = linkB[linkF[t->tileFocus] = NUM_TILE_TOTAL]] = t->tileFocus;
		}
		t->kindFocus = kind;
		t->tileFocus = tile;
	}
	else if (t->state == MJS_CHIPON) {
		char success = 0;
		for (int i = KIND_FIRSTFLOWER; i >= 0; --i) {
			char*const handtile_player = t->handtiles[player];
			mjtile*const linkF = t->handtileLinkF[player];
			mjtile*const linkB = t->handtileLinkB[player];
			mjkind const k_sch = MjBuffKind(i);
			if (hand_player[k_sch] > 0) {
				mjtile const tile = FindTile(handtile_player, k_sch);
				success = 1;
				hand_player[k_sch] -= 1;
				handtile_player[tile] = 0;
                linkF[linkB[linkF[tile]] = linkB[tile]] = linkF[tile];
				t->kindFocus = k_sch;
				t->tileFocus = tile;
				break;
			}
		}
		if (!success) return (void)(t->state = MJS_N);
	}
	t->state = MJS_DISCARD;
	MjtableOptionOther(t);
}

void MjtablePass(struct mjtable*t) {
	mjtile tile_drawn;
	if (t->posFront + NUM_TILE_DEAD <= t->posBack) {
		if (t->state == MJS_DISCARD) {
			t->player = (t->player + 1) % NUM_MJPLAYER;
#if NUM_TILE_DEAD <= 0
			if (t->posFront + NUM_TILE_DEAD == t->posBack && t->posFront % 2 == 0)
				tile_drawn = t->mount[t->posFront ^ 1];
			else
#endif
				tile_drawn = t->mount[t->posFront];
			t->posFront += 1;
		}
		else {
#if NUM_TILE_DEAD <= 0
			if (t->posFront + NUM_TILE_DEAD == t->posBack && t->posBack % 2 == 1)
				tile_drawn = t->mount[t->posFront];
			else
#endif
				tile_drawn = t->mount[t->posBack ^ 1];
			t->posBack -= 1;
		}
	}
	else {
		t->state = MJS_N;
		return;
	}

	t->kindFocus = MjBuffKind((t->tileFocus = tile_drawn) / NUM_TILE_EACHKIND);

	t->state = MJS_DRAW;
	MjtableOptionSelf(t);
}

void MjtableReady(struct mjtable*t) {
	numkind*const hand_player = t->hands[t->player];
	t->ready[t->player] = 1;
	t->optionSelf.kan = 0;
	switch (t->state) {
	default: break;
	case MJS_DRAW: {
		hand_player[t->kindFocus] += 1;
		for (int i = 0; i < KIND_FIRSTFLOWER; ++i) {
			mjkind const k_kan = MjBuffKind(i);
			if (hand_player[k_kan] > 0) {
				if (hand_player[k_kan + NUM_KIND_BUFFED] > 0) {
					hand_player[k_kan] -= 1;
					if (t->predicateReady(t, t->player, t->contextPredicateReady))
						t->optionSelf.kanCandidate[t->optionSelf.kan++] = k_kan;
					hand_player[k_kan] -= 1;
				}
				else if (hand_player[k_kan] >= 4) {
					hand_player[k_kan] -= 4;
					if (t->predicateReady(t, t->player, t->contextPredicateReady))
						t->optionSelf.kanCandidate[t->optionSelf.kan++] = k_kan;
					hand_player[k_kan] += 4;
				}
			}
		}
		hand_player[t->kindFocus] -= 1;
	} break;
	case MJS_CHIPON: {
		for (int i = 0; i < KIND_FIRSTFLOWER; ++i) {
			mjkind const k_kan = MjBuffKind(i);
			if (hand_player[k_kan] > 0) {
				if (hand_player[k_kan + NUM_KIND_BUFFED] > 0) {
					hand_player[k_kan] -= 1;
					if (t->predicateReady(t, t->player, t->contextPredicateReady))
						t->optionSelf.kanCandidate[t->optionSelf.kan++] = k_kan;
					hand_player[k_kan] -= 1;
				}
				else if (hand_player[k_kan] >= 4) {
					hand_player[k_kan] -= 4;
					if (t->predicateReady(t, t->player, t->contextPredicateReady))
						t->optionSelf.kanCandidate[t->optionSelf.kan++] = k_kan;
					hand_player[k_kan] += 4;
				}
			}
		}
	} break;
	}
}

void MjtableChi(struct mjtable*t, int type) {
	unsigned char const player_next = t->player = (t->player + 1) % NUM_MJPLAYER;
	numkind*const hand_player_next = t->hands[player_next];
	char*const handtile_player_next = t->handtiles[player_next];
	mjkind kind1, kind2;
	switch (type) {
	default: assert(0); return;
	case 0:
		kind1 = t->kindFocus + 1;
		kind2 = t->kindFocus + 2;
		hand_player_next[t->kindFocus + 1 + NUM_KIND_BUFFED * 2] += 1;
		break;
	case 1:
		kind1 = t->kindFocus - 1;
		kind2 = t->kindFocus + 1;
		hand_player_next[t->kindFocus + NUM_KIND_BUFFED * 2] += 1;
		break;
	case 2:
		kind1 = t->kindFocus - 2;
		kind2 = t->kindFocus - 1;
		hand_player_next[t->kindFocus - 1 + NUM_KIND_BUFFED * 2] += 1;
		break;
	}
	hand_player_next[kind1] -= 1;
	hand_player_next[kind2] -= 1;
	{
		mjtile*const linkF = t->handtileLinkF[player_next];
		mjtile*const linkB = t->handtileLinkB[player_next];
		RemoveTile(handtile_player_next, linkF, linkB, kind1, 1, t->tilesLastMeld);
		RemoveTile(handtile_player_next, linkF, linkB, kind2, 1, t->tilesLastMeld + 1);
	}

	t->meld[t->player] += 1;
	t->state = MJS_CHIPON;
	MjtableOptionSelf(t);
}

void MjtablePon(struct mjtable*t, int who) {
	unsigned char const player_next = t->player = (t->player + who + 1) % NUM_MJPLAYER;
	numkind*const hand_player_next = t->hands[player_next];
	hand_player_next[t->kindFocus] -= 2;
	hand_player_next[t->kindFocus + NUM_KIND_BUFFED] += 1;
	RemoveTile(t->handtiles[player_next], t->handtileLinkF[player_next], t->handtileLinkB[player_next], t->kindFocus, 2, t->tilesLastMeld);
	t->meld[t->player] += 1;
	t->state = MJS_CHIPON;
	MjtableOptionSelf(t);
}

void MjtableKanOther(struct mjtable*t, int who) {
	unsigned char const player_next = t->player = (t->player + who + 1) % NUM_MJPLAYER;
	numkind*const hand_player_next = t->hands[player_next];
	hand_player_next[t->kindFocus] -= 3;
	hand_player_next[t->kindFocus + NUM_KIND_BUFFED] += 1;
	RemoveTile(t->handtiles[player_next], t->handtileLinkF[player_next], t->handtileLinkB[player_next], t->kindFocus, 3, t->tilesLastMeld);
	t->meld[t->player] += 1;
	t->quad[t->player] += 1;
	t->state = MJS_KAN_C;
	MjtableOptionOther(t);
}

void MjtableKanSelf(struct mjtable*t, int which) {
	unsigned char const player = t->player;
	numkind*const hand_player = t->hands[player];
	char*const handtile_player = t->handtiles[player];
	mjtile*const linkF = t->handtileLinkF[player];
	mjtile*const linkB = t->handtileLinkB[player];
	mjkind const k_kan = t->optionSelf.kanCandidate[which];

	if (t->state == MJS_DRAW) {
		hand_player[t->kindFocus] += 1;
		handtile_player[t->tileFocus] = 1;
		linkB[NUM_TILE_TOTAL] = linkF[linkB[t->tileFocus] = linkB[linkF[t->tileFocus] = NUM_TILE_TOTAL]] = t->tileFocus;
	}
	if (hand_player[k_kan + NUM_KIND_BUFFED] > 0) {
		mjtile tile = FindTile(handtile_player, k_kan);
		hand_player[k_kan] -= 1;
		handtile_player[tile] = 0;
		linkF[linkB[linkF[tile]] = linkB[tile]] = linkF[tile];
		t->kindFocus = k_kan;
		t->tileFocus = tile;
		t->state = MJS_KAN_R;
	}
	else {
		mjtile tile = FindTile(handtile_player, k_kan);
		hand_player[k_kan] -= 4;
		RemoveTile(handtile_player, t->handtileLinkF[player], t->handtileLinkB[player], k_kan, 4, t->tilesLastMeld);
		hand_player[k_kan + NUM_KIND_BUFFED] += 1;
		t->meld[t->player] += 1;
		t->kindFocus = k_kan;
		t->tileFocus = tile;
		t->state = MJS_KAN_C;
	}

	t->quad[t->player] += 1;
	MjtableOptionOther(t);
}

void MjtableFlower(struct mjtable*t) {
	unsigned char const player = t->player;
	numkind*const hand = t->hands[player];
	char*const handtile = t->handtiles[player];
	mjtile*const linkF = t->handtileLinkF[player];
	mjtile*const linkB = t->handtileLinkB[player];
	if (t->state == MJS_DRAW) {
		hand[t->kindFocus] += 1;
		handtile[t->tileFocus] = 1;
		linkB[NUM_TILE_TOTAL] = linkF[linkB[t->tileFocus] = linkB[linkF[t->tileFocus] = NUM_TILE_TOTAL]] = t->tileFocus;
	}
	hand[KIND_FLOWER_BUFFED] -= 1;
	t->flower[t->player] += 1;
	t->kindFocus = KIND_FLOWER_BUFFED;
	{
		mjtile const tile = FindTile(handtile, KIND_FLOWER_BUFFED);
		handtile[tile] = 0;
		linkF[linkB[linkF[tile]] = linkB[tile]] = linkF[tile];
		t->tileFocus = tile;
	}
	t->state = MJS_KAN_R;
	MjtableOptionOther(t);
}

void MjtableDiscardT(struct mjtable*t, mjtile tile) {
	unsigned char const player = t->player;
	numkind*const hand_player = t->hands[player];
	char*const handtile = t->handtiles[player];
	if (handtile[tile]) {
		mjtile*const linkF = t->handtileLinkF[player];
		mjtile*const linkB = t->handtileLinkB[player];
		mjkind const kind = MjBuffKind(tile / NUM_TILE_EACHKIND);
		hand_player[kind] -= 1;
		handtile[tile] = 0;
		linkF[linkB[linkF[tile]] = linkB[tile]] = linkF[tile];
		if (t->state == MJS_DRAW) {
			hand_player[t->kindFocus] += 1;
			handtile[t->tileFocus] = 1;
			linkB[NUM_TILE_TOTAL] = linkF[linkB[t->tileFocus] = linkB[linkF[t->tileFocus] = NUM_TILE_TOTAL]] = t->tileFocus;
		}
		t->kindFocus = kind;
		t->tileFocus = tile;
	}
	else if (t->state == MJS_CHIPON) {
		char success = 0;
		for (int i = KIND_FIRSTFLOWER; i >= 0; --i) {
			mjtile*const linkF = t->handtileLinkF[player];
			mjtile*const linkB = t->handtileLinkB[player];
			mjkind const k_sch = MjBuffKind(i);
			if (hand_player[k_sch] > 0) {
				mjtile const tile_real = FindTile(handtile, k_sch);
				success = 1;
				hand_player[k_sch] -= 1;
				handtile[tile_real] = 0;
				linkF[linkB[linkF[tile_real]] = linkB[tile_real]] = linkF[tile_real];
				t->kindFocus = k_sch;
				t->tileFocus = tile_real;
				break;
			}
		}
		if (!success) return (void)(t->state = MJS_N);
	}
	t->state = MJS_DISCARD;
	MjtableOptionOther(t);
}
#include"mjtable.h"

#include<string.h>
#include<stddef.h>

#if NUM_TILE_EACHKIND > 4
#error Ankan or kakan problem
#endif

static char CanWinDefault(struct mjtable const*t, unsigned char who, void*context) { return 0; }
static char CanReadyDefault(struct mjtable const*t, unsigned char who, void*context) { return 0; }

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

#ifdef MJTABLE_PRESERVE_TILE
	for (int i = 0; i < NUM_MJPLAYER; ++i) {
		memcpy(t->handsTile + i, t->mount + i * NUM_TILE_IN_HAND, NUM_TILE_IN_HAND);
		MjhtreeInit(t->handsTree + i, t->handsTile[i]);
		for (int j = 0; j < NUM_TILE_IN_HAND; ++j)
			t->hands[(t->player + i) % NUM_MJPLAYER][MjBuffKind(t->mount[i * NUM_TILE_IN_HAND + j] / NUM_TILE_EACHKIND)] += 1;
	}
	t->kindFocus = MjBuffKind((t->tileFocus = t->mount[NUM_TILE_IN_HAND * NUM_MJPLAYER]) / NUM_TILE_EACHKIND);
#else
	for (int i = 0; i < NUM_MJPLAYER; ++i) {
		for (int j = 0; j < NUM_TILE_IN_HAND; ++j)
			t->hands[(t->player + i) % NUM_MJPLAYER][MjBuffKind(t->mount[i * NUM_TILE_IN_HAND + j] / NUM_TILE_EACHKIND)] += 1;
	}
	t->kindFocus = MjBuffKind(t->mount[NUM_TILE_IN_HAND * NUM_MJPLAYER] / NUM_TILE_EACHKIND);
#endif

	t->state = MJS_DRAW;
	MjtableOptionSelf(t);
}

void MjtableDiscard(struct mjtable*t, mjkind kind) {
	numkind*const hand_player = t->hands[t->player];
#ifdef MJTABLE_PRESERVE_TILE
	if (hand_player[kind] > 0) {
		unsigned char const index = MjhtreeFindKind(&t->handsTree[t->player], kind);
		mjtile tileReal = t->handsTile[t->player][index];
		hand_player[kind] -= 1;
		t->handsTile[t->player][index] = t->tileFocus;
		if (t->state == MJS_DRAW) {
			hand_player[t->kindFocus] += 1;
			MjhtreeUpdate(t->handsTree + t->player, index);
		}
		else MjhtreeDelete(t->handsTree + t->player, index);

		t->tileFocus = tileReal;
		t->kindFocus = kind;
	}
	else if (t->state == MJS_CHIPON) {
		unsigned char const index = t->handsTree[t->player].child[NUM_TILE_IN_HAND][0];
		if (index != NUM_TILE_IN_HAND) {
			t->handsTile[t->player][index] = t->tileFocus;
			MjhtreeDelete(t->handsTree + t->player, index);
			t->kindFocus = MjBuffKind((t->tileFocus = t->handsTile[t->player][index]) / NUM_TILE_EACHKIND);
			hand_player[t->kindFocus] -= 1;
		}
		else {
			t->state = MJS_N;
			return;
		}
	}
#else
	if (hand_player[kind] > 0) {
		hand_player[kind] -= 1;
		if (t->state == MJS_DRAW) hand_player[t->kindFocus] += 1;
		t->kindFocus = kind;
	}
	else if (t->state == MJS_CHIPON) {
		char success = 0;
		for (int i = KIND_FIRSTFLOWER; i >= 0; --i) {
			mjkind const k_sch = MjBuffKind(i);
			if (hand_player[k_sch] > 0) {
				success = 1;
				hand_player[k_sch] -= 1;
				t->kindFocus = k_sch;
				break;
			}
		}
		if (!success) return (void)(t->state = MJS_N);
	}
#endif
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

#ifdef MJTABLE_PRESERVE_TILE
	t->kindFocus = MjBuffKind((t->tileFocus = tile_drawn) / NUM_TILE_EACHKIND);
#else
	t->kindFocus = MjBuffKind(tile_drawn / NUM_TILE_EACHKIND);
#endif

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
	numkind*const hand_player_next = t->hands[t->player = (t->player + 1) % NUM_MJPLAYER];
#ifdef MJTABLE_PRESERVE_TILE
	switch (type) {
	case 0:
		hand_player_next[t->kindFocus + 1] -= 1;
		hand_player_next[t->kindFocus + 2] -= 1;
		MjhtreeDelete(t->handsTree + t->player, MjhtreeFindKind(t->handsTree + t->player, t->kindFocus + 1));
		MjhtreeDelete(t->handsTree + t->player, MjhtreeFindKind(t->handsTree + t->player, t->kindFocus + 2));
		hand_player_next[t->kindFocus + 1 + NUM_KIND_BUFFED * 2] += 1;
		break;
	default:
	case 1:
		hand_player_next[t->kindFocus - 1] -= 1;
		hand_player_next[t->kindFocus + 1] -= 1;
		MjhtreeDelete(t->handsTree + t->player, MjhtreeFindKind(t->handsTree + t->player, t->kindFocus - 1));
		MjhtreeDelete(t->handsTree + t->player, MjhtreeFindKind(t->handsTree + t->player, t->kindFocus + 1));
		hand_player_next[t->kindFocus + NUM_KIND_BUFFED * 2] += 1;
		break;
	case 2:
		hand_player_next[t->kindFocus - 2] -= 1;
		hand_player_next[t->kindFocus - 1] -= 1;
		MjhtreeDelete(t->handsTree + t->player, MjhtreeFindKind(t->handsTree + t->player, t->kindFocus - 2));
		MjhtreeDelete(t->handsTree + t->player, MjhtreeFindKind(t->handsTree + t->player, t->kindFocus - 1));
		hand_player_next[t->kindFocus - 1 + NUM_KIND_BUFFED * 2] += 1;
		break;
	}
#else
	hand_player_next[t->kindFocus] += 1;
	hand_player_next[t->kindFocus - type] -= 1;
	hand_player_next[t->kindFocus - type + 1] -= 1;
	hand_player_next[t->kindFocus - type + 2] -= 1;
	hand_player_next[t->kindFocus - type + 1 + NUM_KIND_BUFFED * 2] += 1;
#endif

	t->meld[t->player] += 1;
	t->state = MJS_CHIPON;
	MjtableOptionSelf(t);
}

void MjtablePon(struct mjtable*t, int who) {
	numkind*const hand_player_next = t->hands[t->player = (t->player + who + 1) % NUM_MJPLAYER];
	hand_player_next[t->kindFocus] -= 2;
	hand_player_next[t->kindFocus + NUM_KIND_BUFFED] += 1;
#ifdef MJTABLE_PRESERVE_TILE
	MjhtreeDelete(t->handsTree + t->player, MjhtreeFindKind(t->handsTree + t->player, t->kindFocus));
	MjhtreeDelete(t->handsTree + t->player, MjhtreeFindKind(t->handsTree + t->player, t->kindFocus));
#endif
	t->meld[t->player] += 1;
	t->state = MJS_CHIPON;
	MjtableOptionSelf(t);
}

void MjtableKanOther(struct mjtable*t, int who) {
	numkind*const hand_player_next = t->hands[t->player = (t->player + who + 1) % NUM_MJPLAYER];
	hand_player_next[t->kindFocus] -= 3;
	hand_player_next[t->kindFocus + NUM_KIND_BUFFED] += 1;
#ifdef MJTABLE_PRESERVE_TILE
	MjhtreeDelete(t->handsTree + t->player, MjhtreeFindKind(t->handsTree + t->player, t->kindFocus));
	MjhtreeDelete(t->handsTree + t->player, MjhtreeFindKind(t->handsTree + t->player, t->kindFocus));
	MjhtreeDelete(t->handsTree + t->player, MjhtreeFindKind(t->handsTree + t->player, t->kindFocus));
#endif
	t->meld[t->player] += 1;
	t->quad[t->player] += 1;
	t->state = MJS_KAN_C;
	MjtableOptionOther(t);
}

void MjtableKanSelf(struct mjtable*t, int which) {
	numkind*const hand_player = t->hands[t->player];
	mjkind const k_kan = t->optionSelf.kanCandidate[which];

	if (t->state == MJS_DRAW) hand_player[t->kindFocus] += 1;
	if (hand_player[k_kan + NUM_KIND_BUFFED] > 0) {
#ifdef MJTABLE_PRESERVE_TILE
		unsigned char const index = MjhtreeFindKind(t->handsTree + t->player, k_kan);
		if (index != NUM_TILE_IN_HAND) {
			mjtile const tile_focus = t->handsTile[t->player][index];
			t->handsTile[t->player][index] = t->tileFocus;
			t->tileFocus = tile_focus;
			MjhtreeUpdate(t->handsTree + t->player, index);
		}
#endif
		hand_player[k_kan] -= 1;
		t->state = MJS_KAN_R;
	}
	else {
#ifdef MJTABLE_PRESERVE_TILE
		unsigned char index;
		MjhtreeDelete(t->handsTree + t->player, MjhtreeFindKind(t->handsTree + t->player, k_kan));
		MjhtreeDelete(t->handsTree + t->player, MjhtreeFindKind(t->handsTree + t->player, k_kan));
		MjhtreeDelete(t->handsTree + t->player, MjhtreeFindKind(t->handsTree + t->player, k_kan));
		index = MjhtreeFindKind(t->handsTree + t->player, k_kan);
		if (index != NUM_TILE_IN_HAND) {
			mjtile const tile_focus = t->handsTile[t->player][index];
			t->handsTile[t->player][index] = t->tileFocus;
			t->tileFocus = tile_focus;
			MjhtreeUpdate(t->handsTree + t->player, index);
		}
#endif
		hand_player[k_kan] -= 4;
		hand_player[k_kan + NUM_KIND_BUFFED] += 1;
		t->meld[t->player] += 1;
		t->state = MJS_KAN_C;
	}

	t->quad[t->player] += 1;
	t->kindFocus = k_kan;
	MjtableOptionOther(t);
}

void MjtableFlower(struct mjtable*t) {
#ifdef MJTABLE_PRESERVE_TILE
	unsigned char const index = MjhtreeFindKind(t->handsTree + t->player, KIND_FLOWER_BUFFED);
	if (index != NUM_TILE_IN_HAND) {
		mjtile const tile_focus = t->handsTile[t->player][index];
		t->handsTile[t->player][index] = t->tileFocus;
		t->tileFocus = tile_focus;
		MjhtreeUpdate(t->handsTree + t->player, index);
	}
#endif
	if (t->state == MJS_DRAW) t->hands[t->player][t->kindFocus] += 1;
	t->hands[t->player][KIND_FLOWER_BUFFED] -= 1;
	t->flower[t->player] += 1;
	t->kindFocus = KIND_FLOWER_BUFFED;
	t->state = MJS_KAN_R;
	MjtableOptionOther(t);
}

void MjtableDiscardT(struct mjtable*t, mjtile tile) {
	numkind*const hand_player = t->hands[t->player];
#ifdef MJTABLE_PRESERVE_TILE
	unsigned char index = MjhtreeFindTile(&t->handsTree[t->player], tile);
	if (index != NUM_TILE_IN_HAND) {
		mjkind kind_real = MjBuffKind(tile / NUM_TILE_EACHKIND);
		hand_player[kind_real] -= 1;
		t->handsTile[t->player][index] = t->tileFocus;
		if (t->state == MJS_DRAW) {
			hand_player[t->kindFocus] += 1;
			MjhtreeUpdate(t->handsTree + t->player, index);
		}
		else MjhtreeDelete(t->handsTree + t->player, index);

		t->tileFocus = tile;
		t->kindFocus = kind_real;
	}
	else if (t->state == MJS_CHIPON) {
		index = t->handsTree[t->player].child[NUM_TILE_IN_HAND][0];
		if (index != NUM_TILE_IN_HAND) {
			t->handsTile[t->player][index] = t->tileFocus;
			MjhtreeDelete(t->handsTree + t->player, index);
			t->kindFocus = MjBuffKind((t->tileFocus = t->handsTile[t->player][index]) / NUM_TILE_EACHKIND);
			hand_player[t->kindFocus] -= 1;
		}
		else {
			t->state = MJS_N;
			return;
		}
	}
#else
	mjkind const kind = MjBuffKind(tile / NUM_TILE_EACHKIND);
	if (hand_player[kind] > 0) {
		hand_player[kind] -= 1;
		if (t->state == MJS_DRAW) hand_player[t->kindFocus] += 1;
		t->kindFocus = kind;
	}
	else if (t->state == MJS_CHIPON) {
		char success = 0;
		for (int i = KIND_FIRSTFLOWER; i >= 0; --i) {
			mjkind const k_sch = MjBuffKind(i);
			if (hand_player[k_sch] > 0) {
				success = 1;
				hand_player[k_sch] -= 1;
				t->kindFocus = k_sch;
				break;
			}
		}
		if (!success) return (void)(t->state = MJS_N);
	}
#endif
	t->state = MJS_DISCARD;
	MjtableOptionOther(t);
}
#include"mjhand.h"

#include<string.h>

struct handcount {
	int const sum_singlets;
	int const sum_pairs;
	int const sum_grouptwo_not_pair;
	int const sum_flowers;
};

struct handstate {
	int const progress;
	char const needtri;
	char const needdu;
	char const needpair;
	char const pairtotri;
	char const needsig;
};

struct contextprogress {
	int progress;
	int*wanted;
};

static struct handcount MjhandCount(mjhand const h) {
	int sum_singlets = 0;
	int sum_pairs = 0;
	int sum_grouptwo_not_pair = 0;
	for (int i = 0; i < NUM_SUIT; ++i) {
		for (int j = 0; j < NUM_KIND_EACHSUIT; ++j) {
			unsigned const index = i * (NUM_KIND_EACHSUIT + 2) + j + 2;
			sum_singlets += h[index];
			sum_pairs += h[index + NUM_KIND_BUFFED * 3];
			sum_grouptwo_not_pair += h[index + NUM_KIND_BUFFED * 4];
			sum_grouptwo_not_pair += h[index + NUM_KIND_BUFFED * 5];
		}
	}
	for (int i = 0; i < NUM_KIND_CHAR; ++i) {
		unsigned const index = i * 3 + KIND_FIRSTCHAR_BUFFED;
		sum_singlets += h[index];
		sum_pairs += h[index + NUM_KIND_BUFFED * 3];
	}
	return (struct handcount) { sum_singlets, sum_pairs, sum_grouptwo_not_pair, h[KIND_FLOWER_BUFFED] };
}

static struct handstate MjhandProgressNormalExtended(int sum_singlets, int sum_pairs, int sum_grouptwo_not_pair, int sum_flowers) {
	int const shortage_tri = ((sum_pairs + sum_grouptwo_not_pair) * 2 + sum_singlets + sum_flowers) / 3;

	if (sum_pairs > 0) {
		int const sum_grouptwo = sum_pairs + sum_grouptwo_not_pair - 1;
		int const shortage_du = shortage_tri - sum_grouptwo;
		if (shortage_du <= 0)
			return (struct handstate) { shortage_tri, (char)shortage_tri, 0, 0, sum_pairs > 1, 0 };
		else {
			int const shortage_singlets = shortage_du - sum_singlets;
			if (shortage_singlets <= 0)
				return (struct handstate) { shortage_tri + shortage_du, (char)shortage_tri, 1, 0, 1, 0 };
			else
				return (struct handstate) { shortage_tri + shortage_du + shortage_singlets, (char)shortage_tri, 1, 0, 1, 1 };
		}
	}
	else {
		int const shortage_du = shortage_tri - sum_grouptwo_not_pair;
		if (shortage_du <= 0) {
			if (sum_singlets > 0 || shortage_du < 0)
				return (struct handstate) { shortage_tri + 1, (char)shortage_tri, 0, 1, 0, 0 };
			else
				return (struct handstate) { shortage_tri + 2, (char)shortage_tri, 0, 1, 0, 1 };
		}
		else {
			int const shortage_singlets = shortage_du + 1 - sum_singlets;
			if (shortage_singlets <= 0)
				return (struct handstate) { shortage_tri + shortage_du + 1, (char)shortage_tri, 1, 1, 1, 0 };
			else
				return (struct handstate) { shortage_tri + shortage_du + shortage_singlets + 1, (char)shortage_tri, 1, 1, 1, 1 };
		}
	}
}

int MjhandProgressNormal(int sum_singlets, int sum_pairs, int sum_grouptwo_not_pair, int sum_flowers) {
	return MjhandProgressNormalExtended(sum_singlets, sum_pairs, sum_grouptwo_not_pair, sum_flowers).progress;
}

int MjhandProgressPair(int sum_singlets, int sum_pairs, int sum_flowers) {
	if (sum_pairs * 2 + sum_singlets + sum_flowers >= NUM_TILE_IN_HAND) {
		if (sum_pairs + sum_singlets < (NUM_TILE_IN_HAND + 1) / 2)
			return (int)(NUM_TILE_IN_HAND + 1) - sum_pairs * 2 - sum_singlets;
		else return (int)(NUM_TILE_IN_HAND + 1) / 2 - sum_pairs;
	}
	else return NUM_TILE_IN_HAND + 2;
}

static void GetWanted(mjhand const h, char const needtri, char const needdu, char const needpair, char const pairtotri, int output[NUM_KIND_BUFFED]) {
	for (int i = 2; i <= KIND_LASTCHAR_BUFFED; ++i) {
		if (needtri) {
			if (pairtotri && h[i + NUM_KIND_BUFFED * 3] > 0) output[i] |= 1;
			if (h[i + 1 + NUM_KIND_BUFFED * 4] > 0) output[i] |= 2;
			if (h[i + NUM_KIND_BUFFED * 5] > 0) output[i] |= 4;
			if (h[i - 2 + NUM_KIND_BUFFED * 4] > 0) output[i] |= 8;
		}
		if (needdu) {
			if (h[i + 2] > 0) output[i] |= 32;
			if (h[i + 1] > 0) output[i] |= 64;
			if (h[i] > 0) output[i] |= 128;
			if (h[i - 1] > 0) output[i] |= 256;
			if (h[i - 2] > 0) output[i] |= 512;
		}
		if (needpair) {
			if (h[i] > 0) output[i] |= 16;
			if (h[i + 1] > 0 && h[i + 2 + NUM_KIND_BUFFED] > 0 || h[i + 2] > 0 && h[i + 1 + NUM_KIND_BUFFED] > 0)
				output[i] |= 2;
			if (h[i - 1] > 0 && h[i + 1 + NUM_KIND_BUFFED] > 0 || h[i + 1] > 0 && h[i - 1 + NUM_KIND_BUFFED] > 0)
				output[i] |= 4;
			if (h[i - 2] > 0 && h[i - 1 + NUM_KIND_BUFFED] > 0 || h[i - 1] > 0 && h[i - 2 + NUM_KIND_BUFFED] > 0)
				output[i] |= 8;
		}
	}
}

static void MjhandProgressInAnalysis(mjhand const h, void*context) {
	struct contextprogress*c = context;
	struct handcount const hc = MjhandCount(h);
	struct handstate const hp = MjhandProgressNormalExtended(hc.sum_singlets, hc.sum_pairs, hc.sum_grouptwo_not_pair, hc.sum_flowers);
	if (hp.progress < c->progress) c->progress = hp.progress;
}

static void MjhandProgressInAnalysisW(mjhand const h, void*context) {
	struct contextprogress*c = context;
	struct handcount const hc = MjhandCount(h);
	struct handstate const hp = MjhandProgressNormalExtended(hc.sum_singlets, hc.sum_pairs, hc.sum_grouptwo_not_pair, hc.sum_flowers);
	if (hp.progress <= c->progress) {
		if (hp.progress < c->progress) {
			c->progress = hp.progress;
			memset(c->wanted, 0, sizeof(int[NUM_KIND_BUFFED]));
		}
		GetWanted(h, hp.needtri, hp.needdu, hp.needpair, hp.pairtotri, c->wanted);
	}
}

int MjhandProgress(mjhand h, int output[NUM_KIND_BUFFED]) {
	struct contextprogress c;
	c.wanted = output;

#if NUM_TILE_IN_HAND % 2 == 1
	int sum_singlets = 0;
	int sum_pairs = 0;
	int const sum_flowers = h[KIND_FLOWER_BUFFED];
	if (c.wanted) {
		for (int i = 0; i < NUM_SUIT; ++i) {
			for (int j = 0; j < NUM_KIND_EACHSUIT; ++j) {
				unsigned const index = i * (NUM_KIND_EACHSUIT + 2) + j + 2;
				sum_pairs += h[index] / 2;
				sum_singlets += c.wanted[index] = h[index] % 2;
			}
		}
		for (int i = 0; i < NUM_KIND_CHAR; ++i) {
			unsigned const index = i * 3 + KIND_FIRSTCHAR_BUFFED;
			sum_pairs += h[index] / 2;
			sum_singlets += c.wanted[index] = h[index] % 2;
		}
	}
	else {
		for (int i = 0; i < NUM_SUIT; ++i) {
			for (int j = 0; j < NUM_KIND_EACHSUIT; ++j) {
				unsigned const index = i * (NUM_KIND_EACHSUIT + 2) + j + 2;
				sum_pairs += h[index] / 2;
				sum_singlets += h[index] % 2;
			}
		}
		for (int i = 0; i < NUM_KIND_CHAR; ++i) {
			unsigned const index = i * 3 + KIND_FIRSTCHAR_BUFFED;
			sum_pairs += h[index] / 2;
			sum_singlets += h[index] % 2;
		}
	}
	c.progress = MjhandProgressPair(sum_singlets, sum_pairs, sum_flowers);
#else
	c.progress = NUM_TILE_IN_HAND + 2;
#endif

	MjhandAnalyseConcise(h, c.wanted ? MjhandProgressInAnalysisW : MjhandProgressInAnalysis, &c);
	return c.progress;
}
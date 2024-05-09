#ifndef MJHAND_MJHANDANALYSE_H
#define MJHAND_MJHANDANALYSE_H

struct anacontext {
	mjkind*data;
	void(*func)(mjhand const, void*);
	void*context;
};

static void AnalyseDuplets(struct anacontext const*c, int i0, int j0) {
	mjkind*data = c->data;
#ifndef MJHAND_VERBOSE_ANALYSE
	char irreducible = 1;
#endif
	int i_end = KIND_LASTCHAR_BUFFED;
	for (int i = i0, j = j0; i <= i_end; ++i) {
		for (; j < 3; j++) {
			switch (j) {
			case 0:
				if (data[i] >= 2) {
#ifndef MJHAND_VERBOSE_ANALYSE
					irreducible = 0;
					if (i < i_end) i_end = i;
#endif
					data[i] -= 2;
					data[i + NUM_KIND_BUFFED * 3] += 1;
					AnalyseDuplets(c, i, 0);
					data[i] += 2;
					data[i + NUM_KIND_BUFFED * 3] -= 1;
				}
				break;
			case 1:
				if (data[i] > 0 && data[i + 1] > 0) {
#ifndef MJHAND_VERBOSE_ANALYSE
					irreducible = 0;
					if (i + 1 < i_end) i_end = i + 1;
#endif
					data[i] -= 1;
					data[i + 1] -= 1;
					data[i + NUM_KIND_BUFFED * 4] += 1;
					AnalyseDuplets(c, i, 1);
					data[i] += 1;
					data[i + 1] += 1;
					data[i + NUM_KIND_BUFFED * 4] -= 1;
				}
				break;
			case 2:
				if (data[i] > 0 && data[i + 2] > 0) {
#ifndef MJHAND_VERBOSE_ANALYSE
					irreducible = 0;
					if (i + 2 < i_end) i_end = i + 2;
#endif
					data[i] -= 1;
					data[i + 2] -= 1;
					data[i + 1 + NUM_KIND_BUFFED * 5] += 1;
					AnalyseDuplets(c, i, 2);
					data[i] += 1;
					data[i + 2] += 1;
					data[i + 1 + NUM_KIND_BUFFED * 5] -= 1;
				}
				break;
			}
		}
		j = 0;
	}

#ifndef MJHAND_VERBOSE_ANALYSE
	if (irreducible)
#endif
	{
		c->func(data, c->context);
	}
}

static void AnalyseTriplets(struct anacontext const*c, int i0, int j0) {
	mjkind*data = c->data;
#ifndef MJHAND_VERBOSE_ANALYSE
	char irreducible = 1;
#endif
	int i_end = KIND_LASTCHAR_BUFFED;
	for (int i = i0, j = j0; i <= i_end; ++i) {
		for (; j < 2; j++) {
			switch (j) {
			case 0:
				if (data[i] >= 3) {
#ifndef MJHAND_VERBOSE_ANALYSE
					irreducible = 0;
					if (i < i_end) i_end = i;
#endif
					data[i] -= 3;
					data[i + NUM_KIND_BUFFED] += 1;
					AnalyseTriplets(c, i, 0);
					data[i] += 3;
					data[i + NUM_KIND_BUFFED] -= 1;
				}
				break;
			case 1:
				if (data[i] > 0 && data[i + 1] > 0 && data[i + 2] > 0) {
#ifndef MJHAND_VERBOSE_ANALYSE
					irreducible = 0;
					if (i + 2 < i_end) i_end = i + 2;
#endif
					data[i] -= 1;
					data[i + 1] -= 1;
					data[i + 2] -= 1;
					data[i + 1 + NUM_KIND_BUFFED * 2] += 1;
					AnalyseTriplets(c, i, 1);
					data[i] += 1;
					data[i + 1] += 1;
					data[i + 2] += 1;
					data[i + 1 + NUM_KIND_BUFFED * 2] -= 1;
				}
				break;
			}
		}
		j = 0;
	}

#ifndef MJHAND_VERBOSE_ANALYSE
	if (irreducible)
#endif
	{
		AnalyseDuplets(c, 2, 0);
	}
}

static void MjhandAnalyse(mjhand data, void(*func)(mjhand const, void*), void*context) {
	struct anacontext c = { data, func, context };
	AnalyseTriplets(&c, 2, 0);
}

#endif
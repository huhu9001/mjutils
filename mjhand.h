#ifndef MJHAND_H
#define MJHAND_H

#include"mahjong.h"

void MjhandAnalyseConcise(mjhand data, void(*func)(mjhand const, void*), void*context);
void MjhandAnalyseVerbose(mjhand data, void(*func)(mjhand const, void*), void*context);

int MjhandProgressNormal(int sum_singlets, int sum_pairs, int sum_grouptwo_not_pair, int sum_flowers);
int MjhandProgressPair(int sum_singlets, int sum_pairs, int sum_flowers);
int MjhandProgress(mjhand h, int output[NUM_KIND_BUFFED]);

#endif
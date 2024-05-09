#include"mjhand.h"
#define MJHAND_VERBOSE_ANALYSE
#include"mjhand_MjhandAnalyse.h"

void MjhandAnalyseVerbose(mjhand data, void(*func)(mjhand const, void*), void*context) {
	MjhandAnalyse(data, func, context);
}
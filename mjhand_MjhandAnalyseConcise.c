#include"mjhand.h"
#undef MJHAND_VERBOSE_ANALYSE
#include"mjhand_MjhandAnalyse.h"

void MjhandAnalyseConcise(mjhand data, void(*func)(mjhand const, void*), void*context) {
	MjhandAnalyse(data, func, context);
}
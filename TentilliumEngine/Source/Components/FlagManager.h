#include "Flag.h"


class FlagManager
{
	/*
	Sets all Flags to false.
	This should only ever be called between loops
	remember that this has to apply to multiple instances
	*/
	void ClearFlags()
	{
		Flag::currentVersion++;
	}
};

//
//	main/gxt2conv.h
//

#ifndef _GXT2CONV_H_
#define _GXT2CONV_H_

// Project
#include "gxt/gxt2.h"
#include "gxt/convert.h"

#include "system/app.h"

class gxt2conv : public CApp
{
public:
	gxt2conv() = default;
	~gxt2conv() = default;

	int Run(int argc, char* argv[]) override;
};

#endif // _GXT2CONV_H_
//
//	main/gxt2merge.h
//

#ifndef _GXT2MERGE_H_
#define _GXT2MERGE_H_

// Project
#include "gxt/gxt2.h"
#include "gxt/convert.h"

#include "system/app.h"

class gxt2merge : public CApp
{
private:
	gxt2merge() = default;
	~gxt2merge() = default;
public:
	int Run(int argc, char* argv[]) override;
public:
	static gxt2merge& GetInstance();
};

#endif // !_GXT2MERGE_H_
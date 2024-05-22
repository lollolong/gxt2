//
//	data/stringhash.h
//

#ifndef _STRINGHASH_H_
#define _STRINGHASH_H_

namespace rage
{
	unsigned int atFinalizeHash(unsigned int hash);
	unsigned int atPartialStringHash(const char* string);
	unsigned int atStringHash(const char* string);
}

#endif // _STRINGHASH_H_
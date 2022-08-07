//
//  defines.h
//  Project256
//
//  Created by Andreas Stahl on 14.06.22.
//

#ifndef defines_h
#define defines_h

/// clarifying defines for static
#define compiletime static constexpr
#define constant static const
#define globalvar static
#define internalfunc static
#define localpersist static
#define classmethod static

/// import Swift renaming macros
#ifdef __APPLE__
#include "CoreFoundation/CFBase.h"
#else
#define CF_SWIFT_NAME(a)
#endif

#endif /* defines_h */

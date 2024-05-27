// xoverflow.h
// `XOverflow` exception class.

// This file is in the public domain.

#ifndef SMBASE_XOVERFLOW_H
#define SMBASE_XOVERFLOW_H

#include "exc.h"                       // DEFINE_XMESSAGE_SUBCLASS


/* Exception thrown when there would be an arithmetic overflow.

   It is also used for division by zero, with the idea being to
   generalize slightly to any invalid arithmetic operation.

   This is used by the `overflow` and `sm-ap-int` modules.
*/
DEFINE_XMESSAGE_SUBCLASS(XOverflow);


#endif // SMBASE_XOVERFLOW_H

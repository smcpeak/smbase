// gdvalue-transform.h
// `GDValueTransform`, a recursive `GDValue` transformer.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_GDVALUE_TRANSFORM_H
#define SMBASE_GDVALUE_TRANSFORM_H

#include "gdvalue-transform-fwd.h"     // fwds for this module

#include "smbase/gdv-symbol-fwd.h"     // gdv::GDVSymbol [n]
#include "smbase/gdvalue-fwd.h"        // gdv::GDValue [n]
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE


OPEN_NAMESPACE(gdv)


// Recursively transform an input `GDValue` to another, making a deep
// recursive copy.
class GDValueTransform {
public:      // methods
  /* Transform `v` to the desired output.  The returned value does not
     have to be of the same kind.

     Note that there is no way to indicate to entirely remove a value
     using the return value; a null `GDValue` is treated like any other
     value.  To remove values, one must override `transform`.

     Default behavior:

       Return scalars (symbol, integer, float, string) unchanged.

       For containers, make a new container of the same `kind()`, then
       recursively transform the contents:

         - sequence, tuple: Transform every element in order and append
           it.

         - set: Transform every value in iteration order, inserting the
           transformed value into a new set, discarding any duplicates.

         - map, ordered map: Transform every key and value in iteration
           order, then insert that into the new map or ordered map.  If
           the transformed key collides with a previous transformed key,
           the entire entry is discarded.

       For tagged containers, the tag is transformed by calling
       `transformContainerTag` and assigned to the new container after
       all the elements have been transformed.
  */
  virtual GDValue transform(GDValue const &v);

  // Transform a symbol used as a container tag.
  //
  // The default behavior returns `v`.
  virtual GDVSymbol transformContainerTag(GDVSymbol tag);
};


// As a trivial application, `GDValueTransform` (without any
// subclassing) can be used to make a deep copy.
GDValue deepCopyGDValue(GDValue const &v);


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_TRANSFORM_H

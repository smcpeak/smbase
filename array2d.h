// array2d.h
// non-resizable 2D array with bounds checking
// needed because new T[i][j] is not legal if 'i' is not a constant

#ifndef ARRAY2D_H
#define ARRAY2D_H

#include "macros.h"          // DMEMB
#include "xassert.h"         // xassert


// Requirements on T:
//
// * Default (no-arg) ctor, for the initial array construction.
//
// * Destructor, of course.
//
// * Copy constructor, if any of the following Array2D methods
// are used:
//   - getElt
//
// * Copy assignment operator, if any of the following Array2D methods
// are used:
//   - Array2D(Array2D const &)
//   - operator=
//   - setElt
//   - setAll
//
// * Comparison operator==, if the Array2D::operator== method is used.

// Constness: A const Array2D does not permit modification of the
// array elements.

template <class T>
class Array2D {
private:     // data
  // Array dimensions; non-negative.
  //
  // Although I typically expect 'int' to suffice for the dimensions,
  // 'long' may be required to express the total array size and the
  // array indexes, and I do not want to have to remember to cast to
  // 'long' during all the calculations (error-prone, cluttersome,
  // tedious).
  long rows;
  long columns;

  // Array data.  Element (i,j) is at 'i * columns + j', i.e., rows
  // are contiguous.
  T *arr;                    // (owner)

private:     // funcs
  // Allocate the array.
  void alloc()
  {
    // Guard against possible overflow attacks.  This check will
    // be very cheap in comparison to the allocation.
    long totalElements = rows * columns;
    if (rows != 0) {
      xassert(totalElements / rows == columns);
    }
    else {
      // I just let a 0 allocation happen; the C++ runtime should
      // handle the degenerate case fine.  I only need to test for
      // it because of the division by zero possibility.
    }

    arr = new T[totalElements];
  }

  // Deallocate the array.
  void dealloc()
  {
    delete[] arr;
  }

  // Raw (no bounds checking) element reference.
  T const &rawEltRefC(long i, long j) const
  {
    return arr[i * columns + j];
  }

  T &rawEltRef(long i, long j)
  {
    return const_cast<T&>(rawEltRefC(i,j));
  }

  // Bounds-check an element coordinate.
  void bc(long i, long j) const
  {
    xassert(0 <= i && i < rows);
    xassert(0 <= j && j < columns);
  }

  // Copy every element from 'obj', which must already have the
  // same size as 'this'.  Use operator= for T.
  void copyFrom(Array2D const &obj)
  {
    xassert(rows == obj.rows);
    xassert(columns == obj.columns);

    for (long i=0; i < rows; i++) {
      for (long j=0; j < columns; j++) {
        rawEltRef(i,j) = obj.rawEltRefC(i,j);
      }
    }
  }

public:      // funcs
  // Create the array, initializing each element using T's default
  // ctor.  'r' is the number of rows, and 'c' the number of columns.
  Array2D(long r, long c)
    : rows(r),
      columns(c)
  {
    xassert(r >= 0);
    xassert(c >= 0);
    alloc();
  }

  // Destructor.
  ~Array2D()
  {
    dealloc();
  }

  // Create a copy of another array.  The elements are copied using
  // their operator=.
  Array2D(Array2D const &obj)
    : DMEMB(rows),
      DMEMB(columns)
  {
    alloc();
    copyFrom(obj);
  }
  
  // Copy an array.  Elements copied using operator=.  This will
  // require deallocating and allocating again the elements of 'this'
  // if the sizes differ initially.
  Array2D& operator= (Array2D const &obj)
  {
    if (this != &obj) {
      if (!( rows == obj.rows && columns == obj.columns )) {
        dealloc();
        rows = obj.rows;
        columns = obj.columns;
        alloc();
      }
      
      copyFrom(obj);
    }            

    return *this;
  }

  // Get dimensions.
  long getRows() const { return rows; }
  long getColumns() const { return columns; }

  // Element reference.  'i' must be in [0,r-1] where 'r' is the
  // value originally passed to the constructor; likewise for 'j'.
  T const &eltRefC(long i, long j) const
  {
    bc(i,j);
    return rawEltRefC(i,j);
  }

  T &eltRef(long i, long j)
  {
    return const_cast<T&>(eltRefC(i,j));
  }
  
  // Get an element (copy).  Returned object is initialized using
  // T's copy constructor.
  T getElt(long i, long j) const
  {
    return eltRefC(i,j);
  }

  // Set an element.  Uses T's operator= to set the array element.
  void setElt(long i, long j, T const &elt)
  {
    eltRef(i,j) = elt;
  }
  
  // Set all elements.  Uses T's operator=.
  void setAll(T const &elt)
  {
    for (long i=0; i < rows; i++) {
      for (long j=0; j < columns; j++) {
        rawEltRef(i,j) = elt;
      }
    }
  }

  // See if two arrays have all the same elements according to
  // T's operator==.
  bool operator== (Array2D const &obj) const
  {
    if (!( EMEMB(rows) &&
           EMEMB(columns) )) {
      return false;
    }

    for (long i=0; i < rows; i++) {
      for (long j=0; j < columns; j++) {
        if (!( rawEltRefC(i,j) == obj.rawEltRefC(i,j) )) {
          return false;
        }
      }
    }
    
    return true;
  }

  bool operator!= (Array2D const &obj) const
  {
    return !operator==(obj);
  }
};


// Iterate (i,j) over Array2D 'arr'.  Avoid using 'continue' and
// 'break' in the body since their effect might change if this
// definition is changed.
#define FOREACH_ARRAY2D_COORD(arr,i,j)          \
  for (long i=0; i < arr.getRows(); i++)        \
    for (long j=0; j < arr.getColumns(); j++)


#endif // ARRAY2D_H

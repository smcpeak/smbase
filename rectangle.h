// rectangle.h
// TRectangle class.

#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "macros.h"                    // DMEMB
#include "point.h"                     // TPoint

// Pair of points defining the opposite corners of a rectangle.
//
// The name of this class is "TRectangle" rather than "Rectangle"
// because the latter is the name of a function in the Windows API,
// leading to problems when both files are included.  It so happens
// that the leading "T" naming convention, which I believe originated
// with Turbo Pascal, is used with TPoint, so I use it here too.
template <class T>
class TRectangle {
public:      // data
  // Left-most and top-most point "inside" the rectangle.
  TPoint<T> topLeft;

  // Just beyond the right-most and bottom-most point inside
  // the rectangle.
  TPoint<T> bottomRight;

public:
  // Initialize with all zero coordinates.
  TRectangle() : topLeft(), bottomRight() {}

  TRectangle(TRectangle const &obj) :
    DMEMB(topLeft),
    DMEMB(bottomRight)
  {}

  TRectangle(TPoint<T> const &topLeft_, TPoint<T> const &bottomRight_) :
    topLeft(topLeft_),
    bottomRight(bottomRight_)
  {}

  TRectangle(T const &left, T const &top, T const &right, T const &bottom) :
    topLeft(left, top),
    bottomRight(right, bottom)
  {}

  TRectangle& operator= (TRectangle const &obj)
  {
    CMEMB(topLeft);
    CMEMB(bottomRight);
    return *this;
  }

  bool operator== (TRectangle const &obj) const
  {
    return EMEMB(topLeft) && EMEMB(bottomRight);
  }
  NOTEQUAL_OPERATOR(TRectangle)

  T const& left() const   { return topLeft.x; }
  T const& top() const    { return topLeft.y; }
  T const& right() const  { return bottomRight.x; }
  T const& bottom() const { return bottomRight.y; }

  T width() const  { return right() - left(); }
  T height() const { return bottom() - top(); }
  TPoint<T> size() const { return TPoint<T>(width(), height()); }

  // Set edge coordinates without affecting any other.
  void setLeft(T const &x)   { topLeft.x = x; }
  void setTop(T const &y)    { topLeft.y = y; }
  void setRight(T const &x)  { bottomRight.x = x; }
  void setBottom(T const &y) { bottomRight.y = y; }

  // Set the bottom/right coordinate to achieve a given size.
  void setWidth(T const &w) { setRight(left() + w); }
  void setHeight(T const &h) { setBottom(top() + h); }
  void setSize(TPoint<T> const &s) { setWidth(s.x); setHeight(s.y); }

  bool contains(TPoint<T> const &pt) const
  {
    return left() <= pt.x && pt.x < right() &&
           top() <= pt.y && pt.y < bottom();
  }

  // True if any points are contained in the rectangle.  Note that this
  // is different from size().isZero() if width or height is negative.
  bool isEmpty() const
  {
    return left() >= right() || top() >= bottom();
  }

  // Move both corners of the rectangle.
  void moveBy(point const &delta)
  {
    topLeft += delta;
    bottomRight += delta;
  }

  // Return the smallest rectangle that contains all of the points
  // that are in either 'this' or 'obj'.
  TRectangle operator| (TRectangle const &obj) const
  {
    if (this->isEmpty()) {
      return obj;
    }
    else if (obj.isEmpty()) {
      return *this;
    }
    else {
      return TRectangle(
        min(this->left(), obj.left()),
        min(this->top(), obj.top()),
        max(this->right(), obj.right()),
        max(this->bottom(), obj.bottom()));
    }
  }

  TRectangle& operator|= (TRectangle const &obj)
  {
    *this = *this | obj;
    return *this;
  }
};

#endif // RECTANGLE_H

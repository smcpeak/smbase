// bdffont.h
// Parse and represent BDF fonts in memory.

// spec:
// http://partners.adobe.com/public/developer/en/font/5005.BDF_Spec.pdf
//
// I've chosen to use this font representation, and include its module
// in smbase, because it is simple, tools are readily available on X,
// and the plethora of font- and text-related APIs on various
// platforms, even portable platforms such as Qt, fail to produce
// consistent, repeatable results across machines and platforms.
//
// I plan to build other modules on top of this module that are able
// to render glyphs stored in BDFFont objects onto various media, such
// as platform-specific pixmaps, etc.  This module will not incur any
// such dependencies.

// Note on character indices: This module uses 'int' rather than
// 'char' to name a character so that it will work with character
// encoding systems with more than 256 characters (such as Unicode).
// The font module interface makes no assumptions about what the
// characters mean, so is compatible with any character encoding
// system.
//
// That said, the current implementation uses a non-sparse array to
// map from character indices to glyph attributes, so is not as
// efficient as it could be for some encodings.  In the future it may
// be desirable to modify the implementation to more efficiently
// handle such encodings.
//
// Although 'int' is signed, this module does not permit negative
// character indices, as no encoding system uses them.  Why, then, not
// use 'unsigned int'?  A full explanation is beyond the scope of this
// module's documentation, but briefly, 'unsigned' turns out to be a
// poor way of saying "non-negative" because 'unsigned' alters the
// semantics of arithmetic and the implicit conversions in C/C++ make
// it almost meaningless anyway.

#ifndef BDFFONT_H
#define BDFFONT_H

#include "array.h"           // ArrayStack
#include "objlist.h"         // ObjList
#include "point.h"           // point
#include "sm-macros.h"       // NO_OBJECT_COPIES
#include "str.h"             // string

class Bit2d;                 // bit2d.h

// An entire BDF font.  The class itself is responsible only for
// storage of the information.  Global functions (declared below) then
// operate upon the font objects for purposes such as parsing.
class BDFFont {
  // Copying could be added if needed, but is not currently implemented.
  NO_OBJECT_COPIES(BDFFont);

public:      // static data
  static int s_objectCount;

public:      // types
  // This type is used to store values identified in the spec as
  // having type "number".  I would like to use a general 'rational'
  // type, but I haven't made one yet.  I do not want to use 'float'
  // because it loses information for most decimal values.  'string'
  // will do for now since I have no intent to do anything with these
  // values beside store them.
  //
  // All initial values for fields of type Number are "".
  typedef string Number;

  // A single "property", which consists of a name and a value, where
  // the value is either an integer or a string.
  class Property {
  public:    // data
    // Property name.
    string name;

    // True if the value is an integer, false if it is a string.
    bool isInteger;

    // The integer value, if 'isInteger' is true.
    int intValue;

    // The string value, if 'isInteger' is false.
    string stringValue;

  public:    // funcs
    Property(rostring name, int intValue);
    Property(rostring name, string stringValue);
    ~Property();
  };

  // Metrics applicable to an individual glyph.
  //
  // For my purposes, only the bb* and dWidth* values are interesting.
  class GlyphMetrics {
  public:    // data
    // The four values after the FONTBOUNDINGBOX or BBX keyword.  All
    // values are in pixels, and initially 0.
    //
    // Meaning I infer: For any glyph, there is a notion of the origin
    // to which its specification is relative.  The offsets position the
    // bounding box ("bb") lower left corner, and then the width and
    // height provide its size.
    //
    // For the font-wide FONTBOUNDINGBOX, it should be the case that
    // every glyph in the font has its black pixels contained inside
    // the font bounding box; the spec does not explicitly say this,
    // however.  The spec also does not indicate whether it is
    // required that the given bounding box is the smallest such box.
    point bbSize;            // width and height
    point bbOffset;          // positive means bb is right/above

    // The values after the SWIDTH keyword, providing the "scalable
    // width".  See the spec for details.
    Number sWidthX;
    Number sWidthY;

    // The values after the DWIDTH keyword.  Taken together, these
    // provide a vector specifying how to move the origin after
    // rendering each glyph, in writing mode 0 (left-to-right).  The
    // font-wide values can be overridden by individual glyphs.
    // Initially both 0.
    point dWidth;

    // True if DWIDTH appeared for this glyph.  Initially false, which
    // means the font-width DWIDTH should be used for this glyph.
    bool dWidthSpecified;

    // The values after the SWIDTH1 keyword, providing the "scalable
    // width" in writing mode 1 (top-to-bottom).  See spec.
    Number sWidthX1;
    Number sWidthY1;

    // The values after the DWIDTH1 keyword, which is the origin
    // offset vector for writing mode 1.  Initially 0.
    point dWidth1;
    bool dWidth1Specified;

    // The values after the VVECTOR keyword.  Taken together, these
    // provide a vector from origin 0 to origin 1, which are the
    // origins for the two writing modes.  Initially 0.
    point vVector;

  public:    // funcs
    GlyphMetrics();
    ~GlyphMetrics();

    // Return true if the 'dWidth' is specified.
    bool hasDWidth() const { return dWidthSpecified; }
  };

  // Data for a single glyph.
  class Glyph {
  public:    // data
    // The value following STARTCHAR.  In some cases I take it this
    // may be a number rendered in decimal, and in such cases, that
    // number is to be used as the glyph index.
    string name;

    // The first value following ENCODING.  Initially -1, which means
    // that the 'nonstdEncoding' value is to be used.
    int stdEncoding;

    // The second value following ENCODING, or (and initially) -1.
    int nonstdEncoding;

    // The per-glyph metrics from the BBX, SWIDTH, DWIDTH, SWITDH1,
    // DWIDTH1, and VVECTOR keywords.
    GlyphMetrics metrics;

    // Bitmap of black pixels, where 0 means white (non-printing) and
    // 1 means black (printing).  Initially NULL.  Once built,
    // 'bitmap' should be non-NULL, and 'bitmap->Size()' should equal
    // 'metrics.bbSize'.
    //
    // However, this is NULL if the size is (0,0).
    //
    // The 0,0 pixel of the bitmap is the upper-left corner of the
    // glyph image.  (Note that this means the interpretation of
    // increasing 'y' values is opposite here from in the bounding
    // box offset.)
    Bit2d *bitmap;                     // (owner)

  public:
    Glyph();
    ~Glyph();

    // Return the character index this glyph seems to want to use
    // based on the field values.
    int getCharacterIndex() const;
  };

public:      // data
  // Version number following the STARTFONT keyword.
  Number fileFormatVersion;

  // Sequence of COMMENT strings encountered in the file.  The order
  // of COMMENT lines relative to non-COMMENT lines is not retained.
  // Initially empty.
  ArrayStack<string> comments;

  // The integer specified for CONTENTVERSION, or 0 if absent.
  int contentVersion;

  // Font name following FONT keyword.
  string fontName;

  // First value after SIZE keyword.  Unfortunately the spec does not
  // say whether this is a "number" or an "integer", so I am inferring
  // from the example that it is an integer.  Initially 0.
  int pointSize;

  // The second and third values after the SIZE keyword.  I believe
  // these values are intended to be in pixels per inch.  Initially 0.
  point resolution;

  // Font-wide metrics, from the FONTBOUNDINGBOX keyword and the
  // font-wide SWIDTH, DWIDTH, SWIDTH1, DWIDTH1, and VVECTOR keywords.
  // For any glyph that does not specify its own value for the latter
  // five keywords, the values in this structure provide them.
  GlyphMetrics metrics;

  // The value after the METRICSSET keyword.  It should be 0, 1 or 2.
  // Initially 0, which is the default value if METRICSSET is not
  // specified.  This value indicates which writing directions are
  // supported by the supplied metrics: 0 means left-to-right, 1 means
  // top-to-bottom, and 2 means both of the preceding two.
  int metricsSet;

  // Sequence of properties between STARTPROPERTIES and ENDPROPERTIES.
  ObjList<Property> properties;

  // Map from character index to glyph.  Some entries will be NULL,
  // meaning no glyph is available for that character index.  The
  // index for a glyph can apparently come from a couple of different
  // glyph attributes; the parser must make a best effort guess as to
  // which to use.  The intent is if I want to render ASCII
  // characters, the ASCII code will be the index for the
  // corresponding glyph.
  //
  // See note at top of file about character indices and encodings.
  ObjArrayStack<Glyph> glyphs;

public:      // funcs
  BDFFont();
  ~BDFFont();

  // Maximum index for which a glyph is present, or -1 if no glyphs
  // are present.
  int maxValidGlyph() const;

  // Retrieve the glyph for a particular character code, or NULL if no
  // such glyph is defined.  Calling this function is preferable to
  // directly accessing 'glyphs' when possible.  The returned pointer
  // may be invalidated by subsequent modifications to 'this', so
  // should not be stored long-term.
  //
  // See note at top of file about character indices and encodings.
  //
  // Returns NULL if 'charIndex' is negative.
  Glyph const * /*nullable*/ getGlyph(int charIndex) const;

  // Return one greater than the maximum valid glyph index, or 0 if
  // none are valid.
  int glyphIndexLimit() const;
};


// Parse an in-memory string containing the BDF file format.  Throws
// XFormat if it encounters a format violation.
void parseBDFString(BDFFont &destFont, char const *bdfSourceData);

// Parse an on-disk file in BDF file format.  Throws xSysError if the
// file cannot be read, and XFormat if there is a format violation.
void parseBDFFile(BDFFont &destFont, char const *bdfFileName);


// Write a BDF font to a string representation in the BDF file format.
void writeBDFString(stringBuilder &dest, BDFFont const &font);

// Write it to a disk file.
void writeBDFFile(char const *destFname, BDFFont const &font);


#endif // BDFFONT_H

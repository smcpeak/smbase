// bdffont.cc
// code for bdffont.h

#include "bdffont.h"         // this module

#include "bit2d.h"           // Bit2d
#include "exc.h"             // xformat
#include "objcount.h"        // CHECK_OBJECT_COUNT
#include "owner.h"           // Owner
#include "strutil.h"         // readStringFromFile

#include <unistd.h>          // unlink


// ---------------------- BDFFont::Property ------------------------
BDFFont::Property::Property(rostring n, int i)
  : name(n),
    isInteger(true),
    intValue(i),
    stringValue()
{}


BDFFont::Property::Property(rostring n, string s)
  : name(n),
    isInteger(false),
    intValue(0),
    stringValue(s)
{}


BDFFont::Property::~Property()
{}


// ------------------- BDFFont::GlyphMetrics ----------------------
BDFFont::GlyphMetrics::GlyphMetrics()
  : bbSize(0,0),
    bbOffset(0,0),
    sWidthX(),
    sWidthY(),
    dWidth(0,0),
    dWidthSpecified(false),
    sWidthX1(),
    sWidthY1(),
    dWidth1(0,0),
    dWidth1Specified(false),
    vVector(0,0)
{}


BDFFont::GlyphMetrics::~GlyphMetrics()
{}


// ---------------------- BDFFont::Glyph --------------------------
BDFFont::Glyph::Glyph()
  : name(),
    stdEncoding(-1),
    nonstdEncoding(-1),
    metrics(),
    bitmap(NULL)
{}


BDFFont::Glyph::~Glyph()
{
  if (bitmap) {
    delete bitmap;
  }
}


int BDFFont::Glyph::getCharacterIndex() const
{
  // The spec has text suggesting that perhaps 'name' should be
  // considered as an index, but I'm going to ignore that.

  if (stdEncoding >= 0) {
    return stdEncoding;
  }
  else {
    return nonstdEncoding;
  }
}


// -------------------------- BDFFont -----------------------------
int BDFFont::s_objectCount = 0;

CHECK_OBJECT_COUNT(BDFFont);


BDFFont::BDFFont()
  : fileFormatVersion(),
    comments(),
    contentVersion(0),
    fontName(),
    pointSize(0),
    resolution(0,0),
    metrics(),
    metricsSet(0),
    properties(),
    glyphs()
{
  s_objectCount++;
}

BDFFont::~BDFFont()
{
  s_objectCount--;
}


int BDFFont::maxValidGlyph() const
{
  int ret = glyphs.length() - 1;
  while (ret >= 0 && glyphs[ret] == NULL) {
    ret--;
  }
  return ret;
}


BDFFont::Glyph const * /*nullable*/ BDFFont::getGlyph(int charIndex) const
{
  if (0 <= charIndex &&
           charIndex < glyphs.length()) {
    return glyphs[charIndex];
  }
  else {
    // outside range of what is stored in the array
    return NULL;
  }
}


int BDFFont::glyphIndexLimit() const
{
  return glyphs.length();
}


// ------------------------- BDF parser ---------------------------
#define XFORMAT(stuff) xformat(stringb(stuff))

// Expect 'expected' to appear next, and skip it.  Otherwise,
// throw xFormat.
static void expect(char const *&p, char const *expected)
{
  char const *origP = p;
  char const *origExpected = expected;

  while (*expected != 0 && *p == *expected) {
    p++;
    expected++;
  }

  if (*expected != 0) {
    XFORMAT("expected \"" << origExpected <<
            "\", but found \"" << string(origP, p-origP) << "\"");
  }
}


// Skip any blanks and newlines.
//
// Although the spec does not say anything about tolerating blank
// lines, xmbdef creates BDF files with blank lines, so I need to
// accept them.
static void skipBlanks(char const *&p)
{
  while (*p == ' ' || *p == '\r' || *p == '\n') {
    p++;
  }
}


// Skip zero or more spaces.
void skipSpacesOpt(char const *&p)
{
  while (*p == ' ') {
    p++;
  }
}

// Skip at least one space.
static void skipSpaces(char const *&p)
{
  if (*p != ' ') {
    XFORMAT("expected a space, but found '" << *p << "'");
  }

  skipSpacesOpt(p);
}


// Read characters up to the next newline.  Skip the newline.
static string parseString(char const *&p)
{
  stringBuilder ret;
  for (; *p != 0 && *p != '\n'; p++) {
    if (*p == '\r') {
      // The spec says every line ends with CRLF, but my practical
      // expectation is I'll see plenty of files with only LF (and in
      // fact my fonts in ~/wrk/fonts have only LF).  Since the spec
      // also says that CR cannot occur inside any value, I'll just
      // skip any CR I see.
      continue;
    }

    ret << *p;
  }

  if (ret.empty()) {
    xformat("expected a string");
  }

  // skip the newline
  if (*p == '\n') {
    p++;
  }

  return ret.str();
}


// Read characters up to next space.  Skip the spaces.
static string parseWord(char const *&p)
{
  stringBuilder ret;

  for (; *p != 0 && *p != ' ' && *p != '\r' && *p != '\n'; p++) {
    ret << *p;
  }

  if (ret.empty()) {
    xformat("expected a word");
  }

  skipSpacesOpt(p);

  return ret.str();
}


// Read a decimal integer.  Skip any following spaces (but not newlines).
static int parseInteger(char const *&p)
{
  int ret = 0;
  bool neg = false;

  if (*p == '-') {
    neg = true;
    p++;
  }

  if (!( '0' <= *p && *p <= '9' )) {
    XFORMAT("expected a digit: '" << *p << "'");
  }

  for (; '0' <= *p && *p <= '9'; p++) {
    ret = ret * 10 + (*p - '0');
  }

  skipSpacesOpt(p);

  return neg? -ret : ret;
}


// Read two decimal integers as a point, x then y.
static point parsePoint(char const *&p)
{
  point ret;
  ret.x = parseInteger(p);
  ret.y = parseInteger(p);
  return ret;
}


// Skip a newline, optionally preceded by spaces.
static void skipNewline(char const *&p)
{
  while (*p == '\r' || *p == ' ') {
    p++;
  }
  if (*p != '\n') {
    xformat("expected a newline");
  }
  p++;
}


// Parse a "number" which is a decimal fractional value.  Return the
// representation string.  Skip any following spaces.
static string parseNumber(char const *&p)
{
  char const *orig = p;

  while (*p == '-' || *p == '.' || ('0' <= *p && *p <= '9')) {
    p++;
  }

  if (p == orig) {
    XFORMAT("expected a decimal value: '" << *p << "'");
  }

  string ret(orig, p-orig);

  skipSpacesOpt(p);

  return ret;
}


// If 'keyword' is a metrics attribute keyword, then parse its
// following values at 'p' into 'metrics' and return true.  Otherwise,
// return false.
static bool parseMetricsAttribute(char const *&p, rostring keyword,
                                  BDFFont::GlyphMetrics &metrics)
{
  if (keyword == "SWIDTH") {
    metrics.sWidthX = parseNumber(p);
    metrics.sWidthY = parseNumber(p);
    skipNewline(p);
    return true;
  }

  else if (keyword == "SWIDTH1") {
    metrics.sWidthX1 = parseNumber(p);
    metrics.sWidthY1 = parseNumber(p);
    skipNewline(p);
    return true;
  }

  else if (keyword == "DWIDTH") {
    metrics.dWidth = parsePoint(p);
    metrics.dWidthSpecified = true;
    skipNewline(p);
    return true;
  }

  else if (keyword == "DWIDTH1") {
    metrics.dWidth1 = parsePoint(p);
    metrics.dWidth1Specified = true;
    skipNewline(p);
    return true;
  }

  else if (keyword == "VVECTOR") {
    metrics.vVector = parsePoint(p);
    skipNewline(p);
    return true;
  }

  else {
    return false;
  }
}


// Parse a quoted string at 'p'.
static string parseQuotedString(char const *&p)
{
  expect(p, "\"");

  stringBuilder ret;

  for (;; p++) {
    if (*p == 0) {
      XFORMAT("input ended while inside quoted string");
    }

    if (*p == '\n') {
      XFORMAT("found newline in quoted string");
    }

    if (*p == '"') {
      p++;
      if (*p == '"') {
        // is an escaped double-quote
        ret << *p;
      }
      else {
        // is the end of the quoted string
        break;
      }
    }
    else {
      ret << *p;
    }
  }

  return ret;
}


// Parse 'numProps' of property lines, plus the final ENDPROPERTIES line.
static void parseProperties(char const *&p, int numProps,
                            ObjList<BDFFont::Property> &properties)
{
  for (int i=0; i < numProps; i++) {
    string name = parseWord(p);
    if (name == "ENDPROPERTIES") {
      XFORMAT("unexpected ENDPROPERTIES; only read " << i <<
              " out of " << numProps << " properties");
    }

    if (*p == '"') {
      properties.prepend(new BDFFont::Property(name, parseQuotedString(p)));
    }
    else {
      properties.prepend(new BDFFont::Property(name, parseInteger(p)));
    }
    skipNewline(p);
  }

  properties.reverse();

  string end = parseWord(p);
  if (end != "ENDPROPERTIES") {
    XFORMAT("expected ENDPROPERTIES, but got: " << end);
  }
  skipNewline(p);
}


// Parse the arguments after FONTBOUNDINGBOX or BBX.
static void parseBoundingBox(char const *&p, BDFFont::GlyphMetrics &metrics)
{
  metrics.bbSize = parsePoint(p);

  if (metrics.bbSize.x < 0 || metrics.bbSize.y < 0) {
    XFORMAT("bounding box must have non-negative dimensions, but is " <<
            metrics.bbSize);
  }

  metrics.bbOffset = parsePoint(p);

  skipNewline(p);
}


// Parse a single hex digit.
static byte parseHexDigit(char const *&p)
{
  if ('0' <= *p && *p <= '9') {
    return *(p++) - '0';
  }

  if ('A' <= *p && *p <= 'F') {
    return *(p++) - 'A' + 10;
  }

  if ('a' <= *p && *p <= 'f') {
    return *(p++) - 'a' + 10;
  }

  XFORMAT("expected hex digit: '" << *p << "'");
  return 0;   // silence warning
}


// Parse the bitmap lines into 'bitmap', which already has the proper
// size.
static void parseBitmap(char const *&p, Bit2d &bitmap)
{
  // The expected length of each line of text: bitmap width rounded up
  // to nearest multiple of 8, then divided by two because each octet
  // is represented with two hex characters.
  int textLength = ((bitmap.Size().x + 7) / 8) * 2;

  for (int y=0; y < bitmap.Size().y; y++) {
    if (0==memcmp(p, "ENDCHAR", 7)) {
      XFORMAT("unexpected ENDCHAR after reading " << y <<
              " of " << bitmap.Size().y << " lines");
    }

    // Interpret pairs of hex bytes.
    for (int offset=0; offset < textLength; offset += 2) {
      byte b1 = parseHexDigit(p);
      byte b2 = parseHexDigit(p);

      // this has pixel 0 in the MSB
      byte bits = (b1 << 4) | b2;

      // Check that padding bits are 0.
      if (offset * 4 + 8 > bitmap.Size().x) {
        int numPadBits = (offset * 4 + 8) - bitmap.Size().x;
        xassert(0 < numPadBits && numPadBits < 8);

        // This will have 1s where all the pad bits are.
        int padMask = ((1 << numPadBits) - 1);

        if (bits & padMask) {
          xformat(stringf("final byte 0x%02X has non-zero bits in pad mask 0x%02X",
                          bits, padMask));
        }
      }

      // flip the bits around so pixel 0 is the LSB, since
      // that is what 'bit2d::set8' wants
      bits = byteBitSwapLsbMsb(bits);

      // store it
      bitmap.set8(point(offset * 4, y), bits);
    }

    skipNewline(p);
  }
}


// Parse the attributes of 'glyph' at 'p'.
static void parseGlyph(char const *&p, BDFFont::Glyph *glyph,
                       BDFFont const &font)
{
  bool sawBBX = false;

  for (;;) {
    skipBlanks(p);
    string keyword = parseWord(p);

    try {
      if (keyword == "ENCODING") {
        glyph->stdEncoding = parseInteger(p);
        if (glyph->stdEncoding < 0) {
          if (glyph->stdEncoding != -1) {
            XFORMAT("a negative number following ENCODING must be -1, not " <<
                    glyph->stdEncoding);
          }

          glyph->nonstdEncoding = parseInteger(p);
          if (glyph->nonstdEncoding < 0) {
            XFORMAT("the non-standard encoding value must be non-negative, not " <<
                    glyph->nonstdEncoding);
          }
        }
        skipNewline(p);
      }

      else if (parseMetricsAttribute(p, keyword, glyph->metrics)) {
        // already parsed it
      }

      else if (keyword == "BBX") {
        parseBoundingBox(p, glyph->metrics);
        sawBBX = true;
      }

      else if (keyword == "BITMAP") {
        skipNewline(p);

        // The BBX must have already been specified so we know how big
        // a bitmap to make.
        if (!sawBBX) {
          XFORMAT("encountered BITMAP before BBX");
        }

        if (!glyph->metrics.bbSize.isZero()) {
          glyph->bitmap = new Bit2d(glyph->metrics.bbSize);
          parseBitmap(p, *(glyph->bitmap));
        }

        expect(p, "ENDCHAR");
        skipNewline(p);

        // Make sure we have everything.
        if (glyph->getCharacterIndex() == -1) {
          XFORMAT("missing ENCODING attribute");
        }

        // TODO: Check with METRICSSET and 'font' to make sure
        // we have all the required metrics.

        break;
      }

      else {
        XFORMAT("unknown glyph attribute \"" << keyword << "\"");
      }
    }
    catch (xBase &x) {
      x.prependContext(keyword);
      throw;
    }
  }
}


// Parse 'numChars' characters into 'font.glyphs'.
static void parseChars(char const *&p, int numChars, BDFFont &font)
{
  for (int i=0; i<numChars; i++) {
    skipBlanks(p);
    expect(p, "STARTCHAR");
    skipSpaces(p);

    // I store this in a local, in addition to 'glyph->name', so that
    // I can be sure it will be available in the exception handler
    // even after 'glyph' itself might become NULL.
    string glyphName = parseString(p);

    Owner<BDFFont::Glyph> glyph(new BDFFont::Glyph);
    glyph->name = glyphName;

    try {
      // parse attributes and store them in 'glyph'
      parseGlyph(p, glyph, font);

      // choose an index for the glyph
      int index = glyph->getCharacterIndex();
      if (index < 0) {
        XFORMAT("invalid negative index: " << index);
      }

      // test it for uniqueness
      if (BDFFont::Glyph const *other = font.getGlyph(index)) {
        XFORMAT("glyph index " << index <<
                "collides with \"" << other->name << "\"");
      }

      // put the glyph into the font
      while (font.glyphs.length() <= index) {
        font.glyphs.push(NULL);
      }
      BDFFont::Glyph const *g = font.glyphs.swapAt(index, glyph.xfr());
      xassert(g == NULL);
    }
    catch (xBase &x) {
      x.prependContext(stringb("glyph \"" << glyphName << "\""));
      throw;
    }
  }
}


// Return a string of the form "<line>:<col>" describing where 'end'
// is, if 'start' ss 1:1.
static string getLineCol(char const *start, char const *end)
{
  int line = 1;
  int col = 1;

  for (char const *p = start; p < end; p++) {
    if (*p == '\n') {
      line++;
      col = 1;
    }
    else {
      col++;
    }
  }

  return stringb(line << ":" << col);
}


void parseBDFString(BDFFont &font, char const *bdfSourceData)
{
  // pointer to next character to process
  char const *p = bdfSourceData;

  try {
    // STARTFONT should be first
    expect(p, "STARTFONT");
    skipSpaces(p);
    font.fileFormatVersion = parseString(p);

    // drop into loop reading font-wide characteristics
    for (;;) {
      skipBlanks(p);
      string keyword = parseWord(p);

      try {
        if (keyword == "COMMENT") {
          font.comments.push(parseString(p));
        }

        else if (keyword == "CONTENTVERSION") {
          font.contentVersion = parseInteger(p);
          skipNewline(p);
        }

        else if (keyword == "FONT") {
          font.fontName = parseString(p);
        }

        else if (keyword == "SIZE") {
          font.pointSize = parseInteger(p);
          font.resolution = parsePoint(p);
          skipNewline(p);
        }

        else if (keyword == "FONTBOUNDINGBOX") {
          parseBoundingBox(p, font.metrics);
        }

        else if (keyword == "METRICSSET") {
          font.metricsSet = parseInteger(p);
          if (!( 0 <= font.metricsSet && font.metricsSet <= 2 )) {
            XFORMAT("METRICSSET should be in [0,2], not " << font.metricsSet);
          }
          skipNewline(p);
        }

        else if (parseMetricsAttribute(p, keyword, font.metrics)) {
          // already parsed it
        }

        else if (keyword == "STARTPROPERTIES") {
          int numProps = parseInteger(p);
          skipNewline(p);
          parseProperties(p, numProps, font.properties);
        }

        else if (keyword == "CHARS") {
          int numChars = parseInteger(p);
          skipNewline(p);

          // Make sure we got everything we were supposed to from the
          // font-wide attributes.
          if (font.fontName.empty()) {
            xformat("missing FONT attribute");
          }
          if (font.pointSize == 0) {
            xformat("missing SIZE attribute");
          }
          if (font.metrics.bbSize.isZero()) {
            xformat("missing FONTBOUNDINGBOX attribute");
          }

          parseChars(p, numChars, font);

          skipBlanks(p);
          expect(p, "ENDFONT");
          skipNewline(p);
          break;
        }

        else {
          XFORMAT("unknown font attribute \"" << keyword << "\"");
        }
      }
      catch (xBase &x) {
        x.prependContext(keyword);
        throw;
      }
    }
  }

  catch (xBase &x) {
    x.prependContext(getLineCol(bdfSourceData, p));
    throw;
  }
}


void parseBDFFile(BDFFont &font, char const *bdfFileName)
{
  try {
    string contents = readStringFromFile(bdfFileName);
    parseBDFString(font, contents.c_str());
  }
  catch (xBase &x) {
    x.prependContext(bdfFileName);
    throw;
  }
}


#undef XFORMAT


// ------------------------ BDF writer -------------------------
// For now, always write LF, not CRLF ...
#define EOL "\n"


// Render the point as "<x> <y>", i.e., with a space.
static string writePoint(point const &p)
{
  return stringb(p.x << " " << p.y);
}


// Write SWITDH, etc., from 'metrics' to 'dest'.
static void writeMetrics(stringBuilder &dest,
                         BDFFont::GlyphMetrics const &metrics)
{
  if (!metrics.sWidthX.empty()) {
    dest << "SWIDTH " << metrics.sWidthX << " " << metrics.sWidthY << EOL;
  }

  if (metrics.hasDWidth()) {
    dest << "DWIDTH " << writePoint(metrics.dWidth) << EOL;
  }

  if (!metrics.sWidthX1.empty()) {
    dest << "SWIDTH1 " << metrics.sWidthX1 << " " << metrics.sWidthY1 << EOL;
  }

  if (metrics.dWidth1Specified) {
    dest << "DWIDTH1 " << writePoint(metrics.dWidth1) << EOL;
  }

  if (!metrics.vVector.isZero()) {
    dest << "VVECTOR " << writePoint(metrics.vVector) << EOL;
  }
}


// Write 'prop' to 'dest'.
static void writeProperty(stringBuilder &dest, BDFFont::Property const &prop)
{
  dest << prop.name << " ";

  if (prop.isInteger) {
    dest << prop.intValue;
  }
  else {
    // write out as quoted string with escaped quotes
    dest << "\"";
    for (char const *p = prop.stringValue.c_str(); *p; p++) {
      if (*p == '"') {
        dest << "\"\"";
      }
      else {
        dest << *p;
      }
    }
    dest << "\"";
  }

  dest << EOL;
}


// Write 'bitmap' to 'dest'.
static void writeBitmap(stringBuilder &dest, Bit2d const &bitmap)
{
  for (int y=0; y < bitmap.Size().y; y++) {
    for (int x=0; x < bitmap.Size().x; x += 8) {
      // should come back with 0s in padding bits
      byte bits = bitmap.get8(point(x,y));

      // flip order
      bits = byteBitSwapLsbMsb(bits);

      dest << stringf("%02X", bits);
    }
    dest << EOL;
  }
}


// Write 'glyph' to 'dest'.
static void writeGlyph(stringBuilder &dest, BDFFont::Glyph const &glyph)
{
  dest << "STARTCHAR " << glyph.name << EOL;

  dest << "ENCODING " << glyph.stdEncoding;
  if (glyph.stdEncoding < 0) {
    dest << " " << glyph.nonstdEncoding;
  }
  dest << EOL;

  // Write these before BBX to match fonts/sample2.bdf.
  writeMetrics(dest, glyph.metrics);

  dest << "BBX " << writePoint(glyph.metrics.bbSize)
       << " " << writePoint(glyph.metrics.bbOffset) << EOL;

  dest << "BITMAP" << EOL;
  if (glyph.bitmap) {
    writeBitmap(dest, *(glyph.bitmap));
  }

  dest << "ENDCHAR" << EOL;
}


// I'm writing these out with LF rather than CRLF ...
void writeBDFString(stringBuilder &dest, BDFFont const &font)
{
  dest << "STARTFONT " << font.fileFormatVersion << EOL;

  for (int i=0; i < font.comments.length(); i++) {
    dest << "COMMENT " << font.comments[i] << EOL;
  }

  if (font.contentVersion) {
    dest << "CONTENTVERSION " << font.contentVersion << EOL;
  }

  dest << "FONT " << font.fontName << EOL;

  dest << "SIZE " << font.pointSize
       << " " << writePoint(font.resolution) << EOL;

  dest << "FONTBOUNDINGBOX " << writePoint(font.metrics.bbSize)
       << " " << writePoint(font.metrics.bbOffset) << EOL;

  writeMetrics(dest, font.metrics);

  dest << "METRICSSET " << font.metricsSet << EOL;

  if (font.properties.isNotEmpty()) {
    dest << "STARTPROPERTIES " << font.properties.count() << EOL;
    FOREACH_OBJLIST(BDFFont::Property, font.properties, iter) {
      writeProperty(dest, *(iter.data()));
    }
    dest << "ENDPROPERTIES" << EOL;
  }

  // Write the glyphs to an intermediate buffer so we can count
  // them in the same loop.
  stringBuilder glyphBuf;
  int glyphCount = 0;
  for (int i=0; i < font.glyphs.length(); i++) {
    BDFFont::Glyph const *glyph = font.glyphs[i];
    if (!glyph) {
      continue;
    }

    glyphCount++;
    writeGlyph(glyphBuf, *glyph);
  }

  dest << "CHARS " << glyphCount << EOL;
  dest << glyphBuf.str();
  dest << "ENDFONT" << EOL;
}


void writeBDFFile(char const *fname, BDFFont const &font)
{
  try {
    stringBuilder buf;
    writeBDFString(buf, font);
    writeStringToFile(buf.str(), fname);
  }
  catch (xBase &x) {
    x.prependContext(stringb("writing font \"" << font.fontName <<
                             "\" to file " << fname));
    throw;
  }
}


// -------------------------- test code ---------------------------
#ifdef TEST_BDFFONT

#include "sm-test.h"         // USUAL_TEST_MAIN


static void entry()
{
  cout << "bdffont tests" << endl;

  // parse a file
  //
  // Amusingly, the actual sample input in the spec is missing a
  // bitmap line for the "quoteright" character!  I have repaired it
  // in my version of the input.
  //
  // I've made some other changes as well to test some syntax
  // variations and another anomalies.
  BDFFont font;
  parseBDFFile(font, "fonts/sample1.bdf");

  // write it out
  writeBDFFile("tmp.bdf", font);

  // The output should match sample1out.bdf, which is the same as sample1
  // except that "j" comes after "quoteright" and METRICSSET is
  // explicit.
  if (readStringFromFile("fonts/sample1out.bdf") != readStringFromFile("tmp.bdf")) {
    xfatal("fonts/sample1out.bdf and tmp.bdf differ!");
  }

  (void)unlink("tmp.bdf");

  if (char const *otherTest = getenv("BDFFONT_OTHERTEST")) {
    cout << "testing " << otherTest << endl;

    BDFFont otherFont;
    parseBDFFile(otherFont, otherTest);
    writeBDFFile("tmp.bdf", otherFont);
  }

  cout << "bdffont ok\n";
}

USUAL_TEST_MAIN


#endif // TEST_BDFFONT

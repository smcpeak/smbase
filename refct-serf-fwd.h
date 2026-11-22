// refct-serf-fwd.h
// Forward decls for `refct-serf.h`.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_REFCT_SERF_FWD_H
#define SMBASE_REFCT_SERF_FWD_H

class SerfRefCount;
class RCSerfPrivateHelpers;

template <class T>
class RCSerf;

// A nullable `RCSerf`.
//
// For now this is just the same class, and its use just emphasizes the
// null possibility, but I'm considering making it a separate class at
// some point.
template <typename T>
using RCSerfOpt = RCSerf<T>;

// Non-null `RCSerf`.
//
// This too is, for the moment, effectively just a form of
// documentation, but in the future might be its own class.
template <typename T>
using NNRCSerf = RCSerf<T>;

#endif // SMBASE_REFCT_SERF_FWD_H

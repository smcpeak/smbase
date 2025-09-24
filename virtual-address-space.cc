// virtual-address-space.cc
// Code for `virtual-address-space` module.

#include "virtual-address-space.h"     // this module

#include "smbase/chained-cond.h"       // smbase::cc::z_le_lt
#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/overflow.h"           // addWithOverflowCheck
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/sum-tree.h"           // smbase::SumTree [n]

using namespace gdv;


OPEN_NAMESPACE(smbase)


// ------------------------- GlobalASFragment --------------------------
auto VirtualASManager::GlobalASFragment::summary() const -> GlobalOffset
{
  return m_size;
}


void VirtualASManager::GlobalASFragment::selfCheck() const
{
  xassert(m_localStart >= 0);
  xassert(m_size >= 0);
}


VirtualASManager::GlobalASFragment::operator gdv::GDValue() const
{
  GDValue m(GDVK_TAGGED_ORDERED_MAP, "GlobalASFragment"_sym);

  GDV_WRITE_MEMBER_SYM(m_vas);
  GDV_WRITE_MEMBER_SYM(m_size);

  return m;
}


// -------------------------- LocalASFragment --------------------------
auto VirtualASManager::LocalASFragment::summary() const -> LocalOffset
{
  return m_size;
}


void VirtualASManager::LocalASFragment::selfCheck() const
{
  xassert(m_globalStart >= 0);
  xassert(m_size >= 0);
}


VirtualASManager::LocalASFragment::operator gdv::GDValue() const
{
  GDValue m(GDVK_TAGGED_ORDERED_MAP, "LocalASFragment"_sym);

  GDV_WRITE_MEMBER_SYM(m_globalStart);
  GDV_WRITE_MEMBER_SYM(m_size);

  return m;
}


// ------------------------- VirtualASManager --------------------------
VirtualASManager::~VirtualASManager()
{}


VirtualASManager::VirtualASManager()
  : m_globalFragments(new SumTree<GlobalASFragment>),
    m_vasToLocalFragments()
{}


void VirtualASManager::selfCheck() const
{
  m_globalFragments->selfCheck();

  // Check each local tree.
  for (VASID vas = 0; vas < numLocalSpaces(); ++vas) {
    m_vasToLocalFragments.at(vas)->selfCheck();
  }

  // Next global fragment to check
  std::size_t globalIndex = 0;

  // Where we expect the next fragment to start globally.
  GlobalOffset globalStart = 0;

  // Map from VASID to the index of its tree within
  // `m_vasToLocalFragments` of the next fragment to check.
  std::vector<std::size_t> vasToLocalIndex(numLocalSpaces(), 0);

  // For each VAS, where we expect its next fragment to start.
  std::vector<LocalOffset> vasToLocalStart(numLocalSpaces(), 0);

  // Walk the global fragments, checking that each corresponds to the
  // next local fragment for its VAS.
  while (globalIndex < m_globalFragments->size()) {
    // Look up `globalIndex`.
    GlobalASFragment const &globalFrag =
      m_globalFragments->atC(globalIndex);
    globalFrag.selfCheck();

    // Look up the VASID.
    VASID const vas = globalFrag.m_vas;
    xassert(cc::z_le_lt(vas, numLocalSpaces()));
    SumTree<LocalASFragment> const &localFragments =
      *( m_vasToLocalFragments.at(vas) );
    std::size_t &localIndex =
      vasToLocalIndex.at(vas);
    LocalOffset &localStart =
      vasToLocalStart.at(vas);

    // Look up `localIndex`.
    xassert(cc::z_le_lt(localIndex, localFragments.size()));
    LocalASFragment const &localFrag =
      localFragments.atC(localIndex);
    localFrag.selfCheck();

    // Check correspondences.
    xassert(localStart == globalFrag.m_localStart);
    xassert(globalStart == localFrag.m_globalStart);
    xassert(globalFrag.m_size == localFrag.m_size);

    // Advance to next global fragment.
    ++globalIndex;
    globalStart += globalFrag.m_size;

    // Advance to next local fragment.
    ++localIndex;
    localStart += localFrag.m_size;
  }

  // Check that we got to the end of the global space.
  xassert(globalStart == globalSpaceSize());

  // Check all of the final local data.
  for (VASID vas = 0; vas < numLocalSpaces(); ++vas) {
    // Look up `vas`.
    SumTree<LocalASFragment> const &localFragments =
      *( m_vasToLocalFragments.at(vas) );
    std::size_t localIndex =
      vasToLocalIndex.at(vas);
    LocalOffset localStart =
      vasToLocalStart.at(vas);

    // Check that we got to the end of the local space.
    xassert(localIndex == localFragments.size());
    xassert(localStart == localFragments.summary());
    xassert(localStart == localSpaceSize(vas));
  }
}


// ------------------------------ Queries ------------------------------
auto VirtualASManager::globalSpaceSize() const -> GlobalOffset
{
  return m_globalFragments->summary();
}


auto VirtualASManager::numLocalSpaces() const -> VASID
{
  return m_vasToLocalFragments.size();
}


bool VirtualASManager::validLocalSpace(VASID vas) const
{
  return cc::z_le_lt(vas, numLocalSpaces());
}


auto VirtualASManager::localSpaceSize(VASID vas) const -> LocalOffset
{
  xassertPrecondition(validLocalSpace(vas));
  return m_vasToLocalFragments.at(vas)->summary();
}


auto VirtualASManager::localToGlobal(
  VASID vas, LocalOffset offset) const -> GlobalOffset
{
  xassertPrecondition(validLocalSpace(vas));
  xassertPrecondition(cc::z_le_lt(offset, localSpaceSize(vas)));

  // Get info for `vas`.
  SumTree<LocalASFragment> const &localFragments =
    m_vasToLocalFragments.at(vas).operator*();

  // Get the specific fragment that contains `offset`, along with how
  // far into that fragment `offset` is.
  auto [localFrag, fragOffset] =
    localFragments.lookup(offset);

  return localFrag.m_globalStart + fragOffset;
}


auto VirtualASManager::globalToLocal(GlobalOffset offset) const
  -> std::pair<VASID, LocalOffset>
{
  xassertPrecondition(cc::z_le_lt(offset, globalSpaceSize()));

  std::pair<GlobalASFragment const &, GlobalOffset> fragOfs =
    m_globalFragments->lookup(offset);

  return {fragOfs.first.m_vas,
          fragOfs.first.m_localStart + fragOfs.second};
}


// --------------------------- Modification ----------------------------
auto VirtualASManager::allocateLocalSpace() -> VASID
{
  VASID ret = numLocalSpaces();
  m_vasToLocalFragments.push_back(
    std::make_unique<SumTree<LocalASFragment>>());

  xassertPostcondition(numLocalSpaces() == ret + 1);
  xassertPostcondition(localSpaceSize(ret) == 0);

  return ret;
}


auto VirtualASManager::extendLocalSpace(VASID vas, LocalOffset size)
  -> GlobalOffset
{
  xassertPrecondition(validLocalSpace(vas));
  xassertPrecondition(size >= 0);

  GlobalOffset ret = globalSpaceSize();
  LocalOffset oldLocalSize = localSpaceSize(vas);

  m_globalFragments->append(GlobalASFragment{vas, oldLocalSize, size});

  m_vasToLocalFragments.at(vas)->append(LocalASFragment{ret, size});

  xassertPostcondition(globalSpaceSize() == ret + size);
  xassertPostcondition(localSpaceSize(vas) == oldLocalSize + size);

  return ret;
}


CLOSE_NAMESPACE(smbase)


// EOF

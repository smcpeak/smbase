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


// ---------------------------- VASFragment ----------------------------
auto VirtualASManager::VASFragment::summary() const -> GlobalOffset
{
  return m_size;
}


void VirtualASManager::VASFragment::selfCheck() const
{
  xassert(m_start >= 0);
  xassert(m_size >= 0);
}


VirtualASManager::VASFragment::operator gdv::GDValue() const
{
  GDValue m(GDVK_TAGGED_ORDERED_MAP, "VASFragment"_sym);

  GDV_WRITE_MEMBER_SYM(m_vas);
  GDV_WRITE_MEMBER_SYM(m_size);

  return m;
}


// ------------------------- VirtualASManager --------------------------
VirtualASManager::~VirtualASManager()
{}


VirtualASManager::VirtualASManager()
  : m_tree(new SumTree<VASFragment>),
    m_vasSizes()
{}


void VirtualASManager::selfCheck() const
{
  m_tree->selfCheck();

  // Map from `m_vas` to sum of sizes in `m_tree`.
  std::vector<LocalOffset> treeSizes(numLocalSpaces(), 0);

  // Traverse `m_tree` to populate `treeSize`.
  for (std::size_t i=0; i < m_tree->size(); ++i) {
    VASFragment const &frag = m_tree->atC(i);
    frag.selfCheck();

    // The start is the sum of all preceding sizes.
    xassert(frag.m_start == treeSizes.at(frag.m_vas));

    treeSizes.at(frag.m_vas) += frag.m_size;
  }

  // Should match `m_vasSizes`.
  xassert(m_vasSizes == treeSizes);
}


auto VirtualASManager::globalSpaceSize() const -> GlobalOffset
{
  return m_tree->summary();
}


auto VirtualASManager::numLocalSpaces() const -> VASID
{
  return m_vasSizes.size();
}


bool VirtualASManager::validLocalSpace(VASID vas) const
{
  return cc::z_le_lt(vas, numLocalSpaces());
}


auto VirtualASManager::localSpaceSize(VASID vas) const -> LocalOffset
{
  xassertPrecondition(validLocalSpace(vas));
  return m_vasSizes.at(vas);
}


auto VirtualASManager::globalToLocal(GlobalOffset offset) const
  -> std::pair<VASID, LocalOffset>
{
  xassertPrecondition(cc::z_le_lt(offset, globalSpaceSize()));

  std::pair<VASFragment const &, GlobalOffset> fragOfs =
    m_tree->lookup(offset);

  return {fragOfs.first.m_vas,
          fragOfs.first.m_start + fragOfs.second};
}


auto VirtualASManager::allocateLocalSpace() -> VASID
{
  VASID ret = numLocalSpaces();
  m_vasSizes.push_back(0);

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

  m_tree->append(VASFragment{vas, oldLocalSize, size});

  LocalOffset newLocalSize =
    addWithOverflowCheck<LocalOffset>(oldLocalSize, size);
  m_vasSizes.at(vas) = newLocalSize;

  xassertPostcondition(globalSpaceSize() == ret + size);
  xassertPostcondition(localSpaceSize(vas) == oldLocalSize + size);

  return ret;
}


CLOSE_NAMESPACE(smbase)


// EOF

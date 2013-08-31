// This file is part of the dune-gdt project:
//   http://users.dune-project.org/projects/dune-gdt
// Copyright holders: Felix Albrecht
// License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)
//
// Contributors: Kirsten Weber

#ifndef DUNE_GDT_BASEFUNCTIONSET_INTERFACE_HH
#define DUNE_GDT_BASEFUNCTIONSET_INTERFACE_HH

#include <vector>

#include <dune/common/bartonnackmanifcheck.hh>
#include <dune/common/fvector.hh>
#include <dune/common/fmatrix.hh>

namespace Dune {
namespace GDT {


/**
 *  \brief Interface for matrix valued basis functions.
 *
 *  \note   see specialization for rangeDimCols = 1 for vector and scalar valued and basis functions.
 */
template <class Traits, class DomainFieldImp, int domainDim, class RangeFieldImp, int rangeDimRows, int rangeDim = 1>
class BaseFunctionSetInterface;


/**
 *  \brief Interface for scalar valued basis functions.
 */
template <class Traits, class DomainFieldImp, int domainDim, class RangeFieldImp, int rangeDim>
class BaseFunctionSetInterface<Traits, DomainFieldImp, domainDim, RangeFieldImp, rangeDim, 1>
{
public:
  typedef typename Traits::derived_type derived_type;
  typedef typename Traits::BackendType BackendType;
  typedef typename Traits::EntityType EntityType;

  typedef DomainFieldImp DomainFieldType;
  static const unsigned int dimDomain = domainDim;
  typedef Dune::FieldVector<DomainFieldType, dimDomain> DomainType;
  typedef RangeFieldImp RangeFieldType;
  static const unsigned int dimRange     = rangeDim;
  static const unsigned int dimRangeCols = 1;
  typedef Dune::FieldVector<RangeFieldType, dimRange> RangeType;
  typedef Dune::FieldMatrix<RangeFieldType, dimRange, dimDomain> JacobianRangeType;

  const EntityType& entity() const
  {
    CHECK_INTERFACE_IMPLEMENTATION(asImp().entity());
    return asImp().entity();
  }

  const BackendType& backend() const
  {
    CHECK_INTERFACE_IMPLEMENTATION(asImp().backend());
    return asImp().backend();
  }

  size_t size() const
  {
    CHECK_INTERFACE_IMPLEMENTATION(asImp().size());
    return asImp().size();
  }

  size_t order() const
  {
    CHECK_INTERFACE_IMPLEMENTATION(asImp().order());
    return asImp().order();
  }

  void evaluate(const DomainType& x, std::vector<RangeType>& ret) const
  {
    CHECK_AND_CALL_INTERFACE_IMPLEMENTATION(asImp().evaluate(x, ret));
  }

  std::vector<RangeType> evaluate(const DomainType& xx) const
  {
    std::vector<RangeType> ret(size(), RangeType(0));
    evaluate(xx, ret);
    return ret;
  }

  void jacobian(const DomainType& x, std::vector<JacobianRangeType>& ret) const
  {
    CHECK_AND_CALL_INTERFACE_IMPLEMENTATION(asImp().jacobian(x, ret));
  }

  std::vector<JacobianRangeType> jacobian(const DomainType& xx) const
  {
    std::vector<JacobianRangeType> ret(size(), JacobianRangeType(0));
    jacobian(xx, ret);
    return ret;
  }

  derived_type& asImp()
  {
    return static_cast<derived_type&>(*this);
  }

  const derived_type& asImp() const
  {
    return static_cast<const derived_type&>(*this);
  }
}; // class BaseFunctionSetInterface


} // namespace GDT
} // namespace Dune

#endif // DUNE_GDT_BASEFUNCTIONSET_INTERFACE_HH

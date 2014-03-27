// This file is part of the dune-gdt project:
//   http://users.dune-project.org/projects/dune-gdt
// Copyright holders: Felix Albrecht
// License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)

#ifndef DUNE_GDT_PRODUCT_L2_HH
#define DUNE_GDT_PRODUCT_L2_HH

#include <type_traits>

#include <dune/stuff/functions/interfaces.hh>
#include <dune/stuff/functions/constant.hh>
#include <dune/stuff/common/crtp.hh>

#include "../localoperator/codim0.hh"
#include "../localevaluation/product.hh"

#include "base.hh"

namespace Dune {
namespace GDT {
namespace Product {


// forwards, to be used in the traits
template< class GridViewImp, class FieldImp >
class L2Base;


template< class GridViewImp, class RangeImp, class SourceImp = RangeImp, class FieldImp = double >
class L2Localizable;


template< class MatrixImp
        , class RangeSpaceImp
        , class GridViewImp = typename RangeSpaceImp::GridViewType
        , class SourceSpaceImp = RangeSpaceImp >
class L2Assemblable;


template< class GridViewImp, class FieldImp = double >
class L2Traits
{
public:
  typedef GridViewImp GridViewType;
protected:
  typedef Stuff::Function::Constant<  typename GridViewType::template Codim< 0 >::Entity,
                                      typename GridViewType::ctype,
                                      GridViewType::dimension, FieldImp, 1 >  FunctionType;
public:
  typedef LocalOperator::Codim0Integral< LocalEvaluation::Product< FunctionType > > LocalOperatorType;
private:
  friend class L2Base< GridViewImp, FieldImp >;
};


template< class GridViewImp, class FieldImp >
class L2Base
{
  typedef typename L2Traits< GridViewImp, FieldImp >::FunctionType       FunctionType;
  typedef typename L2Traits< GridViewImp, FieldImp >::LocalOperatorType  LocalOperatorType;

public:
  L2Base()
    : function_(1)
    , local_operator_(function_)
  {}

private:
  const FunctionType function_;
protected:
  const LocalOperatorType local_operator_;
}; // class L2Base


template< class GridViewImp, class RangeImp, class SourceImp, class FieldImp = double >
class L2LocalizableTraits
  : public L2Traits< GridViewImp, FieldImp >
{
public:
  typedef L2Localizable< GridViewImp, RangeImp, SourceImp, FieldImp > derived_type;
  typedef RangeImp    RangeType;
  typedef SourceImp   SourceType;
  typedef FieldImp    FieldType;
private:
  friend class L2Localizable< GridViewImp, RangeImp, SourceImp, FieldImp >;
};


template< class GridViewImp, class RangeImp, class SourceImp, class FieldImp >
class L2Localizable
  : public Product::LocalizableBase< L2LocalizableTraits< GridViewImp, RangeImp, SourceImp, FieldImp > >
  , public L2Base< GridViewImp, FieldImp >
{
  typedef Product::LocalizableBase< L2LocalizableTraits< GridViewImp, RangeImp, SourceImp, FieldImp > > BaseType;
public:
  typedef L2LocalizableTraits< GridViewImp, RangeImp, SourceImp, FieldImp > Traits;
  typedef typename Traits::GridViewType GridViewType;
  typedef typename Traits::RangeType    RangeType;
  typedef typename Traits::SourceType   SourceType;
private:
  typedef typename Traits::LocalOperatorType LocalOperatorType;

public:
  L2Localizable(const GridViewType& grid_view, const RangeType& range, const SourceType& source)
    : BaseType(grid_view, range, source)
  {}

  L2Localizable(const GridViewType& grid_view, const RangeType& range)
    : BaseType(grid_view, range, range)
  {}

  virtual const LocalOperatorType& local_operator() const DS_OVERRIDE DS_FINAL
  {
    return this->local_operator_;
  }
}; // class L2Localizable


template< class MatrixImp, class RangeSpaceImp, class GridViewImp, class SourceSpaceImp >
class L2AssemblableTraits
  : public L2Traits< GridViewImp, typename MatrixImp::ScalarType >
{
public:
  typedef L2Assemblable< MatrixImp, RangeSpaceImp, GridViewImp, SourceSpaceImp > derived_type;
  typedef RangeSpaceImp   RangeSpaceType;
  typedef SourceSpaceImp  SourceSpaceType;
  typedef MatrixImp       MatrixType;
private:
  friend class L2Assemblable< MatrixImp, RangeSpaceImp, GridViewImp, SourceSpaceImp >;
};


template< class MatrixImp, class RangeSpaceImp, class GridViewImp, class SourceSpaceImp >
class L2Assemblable
  : public Product::AssemblableBase< L2AssemblableTraits< MatrixImp, RangeSpaceImp, GridViewImp, SourceSpaceImp > >
  , public L2Base< GridViewImp, typename MatrixImp::ScalarType >
{
  typedef Product::AssemblableBase< L2AssemblableTraits< MatrixImp, RangeSpaceImp, GridViewImp, SourceSpaceImp > >
    BaseType;
public:
  typedef L2AssemblableTraits< MatrixImp, RangeSpaceImp, GridViewImp, SourceSpaceImp > Traits;
  typedef typename Traits::GridViewType     GridViewType;
  typedef typename Traits::RangeSpaceType   RangeSpaceType;
  typedef typename Traits::SourceSpaceType  SourceSpaceType;
  typedef typename Traits::MatrixType       MatrixType;
private:
  typedef typename Traits::LocalOperatorType LocalOperatorType;

public:
  using BaseType::pattern;

  static Stuff::LA::SparsityPatternDefault pattern(const RangeSpaceType& range_space,
                                                   const SourceSpaceType& source_space,
                                                   const GridViewType& grid_view)
  {
    return range_space.compute_volume_pattern(grid_view, source_space);
  }

  L2Assemblable(MatrixType& matrix,
                const RangeSpaceType& range_space,
                const GridViewType& grid_view,
                const SourceSpaceType& source_space)
    : BaseType(matrix, range_space, grid_view, source_space)
  {}

  L2Assemblable(MatrixType& matrix,
                const RangeSpaceType& range_space,
                const GridViewType& grid_view)
    : BaseType(matrix, range_space, grid_view, range_space)
  {}

  L2Assemblable(MatrixType& matrix,
                const RangeSpaceType& range_space)
    : BaseType(matrix, range_space, *(range_space.grid_view()), range_space)
  {}

private:
  virtual const LocalOperatorType& local_operator() const DS_OVERRIDE DS_FINAL
  {
    return this->local_operator_;
  }
}; // class L2Assemblable


template< class GridViewImp, class FieldImp = double >
class L2Generic;


template< class GridViewImp, class FieldImp = double >
class L2GenericTraits
{
public:
  typedef L2Generic< GridViewImp > derived_type;
  typedef GridViewImp GridViewType;
  typedef FieldImp    FieldType;
};


template< class GridViewImp, class FieldImp >
class L2Generic
  : public ProductInterface< L2GenericTraits< GridViewImp, FieldImp > >
{
public:
  typedef L2GenericTraits< GridViewImp, FieldImp > Traits;
  typedef typename Traits::GridViewType GridViewType;
  typedef typename Traits::FieldType    FieldType;

  typedef typename GridViewType::template Codim< 0 >::Entity  EntityType;
  typedef typename GridViewType::ctype                        DomainFieldType;
  static const unsigned int                                   dimDomain = GridViewType::dimension;

  L2Generic(const GridViewType& grid_view)
    : grid_view_(grid_view)
  {}

  const GridViewType& grid_view() const
  {
    return grid_view_;
  }

  template< class RR, int rRR, int rCR, class RS, int rRS, int rCS >
  FieldType apply2(const Stuff::LocalizableFunctionInterface< EntityType, DomainFieldType, dimDomain, RR, rRR, rCR >& /*range*/,
                   const Stuff::LocalizableFunctionInterface< EntityType, DomainFieldType, dimDomain, RS, rRS, rCS >& /*source*/) const
  {
    static_assert((Dune::AlwaysFalse< RR >::value), "Not implemented for this combination!");
  }

  template< int dimRangeRows, int dimRangeCols >
  FieldType apply2(const Stuff::LocalizableFunctionInterface
                      < EntityType, DomainFieldType, dimDomain, FieldType, dimRangeRows, dimRangeCols >& range,
                   const Stuff::LocalizableFunctionInterface
                      < EntityType, DomainFieldType, dimDomain, FieldType, dimRangeRows, dimRangeCols >& source) const
  {
    typedef Stuff::LocalizableFunctionInterface
        < EntityType, DomainFieldType, dimDomain, FieldType, dimRangeRows, dimRangeCols > FunctionType;
    L2Localizable< GridViewType, FunctionType, FunctionType, FieldType > product_operator(grid_view_, range, source);
    return product_operator.apply2();
  } // ... apply2(...)

private:
  const GridViewType& grid_view_;
}; // class L2Generic


} // namespace Product
} // namespace GDT
} // namespace Dune

#endif // DUNE_GDT_PRODUCT_L2_HH

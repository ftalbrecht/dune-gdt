// This file is part of the dune-gdt project:
//   http://users.dune-project.org/projects/dune-gdt
// Copyright holders: Felix Albrecht
// License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)

#ifndef DUNE_GDT_TEST_ELLIPTIC_HH
#define DUNE_GDT_TEST_ELLIPTIC_HH

#include <iostream>
#include <memory>

#include <dune/fem/gridpart/levelgridpart.hh>

#include <dune/stuff/grid/provider/cube.hh>
#include <dune/stuff/grid/boundaryinfo.hh>
#include <dune/stuff/functions/constant.hh>
#include <dune/stuff/functions/expression.hh>

namespace EllipticTestCase {


template< class GridType >
class Base
{
public:
  typedef typename Dune::Fem::LevelGridPart< GridType > GridPartType;
  typedef typename GridPartType::template Codim< 0 >::EntityType EntityType;
  typedef typename GridPartType::ctype DomainFieldType;
  static const unsigned int dimDomain = GridPartType::dimension;

  Base(std::shared_ptr< GridType > grd, size_t num_refinements)
    : grid_(grd)
  {
    levels_.push_back(grid_->maxLevel());
    level_grid_parts_.emplace_back(new GridPartType(*grid_, grid_->maxLevel()));
    for (size_t rr = 0; rr < num_refinements; ++rr) {
      grid_->globalRefine(GridType::refineStepsForHalf);
      levels_.push_back(grid_->maxLevel());
      level_grid_parts_.emplace_back(new GridPartType(*grid_, grid_->maxLevel()));
    }
    grid_->globalRefine(GridType::refineStepsForHalf);
    reference_grid_part_ = std::shared_ptr< GridPartType >(new GridPartType(*grid_, grid_->maxLevel()));
  }

  size_t num_levels() const
  {
    return levels_.size();
  }

  std::shared_ptr< const GridPartType > level_grid_part(const size_t level) const
  {
    assert(level < levels_.size());
    assert(level < level_grid_parts_.size());
    assert(ssize_t(levels_[level]) < grid_->maxLevel());
    return level_grid_parts_[level];
  }

  std::shared_ptr< const GridPartType > reference_grid_part() const
  {
    return reference_grid_part_;
  }

private:
  std::shared_ptr< GridType > grid_;
  std::vector< size_t > levels_;
  std::vector< std::shared_ptr< GridPartType > > level_grid_parts_;
  std::shared_ptr< GridPartType > reference_grid_part_;
}; // class Base


template< class GridType >
class ESV07
  : public Base< GridType >
{
  typedef Base< GridType > BaseType;
public:
  typedef typename BaseType::GridPartType GridPartType;
  typedef typename BaseType::EntityType   EntityType;
  typedef typename BaseType::DomainFieldType  DomainFieldType;
  static const unsigned int                   dimDomain = BaseType::dimDomain;
  typedef double            RangeFieldType;
  static const unsigned int dimRange = 1;
  typedef Dune::Stuff::GridboundaryAllDirichlet< typename GridPartType::IntersectionType > BoundaryInfoType;

  typedef Dune::Stuff::Function::Constant
      < EntityType, DomainFieldType, dimDomain, RangeFieldType, dimRange >
    ConstantFunctionType;
  typedef Dune::Stuff::Function::Expression
      < EntityType, DomainFieldType, dimDomain, RangeFieldType, dimRange >
    ExpressionFunctionType;
  typedef ConstantFunctionType    DiffusionType;
  typedef ExpressionFunctionType  ForceType;
  typedef ConstantFunctionType    DirichletType;
  typedef ConstantFunctionType    NeumannType;
  typedef ExpressionFunctionType  ExactSolutionType;

  ESV07(const size_t num_refinements = 3)
    : BaseType(create_initial_grid(), num_refinements)
    , boundary_info_()
    , diffusion_(1)
    , force_("x", "0.5 * pi * pi * cos(0.5 * pi * x[0]) * cos(0.5 * pi * x[1])", 3)
    , dirichlet_(0)
    , neumann_(0)
    , exact_solution_("x",
                      "cos(0.5 * pi * x[0]) * cos(0.5 * pi * x[1])",
                      3,
                      "exact solution",
                      {{"-0.5 * pi * sin(0.5 * pi * x[0]) * cos(0.5 * pi * x[1])",
                        "-0.5 * pi * cos(0.5 * pi * x[0]) * sin(0.5 * pi * x[1])"}})
  {}

  void print_header(std::ostream& out = std::cout) const
  {
    out << "+==================================================================+\n"
        << "|+================================================================+|\n"
        << "||  Testcase ESV07: smooth data, homogeneous dirichlet            ||\n"
        << "||  (see testcase 1, page 23 in Ern, Stephansen, Vohralik, 2007)  ||\n"
        << "|+----------------------------------------------------------------+|\n"
        << "||  domain = [-1, 1] x [-1 , 1]                                   ||\n"
        << "||  diffusion = 1                                                 ||\n"
        << "||  force     = 1/2 pi^2 cos(1/2 pi x) cos(1/2 pi y)              ||\n"
        << "||  dirichlet = 0                                                 ||\n"
        << "||  exact solution = cos(1/2 pi x) cos(1/2 pi y)                  ||\n"
        << "|+================================================================+|\n"
        << "+==================================================================+" << std::endl;
  } // ... print_header(...)

  const BoundaryInfoType& boundary_info() const
  {
    return boundary_info_;
  }

  const DiffusionType& diffusion() const
  {
    return diffusion_;
  }

  const ForceType& force() const
  {
    return force_;
  }

  const DirichletType& dirichlet() const
  {
    return dirichlet_;
  }

  const NeumannType& neumann() const
  {
    return neumann_;
  }

  bool provides_exact_solution() const
  {
    return false;
  }

  const ExactSolutionType& exact_solution() const
  {
    return exact_solution_;
  }

private:
  static std::shared_ptr< GridType > create_initial_grid()
  {
    typedef Dune::Stuff::GridProviderCube< GridType > GridProviderType;
    auto grid_provider = std::unique_ptr< GridProviderType >(new GridProviderType(-1, 1, 2));
    auto grid = grid_provider->grid();
    grid->globalRefine(1);
    return grid;
  } // ... create_initial_grid(...)

  const BoundaryInfoType boundary_info_;
  const DiffusionType diffusion_;
  const ForceType force_;
  const DirichletType dirichlet_;
  const NeumannType neumann_;
  const ExactSolutionType exact_solution_;
}; // class ESV07


} // namespace EllipticTestCase

#endif // DUNE_GDT_TEST_ELLIPTIC_HH

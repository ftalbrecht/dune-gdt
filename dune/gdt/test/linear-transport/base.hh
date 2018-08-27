// This file is part of the dune-gdt project:
//   https://github.com/dune-community/dune-gdt
// Copyright 2010-2018 dune-gdt developers and contributors. All rights reserved.
// License: Dual licensed as BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)
//      or  GPL-2.0+ (http://opensource.org/licenses/gpl-license)
//          with "runtime exception" (http://www.dune-project.org/license.html)
// Authors:
//   Felix Schindler (2018)

#ifndef DUNE_GDT_TEST_LINEAR_TRANSPORT_BASE_HH
#define DUNE_GDT_TEST_LINEAR_TRANSPORT_BASE_HH

#include <cmath>

#include <dune/xt/common/float_cmp.hh>
#include <dune/xt/common/fvector.hh>
#include <dune/xt/common/memory.hh>
#include <dune/xt/functions/lambda/function.hh>
#include <dune/xt/grid/gridprovider/cube.hh>

#include <dune/gdt/interpolations.hh>
#include <dune/gdt/spaces/interface.hh>
#include <dune/gdt/test/instationary-eocstudies/hyperbolic-nonconforming.hh>

namespace Dune {
namespace GDT {
namespace Test {


template <class G>
struct LinearTransportProblem
{
  static const constexpr size_t d = G::dimension;
  using DomainType = XT::Common::FieldVector<double, d>;

  const DomainType direction;
  const XT::Functions::LambdaFunction<1, d, 1> flux;
  const double T_end;

  LinearTransportProblem()
    : direction(XT::Common::from_string<DomainType>("[1 0 0]"))
    , flux(1,
           [&](const auto& u, const auto& /*param*/) { return direction * u; },
           "linear_transport",
           {},
           [&](const auto& /*u*/, const auto& /*param*/) { return direction; })
    , T_end(1.)
  {
  }

  XT::Grid::GridProvider<G> make_initial_grid() const
  {
    return XT::Grid::make_cube_grid<G>(0., 1., 16u);
  }

  template <class Vector, class GV>
  DiscreteFunction<Vector, GV> make_exact_solution__periodic_boundaries(const SpaceInterface<GV, 1>& space,
                                                                        const double& time) const
  {
    DUNE_THROW_IF(time > 1., XT::Common::Exceptions::wrong_input_given, "time = " << time);
    const auto indicator = [](const auto& x) {
      if (0.25 <= x && x <= 0.5)
        return 1.;
      else
        return 0.;
    };
    return interpolate<Vector>(
        0, [&](const auto& xx, const auto& /*mu*/) { return indicator(std::fmod(xx[0] - time + 1., 1.)); }, space);
  } // ... make_exact_solution__periodic_boundaries(...)
}; // struct LinearTransportProblem


template <class G>
class LinearTransportTest : XT::Common::ConstStorageProvider<LinearTransportProblem<G>>,
                            public InstationaryNonconformingHyperbolicEocStudy<G, 1>
{
  using Problem = XT::Common::ConstStorageProvider<LinearTransportProblem<G>>;
  using BaseType = InstationaryNonconformingHyperbolicEocStudy<G, 1>;

protected:
  using BaseType::d;
  using typename BaseType::F;
  using typename BaseType::DF;
  using typename BaseType::GP;
  using typename BaseType::S;
  using typename BaseType::BS;
  using typename BaseType::V;

public:
  LinearTransportTest(const size_t num_refinements = 3,
                      const size_t num_additional_refinements_for_reference = 1) // Not exact, but enough.
      : Problem(new LinearTransportProblem<G>()),
        BaseType(Problem::access().T_end,
                 num_refinements,
                 num_additional_refinements_for_reference,
                 [&](const auto& solution, const auto& prefix) {
                   for (size_t ii = 0; ii < this->visualization_steps_; ++ii) {
                     const double time = ii * (this->T_end_ / this->visualization_steps_);
                     solution.evaluate(time).visualize(prefix + "_solution_" + XT::Common::to_string(ii));
                   }
                 }),
        visualization_steps_(0)
  {
  }

  std::vector<std::string> targets() const override
  {
    if (d == 1) // in 1d dt depends linearly on h, so no need to pollute the EOC table with dt-related values
      return {"h"};
    else
      return BaseType::targets();
  }

  std::pair<double, double> estimate_dt(const S& space) override
  {
    if (d == 1 && this->space_type_ == "fv") { // here we know that dt = h is a good choice
      double grid_width = 0.;
      for (auto&& grid_element : elements(space.grid_view()))
        grid_width = std::max(grid_width, XT::Grid::entity_diameter(grid_element));
      return {grid_width, grid_width};
    } else
      return BaseType::estimate_dt(space);
  } // ... estimate_dt(...)

protected:
  const F& flux() const override
  {
    return Problem::access().flux;
  }

  DF make_initial_values(const S& space) override
  {
    return Problem::access().template make_exact_solution__periodic_boundaries<V>(space, 0.);
  }

  GP make_initial_grid() override
  {
    return Problem::access().make_initial_grid();
  }

  void compute_reference_solution() override
  {
    auto& self = *this;
    if (self.reference_solution_on_reference_grid_)
      return;
    self.reference_grid_ = std::make_unique<GP>(make_initial_grid());
    for (size_t ref = 0; ref < self.num_refinements_ + self.num_additional_refinements_for_reference_; ++ref)
      self.reference_grid_->global_refine(DGFGridInfo<G>::refineStepsForHalf());
    self.reference_space_ = self.make_space(*self.reference_grid_);
    const auto dt = estimate_dt(*self.reference_space_).second;
    self.reference_solution_on_reference_grid_ = std::make_unique<XT::LA::ListVectorArray<V>>(
        self.reference_space_->mapper().size(), /*length=*/0, /*reserve=*/std::ceil(self.T_end_ / dt));
    double time = 0.;
    while (time < self.T_end_ + dt) {
      auto u_t = Problem::access().template make_exact_solution__periodic_boundaries<V>(*self.reference_space_, time);
      self.reference_solution_on_reference_grid_->append(u_t.dofs().vector(), {"_t", time});
      time += dt;
    }
    // visualize
    const BS reference_bochner_space(*self.reference_space_,
                                     self.time_points_from_vector_array(*self.reference_solution_on_reference_grid_));
    self.visualize_(
        make_discrete_bochner_function(reference_bochner_space, *self.reference_solution_on_reference_grid_),
        "reference_solution_on_refinement_"
            + XT::Common::to_string(self.num_refinements_ + self.num_additional_refinements_for_reference_));
  } // ... compute_reference_solution(...)

protected:
  size_t visualization_steps_;
}; // class LinearTransportTest


template <class G>
class LinearTransportExplicitTest : public LinearTransportTest<G>
{
  using BaseType = LinearTransportTest<G>;
  using typename BaseType::S;
  using typename BaseType::V;

protected:
  XT::LA::ListVectorArray<V> solve(const S& space, const double T_end, const double dt) override final
  {
    const auto u_0 = this->make_initial_values(space);
    const auto op = this->make_lhs_operator(space);
    return solve_instationary_system_explicit_euler(u_0, *op, T_end, dt);
  }
}; // class LinearTransportExplicitTest


} // namespace Test
} // namespace GDT
} // namespace Dune

#endif // DUNE_GDT_TEST_LINEAR_TRANSPORT_BASE_HH

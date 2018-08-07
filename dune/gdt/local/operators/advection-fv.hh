// This file is part of the dune-gdt project:
//   https://github.com/dune-community/dune-gdt
// Copyright 2010-2018 dune-gdt developers and contributors. All rights reserved.
// License: Dual licensed as BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)
//      or  GPL-2.0+ (http://opensource.org/licenses/gpl-license)
//          with "runtime exception" (http://www.dune-project.org/license.html)
// Authors:
//   Felix Schindler (2018)

#ifndef DUNE_GDT_LOCAL_OPERATORS_ADVECTION_FV_HH
#define DUNE_GDT_LOCAL_OPERATORS_ADVECTION_FV_HH

#include <functional>

#include <dune/geometry/quadraturerules.hh>

#include <dune/xt/common/matrix.hh>
#include <dune/xt/common/memory.hh>
#include <dune/xt/la/eigen-solver.hh>
#include <dune/xt/functions/interfaces/function.hh>
#include <dune/xt/functions/constant.hh>
#include <dune/xt/functions/type_traits.hh>

#include <dune/gdt/exceptions.hh>
#include <dune/gdt/local/dof-vector.hh>
#include <dune/gdt/type_traits.hh>

#include "interfaces.hh"

namespace Dune {
namespace GDT {


/**
 * Given the sought solution of a system of m conservation laws, u: R^d -> R^m, and d flux functions f_s: R^m -> R^m,
 * for 1 <= s <= d (modelled by the flux f: R^m -> R^{d x m}), the purpose of a numerical flux
 * g: R^m x R^m x R^d -> R^m is to approximate f(.) * n, e.g., g(u, u, n) = f(u) * n.
 */
template <size_t d, size_t m = 1, class R = double>
class NumericalFluxInterface : public XT::Common::ParametricInterface
{
  using ThisType = NumericalFluxInterface<d, m, R>;

public:
  using FluxType = XT::Functions::FunctionInterface<m, d, m, R>;
  using PhysicalDomainType = FieldVector<double, d>;
  using StateRangeType = typename FluxType::DomainType;

  NumericalFluxInterface(const FluxType& flx, const XT::Common::ParameterType& param_type = {})
    : XT::Common::ParametricInterface(param_type + flx.parameter_type())
    , flux_(flx)
  {
  }

  NumericalFluxInterface(FluxType*&& flx_ptr, const XT::Common::ParameterType& param_type = {})
    : XT::Common::ParametricInterface(param_type + flx_ptr->parameter_type())
    , flux_(flx_ptr)
  {
  }

  virtual std::unique_ptr<ThisType> copy() const = 0;

  virtual bool linear() const
  {
    return false;
  }

  const FluxType& flux() const
  {
    return flux_.access();
  }

  virtual StateRangeType apply(const StateRangeType& u,
                               const StateRangeType& v,
                               const PhysicalDomainType& n,
                               const XT::Common::Parameter& param = {}) const = 0;

  template <class V>
  StateRangeType apply(const StateRangeType& u,
                       const XT::LA::VectorInterface<V>& v,
                       const PhysicalDomainType& n,
                       const XT::Common::Parameter& param = {}) const
  {
    DUNE_THROW_IF(v.size() != m, Exceptions::operator_error, "v.size() = " << v.size() << "\n   m = " << m);
    for (size_t ii = 0; ii < m; ++ii)
      v_[ii] = v[ii];
    return this->apply(u, v_, n, param);
  }

  template <class U>
  StateRangeType apply(const XT::LA::VectorInterface<U>& u,
                       const StateRangeType& v,
                       const PhysicalDomainType& n,
                       const XT::Common::Parameter& param = {}) const
  {
    DUNE_THROW_IF(u.size() != m, Exceptions::operator_error, "u.size() = " << u.size() << "\n   m = " << m);
    for (size_t ii = 0; ii < m; ++ii)
      u_[ii] = u[ii];
    return this->apply(u_, v, n, param);
  }

  template <class U, class V>
  StateRangeType apply(const XT::LA::VectorInterface<U>& u,
                       const XT::LA::VectorInterface<V>& v,
                       const PhysicalDomainType& n,
                       const XT::Common::Parameter& param = {}) const
  {
    DUNE_THROW_IF(u.size() != m, Exceptions::operator_error, "u.size() = " << u.size() << "\n   m = " << m);
    DUNE_THROW_IF(v.size() != m, Exceptions::operator_error, "v.size() = " << v.size() << "\n   m = " << m);
    for (size_t ii = 0; ii < m; ++ii) {
      u_[ii] = u[ii];
      v_[ii] = v[ii];
    }
    return this->apply(u_, v_, n, param);
  } // ... apply(...)

private:
  const XT::Common::ConstStorageProvider<FluxType> flux_;
  mutable StateRangeType u_;
  mutable StateRangeType v_;
}; // class NumericalFluxInterface


namespace internal {


template <size_t d, size_t m = 1, class R = double>
class ThisNumericalFluxIsNotAvailableForTheseDimensions : public NumericalFluxInterface<d, m, R>
{
  using ThisType = ThisNumericalFluxIsNotAvailableForTheseDimensions<d, m, R>;
  using BaseType = NumericalFluxInterface<d, m, R>;

public:
  using typename BaseType::StateRangeType;
  using typename BaseType::PhysicalDomainType;

  template <class... Args>
  explicit ThisNumericalFluxIsNotAvailableForTheseDimensions(Args&&... /*args*/)
    : BaseType(new XT::Functions::ConstantFunction<m, d, m, R>(0.))
  {
    DUNE_THROW(NotImplemented, "d = " << d << "\n   m = " << m);
  }

  std::unique_ptr<BaseType> copy() const override final
  {
    DUNE_THROW(NotImplemented, "d = " << d << "\n   m = " << m);
    return nullptr;
  }

  bool linear() const override final
  {
    DUNE_THROW(NotImplemented, "d = " << d << "\n   m = " << m);
    return false;
  }

  using BaseType::apply;

  StateRangeType apply(const StateRangeType& /*u*/,
                       const StateRangeType& /*v*/,
                       const PhysicalDomainType& /*n*/,
                       const XT::Common::Parameter& /*param*/ = {}) const override final
  {
    DUNE_THROW(NotImplemented, "d = " << d << "\n   m = " << m);
    return StateRangeType();
  }
}; // class ThisNumericalFluxIsNotAvailableForTheseDimensions


} // namespace internal


/**
 * \brief Implementation of NumericalFluxInterface for a given lambda expression.
 */
template <size_t d, size_t m = 1, class R = double>
class NumericalLambdaFlux : public NumericalFluxInterface<d, m, R>
{
  using ThisType = NumericalLambdaFlux<d, m, R>;
  using BaseType = NumericalFluxInterface<d, m, R>;

public:
  using typename BaseType::FluxType;
  using typename BaseType::PhysicalDomainType;
  using typename BaseType::StateRangeType;

  using LambdaType = std::function<StateRangeType(
      const StateRangeType&, const StateRangeType&, const PhysicalDomainType&, const XT::Common::Parameter&)>;

  NumericalLambdaFlux(const FluxType& flx, LambdaType lambda, const XT::Common::ParameterType& param_type = {})
    : BaseType(flx, param_type)
    , lambda_(lambda)
  {
  }

  std::unique_ptr<BaseType> copy() const override final
  {
    return std::make_unique<ThisType>(*this);
  }

  using BaseType::apply;

  StateRangeType apply(const StateRangeType& u,
                       const StateRangeType& v,
                       const PhysicalDomainType& n,
                       const XT::Common::Parameter& param = {}) const override final
  {
    return lambda_(u, v, n, this->parse_parameter(param));
  }

private:
  const LambdaType lambda_;
}; // class NumericalLambdaFlux


template <size_t d, size_t m, class R>
NumericalLambdaFlux<d, m, R> make_numerical_lambda_flux(const XT::Functions::FunctionInterface<m, d, m, R>& flux,
                                                        typename NumericalLambdaFlux<d, m, R>::LambdaType lambda,
                                                        const XT::Common::ParameterType& param_type = {})
{
  return NumericalLambdaFlux<d, m, R>(flux, lambda, param_type);
}


template <size_t d, size_t m = 1, class R = double>
class NumericalUpwindFlux : public internal::ThisNumericalFluxIsNotAvailableForTheseDimensions<d, m, R>
{
public:
  template <class... Args>
  explicit NumericalUpwindFlux(Args&&... /*args*/)
    : internal::ThisNumericalFluxIsNotAvailableForTheseDimensions<d, m, R>()
  {
  }
};

template <size_t d, class R>
class NumericalUpwindFlux<d, 1, R> : public NumericalFluxInterface<d, 1, R>
{
  static const constexpr size_t m = 1;
  using ThisType = NumericalUpwindFlux<d, m, R>;
  using BaseType = NumericalFluxInterface<d, m, R>;

public:
  using typename BaseType::FluxType;
  using typename BaseType::PhysicalDomainType;
  using typename BaseType::StateRangeType;

  NumericalUpwindFlux(const FluxType& flx)
    : BaseType(flx)
  {
  }

  NumericalUpwindFlux(const ThisType& other) = default;

  std::unique_ptr<BaseType> copy() const override final
  {
    return std::make_unique<ThisType>(*this);
  }

  using BaseType::apply;

  StateRangeType apply(const StateRangeType& u,
                       const StateRangeType& v,
                       const PhysicalDomainType& n,
                       const XT::Common::Parameter& param = {}) const override final
  {
    const auto df = this->flux().jacobian((u + v) / 2., param);
    if ((n * df) > 0)
      return this->flux().evaluate(u, param) * n;
    else
      return this->flux().evaluate(v, param) * n;
  }
}; // class NumericalUpwindFlux


template <size_t d, size_t m, class R>
NumericalUpwindFlux<d, m, R> make_numerical_upwind_flux(const XT::Functions::FunctionInterface<m, d, m, R>& flux)
{
  return NumericalUpwindFlux<d, m, R>(flux);
}


template <size_t d, size_t m = 1, class R = double>
class NumericalLaxFriedrichsFlux : public internal::ThisNumericalFluxIsNotAvailableForTheseDimensions<d, m, R>
{
public:
  template <class... Args>
  explicit NumericalLaxFriedrichsFlux(Args&&... /*args*/)
    : internal::ThisNumericalFluxIsNotAvailableForTheseDimensions<d, m, R>()
  {
  }
};

template <size_t d, class R>
class NumericalLaxFriedrichsFlux<d, 1, R> : public NumericalFluxInterface<d, 1, R>
{
  static const constexpr size_t m = 1;
  using ThisType = NumericalLaxFriedrichsFlux<d, m, R>;
  using BaseType = NumericalFluxInterface<d, m, R>;

public:
  using typename BaseType::FluxType;
  using typename BaseType::PhysicalDomainType;
  using typename BaseType::StateRangeType;

  NumericalLaxFriedrichsFlux(const FluxType& flx)
    : BaseType(flx)
  {
  }

  NumericalLaxFriedrichsFlux(const ThisType& other) = default;

  std::unique_ptr<BaseType> copy() const override final
  {
    return std::make_unique<ThisType>(*this);
  }

  using BaseType::apply;

  StateRangeType apply(const StateRangeType& u,
                       const StateRangeType& v,
                       const PhysicalDomainType& n,
                       const XT::Common::Parameter& param = {}) const override final
  {
    const auto lambda =
        1. / std::max(this->flux().jacobian(u, param).infinity_norm(), this->flux().jacobian(v, param).infinity_norm());
    return 0.5 * ((this->flux().evaluate(u, param) + this->flux().evaluate(v, param)) * n) + 0.5 * ((u - v) / lambda);
  }
}; // class NumericalLaxFriedrichsFlux


template <size_t d, size_t m, class R>
NumericalLaxFriedrichsFlux<d, m, R>
make_numerical_lax_friedrichs_flux(const XT::Functions::FunctionInterface<m, d, m, R>& flux)
{
  return NumericalLaxFriedrichsFlux<d, m, R>(flux);
}


template <size_t d, size_t m = 1, class R = double>
class NumericalEngquistOsherFlux : public internal::ThisNumericalFluxIsNotAvailableForTheseDimensions<d, m, R>
{
public:
  template <class... Args>
  explicit NumericalEngquistOsherFlux(Args&&... /*args*/)
    : internal::ThisNumericalFluxIsNotAvailableForTheseDimensions<d, m, R>()
  {
  }
};

template <size_t d, class R>
class NumericalEngquistOsherFlux<d, 1, R> : public NumericalFluxInterface<d, 1, R>
{
  static const constexpr size_t m = 1;
  using ThisType = NumericalEngquistOsherFlux<d, m, R>;
  using BaseType = NumericalFluxInterface<d, m, R>;

public:
  using typename BaseType::FluxType;
  using typename BaseType::PhysicalDomainType;
  using typename BaseType::StateRangeType;

  NumericalEngquistOsherFlux(const FluxType& flx)
    : BaseType(flx)
  {
  }

  NumericalEngquistOsherFlux(const ThisType& other) = default;

  std::unique_ptr<BaseType> copy() const override final
  {
    return std::make_unique<ThisType>(*this);
  }

  using BaseType::apply;

  StateRangeType apply(const StateRangeType& u,
                       const StateRangeType& v,
                       const PhysicalDomainType& n,
                       const XT::Common::Parameter& param = {}) const override final
  {
    auto integrate_f = [&](const auto& s, const std::function<double(const R&, const R&)>& min_max) {
      if (!(s[0] > 0.))
        return 0.;
      double ret = 0.;
      const OneDGrid state_grid(1, 0., s[0]);
      const auto state_interval = *state_grid.leafGridView().template begin<0>();
      for (const auto& quadrature_point :
           QuadratureRules<R, 1>::rule(state_interval.type(), this->flux().order(param))) {
        const auto local_uu = quadrature_point.position();
        const auto uu = state_interval.geometry().global(local_uu);
        const auto df = this->flux().jacobian(uu, param);
        ret += state_interval.geometry().integrationElement(local_uu) * quadrature_point.weight() * min_max(n * df, 0.);
      }
      return ret;
    };
    return (this->flux().evaluate(0., param) * n)
           + integrate_f(u, [](const double& a, const double& b) { return std::max(a, b); })
           + integrate_f(v, [](const double& a, const double& b) { return std::min(a, b); });
  }
}; // class NumericalEngquistOsherFlux


template <size_t d, size_t m, class R>
NumericalEngquistOsherFlux<d, m, R>
make_numerical_engquist_osher_flux(const XT::Functions::FunctionInterface<m, d, m, R>& flux)
{
  return NumericalEngquistOsherFlux<d, m, R>(flux);
}


template <size_t d, size_t m = 1, class R = double>
class NumericalVijayasundaramFlux : public NumericalFluxInterface<d, m, R>
{
  using ThisType = NumericalVijayasundaramFlux<d, m, R>;
  using BaseType = NumericalFluxInterface<d, m, R>;

public:
  using typename BaseType::FluxType;
  using typename BaseType::PhysicalDomainType;
  using typename BaseType::StateRangeType;

  using FluxEigenDecompositionLambdaType =
      std::function<std::tuple<std::vector<XT::Common::real_t<R>>,
                               XT::Common::FieldMatrix<XT::Common::real_t<R>, m, m>,
                               XT::Common::FieldMatrix<XT::Common::real_t<R>, m, m>>(const FieldVector<R, m>&,
                                                                                     const FieldVector<double, d>&)>;

  NumericalVijayasundaramFlux(const FluxType& flx)
    : BaseType(flx)
    , flux_eigen_decomposition_lambda_([&](const auto& w, const auto& n) {
      // evaluate flux jacobian, compute P matrix [DF2016, p. 404, (8.17)]
      const auto df = this->flux().jacobian(w);
      const auto P = df * n;
      auto eigensolver = XT::LA::make_eigen_solver(
          P, {{"type", XT::LA::eigen_solver_types(P)[0]}, {"assert_real_eigendecomposition", "1e-10"}});
      return std::make_tuple(
          eigensolver.real_eigenvalues(), eigensolver.real_eigenvectors(), eigensolver.real_eigenvectors_inverse());
    })
  {
  }

  NumericalVijayasundaramFlux(const FluxType& flx, FluxEigenDecompositionLambdaType flux_eigen_decomposition_lambda)
    : BaseType(flx)
    , flux_eigen_decomposition_lambda_(flux_eigen_decomposition_lambda)
  {
  }

  std::unique_ptr<BaseType> copy() const override final
  {
    return std::make_unique<ThisType>(*this);
  }

  using BaseType::apply;

  StateRangeType apply(const StateRangeType& u,
                       const StateRangeType& v,
                       const PhysicalDomainType& n,
                       const XT::Common::Parameter& /*param*/ = {}) const override final
  {
    // compute decomposition
    const auto eigendecomposition = flux_eigen_decomposition_lambda_(0.5 * (u + v), n);
    const auto& evs = std::get<0>(eigendecomposition);
    const auto& T = std::get<1>(eigendecomposition);
    const auto& T_inv = std::get<2>(eigendecomposition);
    // compute numerical flux [DF2016, p. 428, (8.108)]
    auto lambda_plus = XT::Common::zeros_like(T);
    auto lambda_minus = XT::Common::zeros_like(T);
    for (size_t ii = 0; ii < m; ++ii) {
      const auto& real_ev = evs[ii];
      XT::Common::set_matrix_entry(lambda_plus, ii, ii, XT::Common::max(real_ev, 0.));
      XT::Common::set_matrix_entry(lambda_minus, ii, ii, XT::Common::min(real_ev, 0.));
    }
    const auto P_plus = T * lambda_plus * T_inv;
    const auto P_minus = T * lambda_minus * T_inv;
    return P_plus * u + P_minus * v;
  } // ... apply(...)

private:
  FluxEigenDecompositionLambdaType flux_eigen_decomposition_lambda_;
}; // class NumericalVijayasundaramFlux


template <size_t d, size_t m, class R, class... Args>
NumericalVijayasundaramFlux<d, m, R>
make_numerical_vijayasundaram_flux(const XT::Functions::FunctionInterface<m, d, m, R>& flux, Args&&... args)
{
  return NumericalVijayasundaramFlux<d, m, R>(flux, std::forward<Args>(args)...);
}


/**
 * \note Presumes that the basis evaluates to 1.
 *
 * \note See also LocalIntersectionOperatorInterface for a description of the template arguments.
 *
 * \sa LocalIntersectionOperatorInterface
 */
template <class I,
          class SV,
          class SGV,
          size_t m = 1,
          class SR = double,
          class RR = SR,
          class IRGV = SGV,
          class IRV = SV,
          class ORR = RR,
          class ORGV = IRGV,
          class ORV = IRV>
class LocalAdvectionFvCouplingOperator
    : public LocalIntersectionOperatorInterface<I, SV, SGV, m, 1, SR, m, 1, RR, IRGV, IRV, ORGV, ORV>
{
  using ThisType = LocalAdvectionFvCouplingOperator<I, SV, SGV, m, SR, RR, IRGV, IRV, ORR, ORGV, ORV>;
  using BaseType = LocalIntersectionOperatorInterface<I, SV, SGV, m, 1, SR, m, 1, RR, IRGV, IRV, ORGV, ORV>;

public:
  using BaseType::d;
  using typename BaseType::IntersectionType;
  using typename BaseType::SourceType;
  using typename BaseType::LocalInsideRangeType;
  using typename BaseType::LocalOutsideRangeType;

  using NumericalFluxType = NumericalFluxInterface<d, m, RR>;

  LocalAdvectionFvCouplingOperator(const NumericalFluxType& numerical_flux)
    : BaseType(numerical_flux.parameter_type())
    , numerical_flux_(numerical_flux.copy())
  {
  }

  LocalAdvectionFvCouplingOperator(const ThisType& other)
    : BaseType(other.parameter_type())
    , numerical_flux_(other.numerical_flux_->copy())
  {
  }

  std::unique_ptr<BaseType> copy() const override final
  {
    return std::make_unique<ThisType>(*this);
  }

  void apply(const SourceType& source,
             const IntersectionType& intersection,
             LocalInsideRangeType& local_range_inside,
             LocalOutsideRangeType& local_range_outside,
             const XT::Common::Parameter& param = {}) const override final
  {
    DUNE_THROW_IF((source.space().type() != SpaceType::finite_volume)
                      || (local_range_inside.space().type() != SpaceType::finite_volume)
                      || (local_range_outside.space().type() != SpaceType::finite_volume),
                  Exceptions::operator_error,
                  "Use LocalAdvectionDgCouplingOperator instead!");
    const auto& inside_element = local_range_inside.element();
    const auto& outside_element = local_range_outside.element();
    const auto u = source.local_discrete_function(inside_element);
    const auto v = source.local_discrete_function(outside_element);
    const auto normal = intersection.centerUnitOuterNormal();
    const auto g = numerical_flux_->apply(u->dofs(), v->dofs(), normal, param);
    const auto h_intersection = intersection.geometry().volume();
    const auto h_inside_element = inside_element.geometry().volume();
    const auto h_outside_element = outside_element.geometry().volume();
    for (size_t ii = 0; ii < m; ++ii) {
      local_range_inside.dofs()[ii] += (g[ii] * h_intersection) / h_inside_element;
      local_range_outside.dofs()[ii] -= (g[ii] * h_intersection) / h_outside_element;
    }
  } // ... apply(...)

private:
  std::unique_ptr<NumericalFluxType> numerical_flux_;
}; // class LocalAdvectionFvCouplingOperator


template <class I, class SV, class SGV, size_t m = 1, class SF = double, class RF = SF, class RGV = SGV, class RV = SV>
class LocalAdvectionFvBoundaryTreatmentByCustomNumericalFluxOperator
    : public LocalIntersectionOperatorInterface<I, SV, SGV, m, 1, SF, m, 1, RF, RGV, RV>
{
  using ThisType = LocalAdvectionFvBoundaryTreatmentByCustomNumericalFluxOperator<I, SV, SGV, m, SF, RF, RGV, RV>;
  using BaseType = LocalIntersectionOperatorInterface<I, SV, SGV, m, 1, SF, m, 1, RF, RGV, RV>;

public:
  using BaseType::d;
  using typename BaseType::IntersectionType;
  using typename BaseType::SourceType;
  using typename BaseType::LocalInsideRangeType;
  using typename BaseType::LocalOutsideRangeType;

  using StateDomainType = FieldVector<typename SGV::ctype, SGV::dimension>;
  using StateDofsType = ConstLocalDofVector<SV, SGV>;
  using StateRangeType = typename XT::Functions::RangeTypeSelector<SF, m, 1>::type;
  using LambdaType = std::function<StateRangeType(
      const StateDofsType& /*u*/, const StateDomainType& /*n*/, const XT::Common::Parameter& /*param*/)>;

  LocalAdvectionFvBoundaryTreatmentByCustomNumericalFluxOperator(
      LambdaType numerical_boundary_flux_lambda, const XT::Common::ParameterType& boundary_treatment_param_type = {})
    : BaseType(boundary_treatment_param_type)
    , numerical_boundary_flux_(numerical_boundary_flux_lambda)
  {
  }

  LocalAdvectionFvBoundaryTreatmentByCustomNumericalFluxOperator(const ThisType& other)
    : BaseType(other.parameter_type())
    , numerical_boundary_flux_(other.numerical_boundary_flux_)
  {
  }

  std::unique_ptr<BaseType> copy() const override final
  {
    return std::make_unique<ThisType>(*this);
  }

  void apply(const SourceType& source,
             const IntersectionType& intersection,
             LocalInsideRangeType& local_range_inside,
             LocalOutsideRangeType& /*local_range_outside*/,
             const XT::Common::Parameter& param = {}) const override final
  {
    DUNE_THROW_IF((source.space().type() != SpaceType::finite_volume)
                      || (local_range_inside.space().type() != SpaceType::finite_volume),
                  Exceptions::operator_error,
                  "Use LocalAdvectionDgBoundaryOperatorByCustomNumericalFlux instead!");
    const auto& element = local_range_inside.element();
    const auto u = source.local_discrete_function(element);
    const auto normal = intersection.centerUnitOuterNormal();
    const auto g = numerical_boundary_flux_(u->dofs(), normal, param);
    const auto h_intersection = intersection.geometry().volume();
    const auto h_element = element.geometry().volume();
    for (size_t ii = 0; ii < m; ++ii)
      local_range_inside.dofs()[ii] += (g[ii] * h_intersection) / h_element;
  } // ... apply(...)

private:
  const LambdaType numerical_boundary_flux_;
}; // class LocalAdvectionFvBoundaryTreatmentByCustomNumericalFluxOperator


} // namespace GDT
} // namespace Dune

#endif // DUNE_GDT_LOCAL_OPERATORS_ADVECTION_FV_HH

// This file is part of the dune-gdt project:
//   https://github.com/dune-community/dune-gdt
// Copyright 2010-2017 dune-gdt developers and contributors. All rights reserved.
// License: Dual licensed as BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)
//      or  GPL-2.0+ (http://opensource.org/licenses/gpl-license)
//          with "runtime exception" (http://www.dune-project.org/license.html)
// Authors:
//   Felix Schindler (2017)

#ifndef DUNE_GDT_FUNCTIONALS_ELLIPTIC_IPDG_BINDINGS_HH
#define DUNE_GDT_FUNCTIONALS_ELLIPTIC_IPDG_BINDINGS_HH
#if HAVE_DUNE_PYBINDXI

#include <dune/pybindxi/pybind11.h>

#include "elliptic-ipdg.hh"
#include "base.bindings.hh"

namespace Dune {
namespace GDT {
namespace internal {


template <class DI, class DF, typename DT, LocalEllipticIpdgIntegrands::Method ipdg_method>
struct elliptic_ipdg_dirichlet_vector_functional_bind_helper
{
  static std::string suffix()
  {
    return "diffusion_factor_and_tensor";
  }

  template <class C>
  static void addbind_factory_methods(pybind11::module& m, const std::string& method_id, const std::string& la_id)
  {
    namespace py = pybind11;
    using namespace pybind11::literals;

    typedef typename C::SpaceType S;
    typedef typename C::VectorType V;

    m.def(std::string(method_id + "__" + la_id).c_str(),
          [](const DI& dirichlet,
             const DF& diffusion_factor,
             const DT& diffusion_tensor,
             const XT::Grid::BoundaryInfo<typename S::GridViewType::Intersection>& boundary_info,
             const S& space,
             const size_t over_integrate) {
            return make_elliptic_ipdg_dirichlet_vector_functional<V, ipdg_method>(
                       dirichlet, diffusion_factor, diffusion_tensor, boundary_info, space, over_integrate)
                .release(); //   <- b.c. EllipticIpdgDirichletVectorFunctional is not movable, returning the raw pointer
          }, //                                                                lets pybind11 correctly manage the memory
          "dirichlet"_a,
          "diffusion_factor"_a,
          "diffusion_tensor"_a,
          "boundary_info"_a,
          "space"_a,
          "over_integrate"_a = 0,
          py::keep_alive<0, 1>(),
          py::keep_alive<0, 2>(),
          py::keep_alive<0, 3>(),
          py::keep_alive<0, 4>(),
          py::keep_alive<0, 5>());

    m.def(std::string(method_id).c_str(),
          [](const DI& dirichlet,
             const DF& diffusion_factor,
             const DT& diffusion_tensor,
             const XT::Grid::BoundaryInfo<typename S::GridViewType::Intersection>& boundary_info,
             V& vector,
             const S& space,
             const size_t over_integrate) {
            return make_elliptic_ipdg_dirichlet_vector_functional<ipdg_method>(
                       dirichlet, diffusion_factor, diffusion_tensor, boundary_info, vector, space, over_integrate)
                .release(); //                                                                     <- s.a. for release()
          },
          "dirichlet"_a,
          "diffusion_factor"_a,
          "diffusion_tensor"_a,
          "boundary_info"_a,
          "vector"_a,
          "space"_a,
          "over_integrate"_a = 0,
          py::keep_alive<0, 1>(),
          py::keep_alive<0, 2>(),
          py::keep_alive<0, 3>(),
          py::keep_alive<0, 4>(),
          py::keep_alive<0, 5>(),
          py::keep_alive<0, 6>());
  } // ... addbind_factory_methods(...)
}; // struct elliptic_ipdg_dirichlet_vector_functional_bind_helper


template <class DI, class D, LocalEllipticIpdgIntegrands::Method ipdg_method>
struct elliptic_ipdg_dirichlet_vector_functional_bind_helper<DI, D, void, ipdg_method>
{
  static std::string suffix()
  {
    return "single_diffusion";
  }

  template <class C>
  static void addbind_factory_methods(pybind11::module& m, const std::string& method_id, const std::string& la_id)
  {
    namespace py = pybind11;
    using namespace pybind11::literals;

    typedef typename C::SpaceType S;
    typedef typename C::VectorType V;

    m.def(std::string(method_id + "__" + la_id).c_str(),
          [](const DI& dirichlet,
             const D& diffusion,
             const XT::Grid::BoundaryInfo<typename S::GridViewType::Intersection>& boundary_info,
             const S& space,
             const size_t over_integrate) {
            return make_elliptic_ipdg_dirichlet_vector_functional<V, ipdg_method>(
                       dirichlet, diffusion, boundary_info, space, over_integrate)
                .release(); //   <- b.c. EllipticIpdgDirichletVectorFunctional is not movable, returning the raw pointer
          }, //                                                                lets pybind11 correctly manage the memory
          "dirichlet"_a,
          "diffusion"_a,
          "boundary_info"_a,
          "space"_a,
          "over_integrate"_a = 0,
          py::keep_alive<0, 1>(),
          py::keep_alive<0, 2>(),
          py::keep_alive<0, 3>(),
          py::keep_alive<0, 4>());

    m.def(std::string(method_id).c_str(),
          [](const DI& dirichlet,
             const D& diffusion,
             const XT::Grid::BoundaryInfo<typename S::GridViewType::Intersection>& boundary_info,
             V& vector,
             const S& space,
             const size_t over_integrate) {
            return make_elliptic_ipdg_dirichlet_vector_functional<ipdg_method>(
                       dirichlet, diffusion, boundary_info, vector, space, over_integrate)
                .release(); //                                                                     <- s.a. for release()
          },
          "dirichlet"_a,
          "diffusion"_a,
          "boundary_info"_a,
          "vector"_a,
          "space"_a,
          "over_integrate"_a = 0,
          py::keep_alive<0, 1>(),
          py::keep_alive<0, 2>(),
          py::keep_alive<0, 3>(),
          py::keep_alive<0, 4>(),
          py::keep_alive<0, 5>());
  } // ... addbind_factory_methods(...)
}; // struct elliptic_ipdg_dirichlet_vector_functional_bind_helper


} // namespace internal

template <class DI,
          class DF,
          class DT, // <- may be void
          class S,
          LocalEllipticIpdgIntegrands::Method method = LocalEllipticIpdgIntegrands::default_method,
          class V = typename XT::LA::Container<typename S::RangeFieldType>::VectorType,
          class GV = typename S::GridViewType,
          class F = typename S::RangeFieldType>
pybind11::class_<EllipticIpdgDirichletVectorFunctional<DI, DF, DT, S, method, V, GV, F>>
bind_elliptic_ipdg_dirichlet_vector_functional(pybind11::module& m,
                                               const std::string& space_id,
                                               const std::string& la_id,
                                               const std::string& method_id)
{
  static_assert(std::is_same<GV, typename S::GridViewType>::value, "Not tested yet!");

  namespace py = pybind11;
  using namespace pybind11::literals;

  typedef EllipticIpdgDirichletVectorFunctional<DI, DF, DT, S, method, V, GV, F> C;
  const std::string suffix =
      la_id + "__" + space_id + "_"
      + internal::elliptic_ipdg_dirichlet_vector_functional_bind_helper<DI, DF, DT, method>::suffix();

  auto c = bind_vector_functional<C>(m, "Elliptic" + method_id + "DirichletVectorFunctional__" + suffix);

  internal::elliptic_ipdg_dirichlet_vector_functional_bind_helper<DI, DF, DT, method>::
      template addbind_factory_methods<C>(
          m, "make_elliptic_" + XT::Common::to_lower(method_id) + "_dirichlet_vector_functional", la_id);

  return c;
} // ... bind_elliptic_ipdg_dirichlet_vector_functional(...)


} // namespace GDT
} // namespace Dune

#endif // HAVE_DUNE_PYBINDXI
#endif // DUNE_GDT_FUNCTIONALS_ELLIPTIC_IPDG_BINDINGS_HH

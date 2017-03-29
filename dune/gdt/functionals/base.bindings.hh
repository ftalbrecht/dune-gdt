// This file is part of the dune-gdt project:
//   https://github.com/dune-community/dune-gdt
// Copyright 2010-2017 dune-gdt developers and contributors. All rights reserved.
// License: Dual licensed as BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)
//      or  GPL-2.0+ (http://opensource.org/licenses/gpl-license)
//          with "runtime exception" (http://www.dune-project.org/license.html)
// Authors:
//   Felix Schindler (2017)

#ifndef DUNE_GDT_FUNCTIONALS_BASE_PBH
#define DUNE_GDT_FUNCTIONALS_BASE_PBH
#if HAVE_DUNE_PYBINDXI

#include <dune/pybindxi/pybind11.h>

#include <dune/xt/la/container.hh>

#include "base.hh"

namespace Dune {
namespace GDT {


template <class FunctionalType>
pybind11::class_<FunctionalType,
                 SystemAssembler<typename FunctionalType::SpaceType, typename FunctionalType::GridViewType>>
bind_vector_functional(pybind11::module& m, const std::string& class_id)
{
  namespace py = pybind11;
  using namespace pybind11::literals;

  typedef FunctionalType C;
  typedef typename C::SpaceType S;
  typedef SystemAssembler<typename FunctionalType::SpaceType, typename FunctionalType::GridViewType> I;
  typedef typename C::VectorType V;

  py::class_<C, I> c(m, std::string(class_id).c_str(), std::string(class_id).c_str());

  c.def("vector", [](C& self) { return self.vector(); });
  c.def("space", [](C& self) { return self.space(); });
  c.def("apply", [](C& self, const V& source) { return self.apply(source); }, "source"_a);
  c.def("apply", [](C& self, const ConstDiscreteFunction<S, V>& source) { return self.apply(source); }, "source"_a);

  return c;
} // ... bind_vector_functional(...)


} // namespace GDT
} // namespace Dune

#endif // HAVE_DUNE_PYBINDXI
#endif // DUNE_GDT_FUNCTIONALS_BASE_PBH
#ifndef SOLVER_HPP_
#define SOLVER_HPP_

#include <vector>

#include "prox_lib.hpp"

// Data structure for input to Solver().
template <typename T, typename M>
struct AdmmData {
  // Input.
  std::vector<FunctionObj<T> > f, g;
  const M A;
  size_t m, n;

  // Output.
  T *x, *y;

  // Parameters.
  T rho;
  unsigned int max_iter;
  T rel_tol, abs_tol;
  bool quiet;

  // Constructor.
  AdmmData(const M &A, size_t m, size_t n)
      : A(A), m(m), n(n), rho(static_cast<T>(1)), max_iter(1000),
        rel_tol(static_cast<T>(1e-3)), abs_tol(static_cast<T>(1e-4)),
        quiet(false) { }
};

template <typename T, typename M>
void Solver(AdmmData<T, M> *admm_data);

#endif /* SOLVER_HPP_ */


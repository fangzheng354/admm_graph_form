#include <random>
#include <vector>

#include "solver.hpp"
#include "timer.hpp"

typedef double real_t;

// Non-Negative Least Squares.
//   minimize    (1/2) ||Ax - b||_2^2
//   subject to  x >= 0.
//
// See <admm_graph_form>/matlab/test_nonneg_l2.m for detailed description.
real_t test1() {
  printf("\nNon-Negative Least Squares.\n");
  size_t m = 1000;
  size_t n = 100;
  std::vector<real_t> A(m * n);
  std::vector<real_t> x(n);
  std::vector<real_t> y(m);

  std::default_random_engine generator;
  std::uniform_real_distribution<real_t> u_dist(static_cast<real_t>(0),
                                                static_cast<real_t>(1));
  std::normal_distribution<real_t> n_dist(static_cast<real_t>(0),
                                          static_cast<real_t>(1));

  // Generate A according to:
  //   A = 1 / n * rand(m, n)
  for (unsigned int i = 0; i < m * n; ++i)
    A[i] = static_cast<real_t>(1) / static_cast<real_t>(n) * u_dist(generator);

  AdmmData<real_t, real_t*> admm_data(A.data(), m, n);
  admm_data.x = x.data();
  admm_data.y = y.data();

  admm_data.f.reserve(m);
  for (unsigned int i = 0; i < m; ++i) {
    // Generate b according to:
    //   n_half = floor(2 * n / 3);
    //   b = A * [ones(n_half, 1); -ones(n - n_half, 1)] + 0.1 * randn(m, 1)
    real_t b_i = static_cast<real_t>(0);
    for (unsigned int j = 0; j < n; j++)
      b_i += 3 * j < 2 * n ? A[i * n + j] : -A[i * n + j];
    b_i += static_cast<real_t>(0.01) * n_dist(generator);
    admm_data.f.emplace_back(kSquare, static_cast<real_t>(1), b_i);
  }

  admm_data.g.reserve(n);
  for (unsigned int i = 0; i < n; ++i)
    admm_data.g.emplace_back(kIndGe0);

  Solver(&admm_data);

  return 0;
}


// Linear program in inequality form.
//   minimize    c^T * x
//   subject to  Ax <= b.
//
// See <admm_graph_form>/matlab/test_lp_ineq.m for detailed description.
real_t test2() {
  printf("\nLinear Program in Inequality Form.\n");
  size_t m = 1000;
  size_t n = 200;
  std::vector<real_t> A(m * n);
  std::vector<real_t> x(n);
  std::vector<real_t> y(m);

  std::default_random_engine generator;
  std::uniform_real_distribution<real_t> u_dist(static_cast<real_t>(0),
                                                static_cast<real_t>(1));

  // Generate A according to:
  //   A = [-1 / n *rand(m - n, n); -eye(n)]
  for (unsigned int i = 0; i < (m - n) * n; ++i)
    A[i] = -static_cast<real_t>(1) / static_cast<real_t>(n) * u_dist(generator);
  for (unsigned int i = static_cast<unsigned int>(n * n); i < m * n; ++i)
    A[i] = i % (n + 1) == 0 ? static_cast<real_t>(-1) : static_cast<real_t>(0);

  AdmmData<real_t, real_t*> admm_data(A.data(), m, n);
  admm_data.x = x.data();
  admm_data.y = y.data();

  // Generate b according to:
  //   b = A * rand(n, 1) + 0.2 * rand(m, 1)
  admm_data.f.reserve(m);
  for (unsigned int i = 0; i < m; ++i) {
    real_t b_i = static_cast<real_t>(0);
    for (unsigned int j = 0; j < n; ++j)
      b_i += A[i * n + j] * u_dist(generator);
    b_i += static_cast<real_t>(0.2) * u_dist(generator);
    admm_data.f.emplace_back(kIndLe0, static_cast<real_t>(1), b_i);
  }

  // Generate c according to:
  //   c = rand(n, 1)
  admm_data.g.reserve(n);
  for (unsigned int i = 0; i < n; ++i)
    admm_data.g.emplace_back(kIdentity, u_dist(generator));

  Solver(&admm_data);

  return 0;
}


// Linear program in equality form.
//   minimize    c^T * x
//   subject to  Ax = b
//               x >= 0.
//
// See <admm_graph_form>/matlab/test_lp_eq.m for detailed description.
real_t test3() {
  printf("\nLinear Program in Equality Form.\n");
  size_t m = 200;
  size_t n = 1000;
  std::vector<real_t> A((m + 1) * n);
  std::vector<real_t> x(n);
  std::vector<real_t> y(m + 1);

  std::default_random_engine generator;
  std::uniform_real_distribution<real_t> u_dist(static_cast<real_t>(0),
                                                static_cast<real_t>(1));

  // Generate A and c according to:
  //   A = rand(m, n)
  //   c = rand(n, 1)
  for (unsigned int i = 0; i < (m + 1) * n; ++i)
    A[i] = u_dist(generator);

  AdmmData<real_t, real_t*> admm_data(A.data(), m + 1, n);
  admm_data.x = x.data();
  admm_data.y = y.data();

  // Generate b according to:
  //   v = rand(n, 1)
  //   b = A * v
  std::vector<real_t> v(n);
  for (unsigned int i = 0; i < n; ++i)
    v[i] = u_dist(generator);

  admm_data.f.reserve(m + 1);
  for (unsigned int i = 0; i < m; ++i) {
    real_t b_i = static_cast<real_t>(0);
    for (unsigned int j = 0; j < n; ++j)
      b_i += A[i * n + j] * v[j];
    admm_data.f.emplace_back(kIndEq0, static_cast<real_t>(1), b_i);
  }
  admm_data.f.emplace_back(kIdentity);

  admm_data.g.reserve(n);
  for (unsigned int i = 0; i < n; ++i)
    admm_data.g.emplace_back(kIndGe0);

  Solver(&admm_data);

  return 0;
}


// Support Vector Machine.
//   minimize    (1/2) ||w||_2^2 + \lambda \sum (a_i^T * [w; b] + 1)_+.
//
// See <admm_graph_form>/matlab/test_svm.m for detailed description.
real_t test4() {
  printf("\nSupport Vector Machine.\n");
  size_t m = 1000;
  size_t n = 100;
  std::vector<real_t> A(m * (n + 1));
  std::vector<real_t> x(n + 1);
  std::vector<real_t> y(m);

  std::default_random_engine generator;
  std::uniform_real_distribution<real_t> u_dist(static_cast<real_t>(0),
                                                static_cast<real_t>(1));
  std::normal_distribution<real_t> n_dist(static_cast<real_t>(0),
                                          static_cast<real_t>(1));

  // Generate A according to:
  //   x = [randn(N, n) + ones(N, n); randn(N, n) - ones(N, n)]
  //   y = [ones(N, 1); -ones(N, 1)]
  //   A = [(-y * ones(1, n)) .* x, -y]
  for (unsigned int i = 0; i < m; ++i) {
    real_t sign_yi = i < m / 2 ? static_cast<real_t>(1) :
                                 static_cast<real_t>(-1);
    for (unsigned int j = 0; j < n; ++j) {
      A[i * (n + 1) + j] = -sign_yi * (n_dist(generator) + sign_yi);
    }
    A[i * (n + 1) + n] = -sign_yi;
  }

  AdmmData<real_t, real_t*> admm_data(A.data(), m, n + 1);
  admm_data.x = x.data();
  admm_data.y = y.data();

  real_t lambda = static_cast<real_t>(1);

  admm_data.f.reserve(m);
  for (unsigned int i = 0; i < m; ++i)
    admm_data.f.emplace_back(kMaxPos0, static_cast<real_t>(1),
                             static_cast<real_t>(-1), lambda);

  admm_data.g.reserve(n + 1);
  for (unsigned int i = 0; i < n; ++i)
    admm_data.g.emplace_back(kSquare);
  admm_data.g.emplace_back(kZero);

  Solver(&admm_data);
  return 0;
}

// Lasso
//   minimize    (1/2) ||Ax - b||_2^2 + \lambda ||x||_1
//
// See <admm_graph_form>/matlab/test_lasso.m for detailed description.
real_t test5(size_t m, size_t n) {
  printf("\nSupport Vector Machine.\n");
  std::vector<real_t> A(m * n);
  std::vector<real_t> b(m);
  std::vector<real_t> x(n);
  std::vector<real_t> y(m);

  std::default_random_engine generator;
  std::uniform_real_distribution<real_t> u_dist(static_cast<real_t>(0),
                                                static_cast<real_t>(1));
  std::normal_distribution<real_t> n_dist(static_cast<real_t>(0),
                                          static_cast<real_t>(1));

  for (unsigned int i = 0; i < m * n; ++i)
    A[i] = 1 / static_cast<real_t>(n) * n_dist(generator);

  std::vector<real_t> x_true(n);
  for (unsigned int i = 0; i < n; ++i)
    x_true[i] = u_dist(generator) < 0.8 ? 0 : n_dist(generator);

  for (unsigned int i = 0; i < m; ++i) {
    for (unsigned int j = 0; j < n; ++j)
      b[i] += A[i * n + j] * x_true[j];
    b[i] += 0.5 * n_dist(generator);
  }

  AdmmData<real_t, real_t*> admm_data(A.data(), m, n);
  admm_data.x = x.data();
  admm_data.y = y.data();

  real_t lambda = static_cast<real_t>(2e-2 + 5e-6 * static_cast<real_t>(m));

  admm_data.f.reserve(m);
  for (unsigned int i = 0; i < m; ++i)
    admm_data.f.emplace_back(kSquare, static_cast<real_t>(1), b[i]);

  admm_data.g.reserve(n);
  for (unsigned int i = 0; i < n; ++i)
    admm_data.g.emplace_back(kAbs, lambda);

  double t = timer();
  Solver(&admm_data);
  printf("%lu, %e\n", m, timer() - t);

  return 0;
}

int main() {
  // test1();
  // test2();
  // test3();
  // test4();
  size_t dim[] = {
      600, 743, 921, 1141, 1413, 1751, 2170, 2689, 3331, 4128, 5114,
      6337, 7851, 9728, 12053, 14933, 18502, 22924, 28403, 35191, 43602,
      54022, 66933, 82930, 102749, 127306, 157731, 195427, 242132, 299999 };
  for (unsigned int i = 0; i < 30; ++i)
    test5(dim[i], 500);
}


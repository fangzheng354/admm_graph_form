function results = test_lp_ineq(m, n, rho, quiet, save_mat)
%%TEST_LP_INEQ Test ADMM on an inequality constrained LP.
%   Compares ADMM to CVX when solving the problem
%
%     minimize    c^T * x
%     subject to  Ax <= b.
%
%   We transform this problem to
%
%     minimize    f(y) + g(x)
%     subject to  y = A * x,
%
%   where g_i(x_i) = c_i * x_i
%         f_i(y_i) = I(y_i <= b_i).
%
%   Test data are generated as follows
%     - The entries in c are chosen uniformly in the interval [0, 1].
%     - The first m-n entries in A are generated uniformly in
%       [-1/n, 0], this ensures that the problem will be feasible (since 
%       x -> \infty will always be a solution). In addition the last m
%       entries of A are set to the negative of the identity matrix. Since 
%       the the vector c is non-negative, this added constraint ensures 
%       that the problem is bounded. 
%     - To generate b, we first choose a vector v with entries drawn
%       uniformly from [0, 1], we assign b = A * v and add Gaussian
%       noise. The vector b is chosen this way, so that the solution
%       x^\star has reasonably uniform entries.
%
%   results = test_lp_ineq()
%   results = test_lp_ineq(m, n, rho, quiet, save_mat)
% 
%   Optional Inputs: (m, n), rho, quiet, save_mat
%
%   Optional Inputs:
%   (m, n)    - (default 1000, 200) Dimensions of the matrix A.
%   
%   rho       - (default 1.0) Penalty parameter to proximal operators.
% 
%   quiet     - (default false) Set flag to true, to disable output to
%               console.
%
%   save_mat  - (default false) Save data matrices to MatrixMarket files.
%
%   Outputs:
%   results   - Structure containg test results. Fields are:
%                 + rel_err_obj: Relative error of the objective, as
%                   compared to the solution obtained from CVX, defined as
%                   (admm_optval - cvx_optval) / abs(cvx_optval).
%                 + rel_err_soln: Relative difference in solution between
%                   CVX and ADMM, defined as 
%                   norm(x_admm - x_cvx) / norm(x_cvx).
%                 + max_violation: Maximum constraint violation (nan if 
%                   problem has no constraints).
%                 + avg_violation: Average constraint violation.
%                 + time_admm: Time required by ADMM to solve problem.
%                 + time_cvx: Time required by CVX to solve problem.
%

% Parse inputs.
if nargin < 2
  m = 1000;
  n = 200;
elseif m < n
  error('A must be a skinny matrix')
end
if nargin < 3
  rho = 1.0;
end
if nargin < 4
  quiet = false;
end
if nargin < 5
  save_mat = false;
end

% Initialize Data.
rng(0, 'twister')

A = -[4 / n * rand(m - n, n); eye(n)];
b = A * rand(n, 1) + 0.2 * rand(m, 1);
c = rand(n, 1);

% Export Matrices
if save_mat
  mmwrite('data/A_lp_ineq.dat', A, 'Matrix A for test_lp_ineq.m')
  mmwrite('data/b_lp_ineq.dat', b, 'Matrix b for test_lp_ineq.m')
  mmwrite('data/c_lp_ineq.dat', c, 'Matrix c for test_lp_ineq.m')
end

% Declare proximal operators.
g_prox = @(x, rho) x - c ./ rho;
f_prox = @(x, rho) min(b, x);
obj_fn = @(x, y) c' * x;

% Initialize ADMM input.
params.rho = rho;
params.quiet = quiet;
params.MAXITR = 1000;
params.RELTOL = 1e-3;

% Solve using ADMM.
tic
x_admm = admm(f_prox, g_prox, obj_fn, A, params);
time_admm = toc;

% Solve using CVX.
tic
cvx_begin quiet
  variable x_cvx(n)
  minimize(c' * x_cvx);
  subject to
    A * x_cvx <= b;
cvx_end
time_cvx = toc;

% Compute error metrics.
results.rel_err_obj = ...
    (obj_fn(x_admm, A * x_admm) - cvx_optval) / abs(cvx_optval);
results.rel_diff_soln = norm(x_admm - x_cvx) / norm(x_cvx);
results.max_violation = abs(min(min(b - A * x_admm), 0));
results.avg_violation = mean(abs(min(b - A * x_admm, 0)));
results.time_admm = time_admm;
results.time_cvx = time_cvx;

% Print error metrics
if ~quiet
  fprintf('\nRelative Error of Objective: %e\n', results.rel_err_obj)
  fprintf('Relative Difference in Solution: %e\n', results.rel_diff_soln)
  fprintf('Maximum Constraint Violation: %e\n', results.max_violation)
  fprintf('Average Constraint Violation: %e\n', results.avg_violation)
  fprintf('Time ADMM: %e\n', results.time_admm)
  fprintf('Time CVX: %e\n', results.time_cvx)
end

end

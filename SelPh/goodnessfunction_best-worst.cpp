// #define TIME

#include "goodnessfunction.h"

#include <iostream>
#ifdef TIME
#include <chrono>
#endif
#include <nlopt.hpp>
#include "eigenutility.h"
#include "interpolator.h"
#include "core.h"

using std::vector;
using namespace Eigen;

namespace {

struct Arg
{
    Arg(const GoodnessFunction* func, const VectorXd* feat) : functionPtr(func), featurePtr(feat) {}
    const GoodnessFunction* functionPtr;
    const VectorXd*         featurePtr;
};

double objectiveFunction(const vector<double>& x, vector<double>& grad, void* argStruct)
{
    const Arg*      arg = static_cast<Arg*>(argStruct);
    const VectorXd  p   = EigenUtility::std2eigen(x);
    const VectorXd& f   = *(arg->featurePtr);
    const VectorXd  g   = arg->functionPtr->computeGradient(p, f).block(0, 0, x.size(), 1);

    grad = EigenUtility::eigen2std(g);
    return arg->functionPtr->getValue(p, f);
}

}

VectorXd GoodnessFunction::getBestParameterSet(const VectorXd &f, bool inverse) const
{
    Arg arg(this, &f);

#ifdef TIME
    auto t1 = std::chrono::system_clock::now();
#endif

    assert (!getParameterList().empty());

    const unsigned dim = getParameterList()[0].rows();

    vector<double> x(dim, 0.5);
    double value;

    auto solve = [&](nlopt::opt& opt)
    {
        if (!inverse) {
            opt.set_max_objective(objectiveFunction, (void*) &arg);
        } else {
            opt.set_min_objective(objectiveFunction, (void*) &arg);
        }
        opt.set_lower_bounds(0.0);
        opt.set_upper_bounds(1.0);
        opt.set_maxtime(0.040);
        try
        {
            opt.optimize(x, value);
        }
        catch (nlopt::roundoff_limited e)
        {
            std::cerr << e.what() << std::endl;
        }
        catch (std::runtime_error e)
        {
            std::cerr << e.what() << std::endl;
        }
    };

    if (inverse)
    {
        // Compute global optimization
        nlopt::opt gOpt(nlopt::GD_MLSL_LDS, dim);
        solve(gOpt);
    }

    // Compute local optimization
    nlopt::opt lOpt(nlopt::LD_LBFGS, dim);
    solve(lOpt);

#ifdef TIME
    auto t2 = std::chrono::system_clock::now();
    auto type = inverse ? string("worst") : string("best");
    cout << "time: " << chrono::duration_cast<chrono::milliseconds>(t2 - t1).count() << ", type: " << type << endl;
#endif

    return EigenUtility::std2eigen(x);
}

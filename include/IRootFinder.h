#pragma once
/**
 * @file IRootFinder.h
 * @brief Strategy interface for scalar root-finding algorithms.
 *
 * Open/Closed: Brent's method is the default, but any bracketed solver
 * (bisection, Illinois, Ridders…) can be dropped in.
 *
 * Interface Segregation: only Solve() is required.
 */
#include <functional>
#include <optional>

using ScalarFn = std::function<double(double)>;

/**
 * @brief Root finder contract.
 */
class IRootFinder
{
public:
    virtual ~IRootFinder() = default;

    /**
     * @brief Find x in [lower, upper] such that f(x) ≈ 0.
     * @return The root, or std::nullopt if the method failed.
     */
    virtual std::optional<double> Solve(ScalarFn f,
                                        double lower,
                                        double upper) const = 0;
};

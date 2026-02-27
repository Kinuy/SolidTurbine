/**
 * @file TurbSimManager.cpp
 * @brief Implementation of the refactored TurbSimManager.
 *
 * This file contains only orchestration.  File parsing lives in
 * BladedBinaryReader; bilinear interpolation lives in
 * BilinearTurbSimInterpolator.  This class wires them together.
 */
#include "TurbSimManager.h"

#include <memory>
#include <stdexcept>

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────
TurbSimManager::TurbSimManager(ITurbSimReader const *reader,
                               ITurbSimInterpolator const *interpolator)
    : reader_(reader), interpolator_(interpolator)
{
    if (!reader_)
        throw std::invalid_argument("TurbSimManager: reader must be non-null");
    if (!interpolator_)
        throw std::invalid_argument("TurbSimManager: interpolator must be non-null");
}

// ─────────────────────────────────────────────────────────────────────────────
// Load — delegate entirely to the injected reader
// ─────────────────────────────────────────────────────────────────────────────
void TurbSimManager::Load(std::string const &filename)
{
    TurbSimFileData data = reader_->Read(filename);

    grid_ = std::move(data.grid);
    velocity_ = std::move(data.velocity);
    hub_velocity_ = data.hub_velocity;
    timing_store_ = std::make_unique<TurbSimTimingInfo>(std::move(data.timing));
    timing_ = timing_store_.get();
    loaded_ = true;
}

// ─────────────────────────────────────────────────────────────────────────────
// VelocityAt — translate user iteration → raw index, then delegate to
//              the injected interpolator
// ─────────────────────────────────────────────────────────────────────────────
WVPMUtilities::Vec3D<double>
TurbSimManager::VelocityAt(WVPMUtilities::Vec3D<double> const &point,
                           unsigned iteration) const
{
    AssertLoaded();
    std::size_t raw_ts = timing_->RawIndex(iteration);
    return interpolator_->Interpolate(point, raw_ts, grid_, velocity_);
}

// ─────────────────────────────────────────────────────────────────────────────
// Accessors
// ─────────────────────────────────────────────────────────────────────────────
double TurbSimManager::hub_velocity() const
{
    AssertLoaded();
    return hub_velocity_;
}

double TurbSimManager::timestep() const
{
    AssertLoaded();
    return timing_->timestep();
}

unsigned TurbSimManager::requested_num_iterations() const
{
    AssertLoaded();
    return timing_->usable_iterations();
}

TurbSimGrid TurbSimManager::grid() const
{
    AssertLoaded();
    return grid_;
}

// ─────────────────────────────────────────────────────────────────────────────
// velocity_data_not_padded_as_vectors — legacy diagnostic API
// ─────────────────────────────────────────────────────────────────────────────
std::tuple<std::vector<double>, std::vector<double>, std::vector<double>>
TurbSimManager::velocity_data_not_padded_as_vectors(unsigned iteration) const
{
    AssertLoaded();
    std::size_t raw_ts = timing_->RawIndex(iteration);
    VelocityTimestep const &ts = velocity_.Timestep(raw_ts);

    std::vector<double> u(ts.size()), v(ts.size()), w(ts.size());
    for (std::size_t i = 0; i < ts.size(); ++i)
    {
        u[i] = ts[i].x();
        v[i] = ts[i].y();
        w[i] = ts[i].z();
    }
    return {u, v, w};
}

// ─────────────────────────────────────────────────────────────────────────────
// AssertLoaded — guard for all methods that require data to be loaded
// ─────────────────────────────────────────────────────────────────────────────
void TurbSimManager::AssertLoaded() const
{
    if (!loaded_)
        throw std::logic_error("TurbSimManager: Load() must be called before use");
}

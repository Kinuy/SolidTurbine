/**
 * @file FlowCalculator.cpp
 * @brief Implementation of the refactored FlowCalculator.
 *
 * This file contains only orchestration logic.  No shear formula, veer
 * formula, or rotation matrix is computed here — all are delegated to the
 * injected strategy objects (DIP, SRP).
 */
#define _USE_MATH_DEFINES
#include "FlowCalculator.h"

#include <cmath>
#include <float.h>
#include <stdexcept>

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────
FlowCalculator::FlowCalculator(TurbineGeometry const *geometry,
                               double rot_rate,
                               double psi,
                               IInletVelocityProvider const *inlet,
                               IShearModel const *shear,
                               IVeerModel const *veer,
                               ICoordinateTransformer const *transformer)
    : geometry_(geometry), inlet_(inlet), shear_(shear), veer_(veer), transformer_(transformer), rot_rate_(rot_rate), psi_(psi), v_inf_(inlet ? inlet->HubVelocity() : 0.0), global_vels_(geometry ? geometry->num_sections()
                                                                                                                                                                                                   : throw std::invalid_argument(
                                                                                                                                                                                                         "FlowCalculator: geometry must be non-null")),
      local_vels_(geometry->num_sections())
{
    if (!inlet_)
        throw std::invalid_argument("FlowCalculator: inlet must be non-null");
    if (!shear_)
        throw std::invalid_argument("FlowCalculator: shear must be non-null");
    if (!veer_)
        throw std::invalid_argument("FlowCalculator: veer must be non-null");
    if (!transformer_)
        throw std::invalid_argument("FlowCalculator: transformer must be non-null");

    // Compute the full velocity field immediately on construction so all
    // accessors are valid from the first call.
    BuildInletField();
    ApplyShear();
    ApplyVeer();
    BuildLocalField();
}

// ─────────────────────────────────────────────────────────────────────────────
// Step 1: Populate global_vels_ from the inlet provider
// ─────────────────────────────────────────────────────────────────────────────
void FlowCalculator::BuildInletField()
{
    std::vector<WVPMUtilities::Vec3D<double>> global_positions =
        geometry_->GlobalPositionsAtPsi(psi_);

    for (std::size_t i = 0; i < geometry_->num_sections(); ++i)
    {
        global_vels_[i] = inlet_->VelocityAt(global_positions[i], i);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Step 2: Apply shear to axial component of each section's global velocity
// ─────────────────────────────────────────────────────────────────────────────
void FlowCalculator::ApplyShear()
{
    std::vector<WVPMUtilities::Vec3D<double>> global_positions =
        geometry_->GlobalPositionsAtPsi(psi_);

    ShearInput si;
    si.hub_height = geometry_->hub_height();
    si.v_hub = v_inf_;

    for (std::size_t i = 0; i < geometry_->num_sections(); ++i)
    {
        si.height = global_positions[i].z();
        global_vels_[i][0] = shear_->VelocityAt(si);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Step 3: Apply veer rotation to each section's global velocity vector
// ─────────────────────────────────────────────────────────────────────────────
void FlowCalculator::ApplyVeer()
{
    std::vector<WVPMUtilities::Vec3D<double>> global_positions =
        geometry_->GlobalPositionsAtPsi(psi_);

    VeerInput vi;
    vi.hub_height = geometry_->hub_height();

    for (std::size_t i = 0; i < geometry_->num_sections(); ++i)
    {
        vi.height = global_positions[i].z();
        global_vels_[i] = veer_->Apply(global_vels_[i], vi);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Step 4: Convert to local frame and add rotational contribution
// ─────────────────────────────────────────────────────────────────────────────
void FlowCalculator::BuildLocalField()
{
    std::vector<double> rot_vels = RotationalVelocities();

    for (std::size_t i = 0; i < geometry_->num_sections(); ++i)
    {
        local_vels_[i] = transformer_->ToLocal(global_vels_[i]);
        local_vels_[i][1] += rot_vels[i]; // tangential rotation component
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Rotational velocity per section: Ω × r_perp
//
// Uses the hub-relative position at psi=0 because the rotational velocity
// depends only on the distance from the rotation axis, not the current psi.
// The perpendicular distance is the z-component in hub-relative coordinates
// (same convention as the original code).
// ─────────────────────────────────────────────────────────────────────────────
std::vector<double> FlowCalculator::RotationalVelocities() const
{
    std::vector<WVPMUtilities::Vec3D<double>> axis_rel =
        geometry_->HubRelativePositionsAtPsi(0.0);

    std::vector<double> rot_vels;
    rot_vels.reserve(axis_rel.size());

    for (auto const &pos : axis_rel)
    {
        rot_vels.push_back(rot_rate_ * pos.z());
    }
    return rot_vels;
}

// ─────────────────────────────────────────────────────────────────────────────
// Public accessors
// ─────────────────────────────────────────────────────────────────────────────
double FlowCalculator::LocalLambda(std::size_t section) const
{
    double lam = local_vels_[section].y() / local_vels_[section].x();
    if (lam > -DBL_EPSILON && lam < DBL_EPSILON)
    {
        return DBL_EPSILON;
    }
    return lam;
}

void FlowCalculator::BladeLocalVelocities(std::size_t section,
                                          double *axial_vel,
                                          double *tangt_vel) const
{
    *axial_vel = local_vels_[section].x();
    *tangt_vel = local_vels_[section].y();
}

double FlowCalculator::Lambda() const
{
    double v_tip = rot_rate_ * geometry_->RotorRadius();
    return (v_inf_ > 0.0) ? v_tip / v_inf_ : 0.0;
}

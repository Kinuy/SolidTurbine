#pragma once
/**
 * @file PsiCoordinateTransformer.h
 * @brief Global-to-local coordinate transformer based on azimuth angle (psi).
 *
 * Single Responsibility: constructs the psi rotation matrix once on
 * construction and applies it in ToLocal().  No aerodynamic knowledge here.
 */
#include "ICoordinateTransformer.h"
#include "TurbineGeometry.h" // TurbineGeometry::CreateMatrix14()
#include "MathUtilities.h"   // WVPMUtilities::RotateVec3D

class PsiCoordinateTransformer final : public ICoordinateTransformer
{
public:
    /**
     * @param geometry  Non-owning pointer (must outlive this object).
     * @param psi       Rotor azimuth angle [rad].
     */
    PsiCoordinateTransformer(TurbineGeometry const *geometry, double psi)
        : matrix_(geometry->CreateMatrix14(psi))
    {
    }

    WVPMUtilities::Vec3D<double> ToLocal(
        WVPMUtilities::Vec3D<double> const &global_vel) const override
    {
        WVPMUtilities::Vec3D<double> result = global_vel;
        WVPMUtilities::RotateVec3D(matrix_, &result);
        return result;
    }

private:
    WVPMUtilities::SquareMatrix<double> matrix_;
};

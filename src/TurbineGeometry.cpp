#include "TurbineGeometry.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <array>

TurbineGeometry::TurbineGeometry(std::unique_ptr<BladeInterpolator> blade)
    : blade_(std::move(blade))
{

    num_sections_ = blade_->getBladeSections().size();

    radii_.reserve(num_sections_);
    for (int i = 1; i < num_sections_; ++i)
    {
        radii_.push_back(blade_->getBladeSections().at(i)->bladeRadius);
    }
}

// ---------------------------------------------------------------------------
// Rotation matrix pre-computation
// ---------------------------------------------------------------------------

void TurbineGeometry::PreComputeRotationMatrices()
{
    a1_ = MakeYawMatrix();
    a2_ = MakeTiltMatrix();
    a12_ = WVPMUtilities::MatMult(a2_, a1_); // world -> hub shaft
    a34_ = MakeConeMatrix();

    a21_ = Transposed(a12_); // hub shaft -> world
    a43_ = Transposed(a34_); // blade root -> hub shaft
}

// ---------------------------------------------------------------------------
// Position queries
// ---------------------------------------------------------------------------

std::vector<WVPMUtilities::Vec3D<double>>
TurbineGeometry::GlobalPositionsAtPsi(double psi) const
{
    using Vec3D = WVPMUtilities::Vec3D<double>;

    // Tower-offset vector rotated into world frame
    const std::vector<Vec3D> rs = ComputeTowerOffsets();

    // Blade radii rotated into world frame via a41
    const auto a41 = WVPMUtilities::MatMult(a21_, CreateMatrix42(psi));
    const std::vector<Vec3D> rb_glob = RotateRadiiByMatrix(a41);

    // Combine: hub_height offset + tower offset + blade position

    std::vector<Vec3D> xyz;
    xyz.reserve(num_sections_);
    for (int r = 0; r < num_sections_; ++r)
    {
        Vec3D p(0.0, 0.0, hub_height_);
        p += rs[r];
        p += rb_glob[r];
        xyz.push_back(p);
    }
    return xyz;
}

std::vector<WVPMUtilities::Vec3D<double>>
TurbineGeometry::HubRelativePositionsAtPsi(double psi) const
{
    return RotateRadiiByMatrix(CreateMatrix42(psi));
}

// ---------------------------------------------------------------------------
// Coordinate transformation matrices
// ---------------------------------------------------------------------------

WVPMUtilities::SquareMatrix<double>
TurbineGeometry::CreateMatrix23(double psi) const
{
    WVPMUtilities::SquareMatrix<double> a23(3);
    const std::array<double, 9> data = {
        1.0, 0.0, 0.0,
        0.0, cos(psi), sin(psi),
        0.0, -sin(psi), cos(psi)};
    for (std::size_t i = 0; i < data.size(); ++i)
    {
        a23[i] = data[i];
    }
    return a23;
}

WVPMUtilities::SquareMatrix<double>
TurbineGeometry::CreateMatrix42(double psi) const
{
    auto a32 = CreateMatrix23(psi);
    a32.Transpose();
    return WVPMUtilities::MatMult(a32, a43_);
}

WVPMUtilities::SquareMatrix<double>
TurbineGeometry::CreateMatrix13(double psi) const
{
    return WVPMUtilities::MatMult(CreateMatrix23(psi), a12_);
}

WVPMUtilities::SquareMatrix<double>
TurbineGeometry::CreateMatrix14(double psi) const
{
    return WVPMUtilities::MatMult(a34_, CreateMatrix13(psi));
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

WVPMUtilities::SquareMatrix<double>
TurbineGeometry::MakeYawMatrix() const
{
    const double cy = cos(yaw_);
    const double sy = sin(yaw_);
    WVPMUtilities::SquareMatrix<double> m(3);
    const std::array<double, 9> data = {
        cy, sy, 0.0,
        -sy, cy, 0.0,
        0.0, 0.0, 1.0};
    for (std::size_t i = 0; i < data.size(); ++i)
    {
        m[i] = data[i];
    }
    return m;
}

WVPMUtilities::SquareMatrix<double>
TurbineGeometry::MakeTiltMatrix() const
{
    const double ct = cos(tilt_);
    const double st = sin(tilt_);
    WVPMUtilities::SquareMatrix<double> m(3);
    const std::array<double, 9> data = {
        ct, 0.0, -st,
        0.0, 1.0, 0.0,
        st, 0.0, ct};
    for (std::size_t i = 0; i < data.size(); ++i)
    {
        m[i] = data[i];
    }
    return m;
}

WVPMUtilities::SquareMatrix<double>
TurbineGeometry::MakeConeMatrix() const
{
    const double cc = cos(cone_);
    const double sc = sin(cone_);
    WVPMUtilities::SquareMatrix<double> m(3);
    const std::array<double, 9> data = {
        cc, 0.0, -sc,
        0.0, 1.0, 0.0,
        sc, 0.0, cc};
    for (std::size_t i = 0; i < data.size(); ++i)
    {
        m[i] = data[i];
    }
    return m;
}

WVPMUtilities::SquareMatrix<double>
TurbineGeometry::Transposed(WVPMUtilities::SquareMatrix<double> m) const
{
    m.Transpose();
    return m;
}

std::vector<WVPMUtilities::Vec3D<double>>
TurbineGeometry::ComputeTowerOffsets() const
{
    std::vector<WVPMUtilities::Vec3D<double>> offsets;
    offsets.reserve(num_sections_);
    for (int r = 0; r < num_sections_; ++r)
    {
        WVPMUtilities::Vec3D<double> v(-tower_dist_, 0.0, 0.0);
        WVPMUtilities::RotateVec3D(a21_, &v);
        offsets.push_back(v);
    }
    return offsets;
}

std::vector<WVPMUtilities::Vec3D<double>>
TurbineGeometry::RotateRadiiByMatrix(
    WVPMUtilities::SquareMatrix<double> const &mat) const
{
    std::vector<WVPMUtilities::Vec3D<double>> points;
    points.reserve(radii_.size());
    for (const double r : radii_)
    {
        WVPMUtilities::Vec3D<double> v(0.0, 0.0, r);
        WVPMUtilities::RotateVec3D(mat, &v);
        points.push_back(v);
    }
    return points;
}

void TurbineGeometry::InterpForCoeff(std::size_t sec, double Re, double Ma, double alpha,
                                     double *Cl, double *Cd, double *Cm) const
{
    AirfoilOperationCondition aoc(Re, Ma, alpha);

    // TODO
    AirfoilAeroCoefficients coeffs = blade_->getBladeSections().at(sec)->airfoilPolar->findOrInterpolateCoefficients(aoc); // polar->InterpForCoeff(Re, Ma, alpha, Cl, Cd, Cm);
    *Cl = coeffs.cl;
    *Cd = coeffs.cd;
    *Cm = coeffs.cm;
}

#ifndef CC_TURBINEGEOMETRY_H_
#define CC_TURBINEGEOMETRY_H_

#include <string>
#include <vector>
#include <numbers>
#include "MathUtilities.h"
#include "BladeInterpolator.h"
// #include "Angles.h"

/// @file turbinegeometry.h
/// @brief Defines the TurbineGeometry class, which holds all geometric
///        parameters of a wind turbine rotor and provides coordinate
///        transformations between reference frames.

// ---------------------------------------------------------------------------
/// @brief Holds all geometric parameters of a wind turbine rotor and provides
///        coordinate transformations between reference frames.
///
/// ## Reference frames
/// The class maintains six pre-computed rotation matrices that form a chain
/// of coordinate transformations:
///
/// | Matrix  | Transforms from → to                       |
/// |---------|---------------------------------------------|
/// | a1_     | World → yawed nacelle frame                 |
/// | a2_     | Yawed → tilted shaft frame                  |
/// | a12_    | World → hub-shaft frame  (= a2 × a1)        |
/// | a21_    | Hub-shaft → world  (= transpose of a12)     |
/// | a34_    | Hub-shaft → blade-root frame (cone)         |
/// | a43_    | Blade-root → hub-shaft  (= transpose of a34)|
///
/// Matrices that depend on the instantaneous blade azimuth angle ψ
/// (a23, a42, a13, a14) are computed on demand rather than cached.
///
/// ## Typical usage
/// @code
/// TurbineGeometry geom(chords, radii, ...);
/// geom.SetMacroProperties(cone, yaw, tilt, tower_dist, hub_height);
/// geom.PreComputeRotationMatrices();        // must be called before position queries
/// auto pts = geom.GlobalPositionsAtPsi(psi);
/// @endcode
// ---------------------------------------------------------------------------
class TurbineGeometry
{
public:
    // -----------------------------------------------------------------------
    /// @brief Constructs a TurbineGeometry from per-section blade data.
    ///
    /// All vectors must have the same length (equal to the number of
    /// aerodynamic sections). Data is moved into the object; the caller's
    /// vectors are left in a valid but unspecified state.
    ///
    /// Macro properties (yaw, tilt, cone, hub height, tower distance) default
    /// to zero / 100 m and must be set via @ref SetMacroProperties before use.
    ///
    /// @param chords         Chord lengths [m] at each section.
    /// @param radii          Radial positions [m] of each section measured
    ///                       from the rotor centre.  The last element is the
    ///                       rotor tip radius.
    /// @param thicknesses    Absolute section thicknesses [m].
    /// @param twists         Aerodynamic twist angles [deg] at each section.
    /// @param aero_centres_x X-coordinates of the aerodynamic centre for each
    ///                       section (in local chord coordinates).
    /// @param aero_centres_y Y-coordinates of the aerodynamic centre for each
    ///                       section (in local chord coordinates).
    /// @param airfoils       Identifier strings naming the airfoil profile at
    ///                       each section.
    // -----------------------------------------------------------------------
    TurbineGeometry(std::unique_ptr<BladeInterpolator> blade);

    // -----------------------------------------------------------------------
    /// @name Configuration
    /// @{
    // -----------------------------------------------------------------------

    /// @brief Sets the rotor-level geometric parameters.
    ///
    /// Must be called before @ref PreComputeRotationMatrices.
    ///
    /// @param hub_radius  Radius of the rotor hub [m].
    /// @param cone        Half-cone angle of the rotor [deg].
    /// @param yaw         Nacelle yaw angle [deg].
    /// @param tilt        Shaft tilt angle [deg].
    /// @param tower_dist  Distance from the tower centreline to the rotor
    ///                    hub [m] (positive upwind).
    /// @param hub_height  Height of the rotor hub above ground [m].
    void setTurbineConfiguration(
        double hub_radius,
        double cone,
        double yaw,
        double tilt,
        double tower_dist,
        double hub_height,
        int num_blades)
    {
        hub_radius_ = hub_radius;
        cone_ = cone * std::numbers::pi / 180.0; // convert to radians
        yaw_ = yaw * std::numbers::pi / 180.0;   // convert to radians
        tilt_ = tilt * std::numbers::pi / 180.0; // convert to radians
        tower_dist_ = tower_dist;
        hub_height_ = hub_height;
        num_blades_ = num_blades;
    }

    /// @brief Pre-computes the fixed rotation matrices from the current macro
    ///        properties.
    ///
    /// Populates a1_, a2_, a12_, a21_, a34_, and a43_. Must be called after
    /// @ref SetMacroProperties and before any position or matrix queries.
    void PreComputeRotationMatrices();

    /// @brief Returns the global (world-frame) 3-D positions of all sections
    ///        at a given blade azimuth angle.
    ///
    /// The returned vector has @ref num_sections_ elements, one per
    /// aerodynamic section.  Positions include the hub height offset and the
    /// tower distance offset.
    ///
    /// @param psi  Blade azimuth angle ψ [rad], measured from the top of the
    ///             rotor disc.
    /// @return     World-frame positions of each section as Vec3D<double>.
    std::vector<WVPMUtilities::Vec3D<double>>
    GlobalPositionsAtPsi(double psi) const;

    /// @brief Returns the hub-relative 3-D positions of all sections at a
    ///        given blade azimuth angle.
    ///
    /// Positions are expressed in the hub-centre frame (no hub-height or
    /// tower-distance offset applied).
    ///
    /// @param psi  Blade azimuth angle ψ [rad].
    /// @return     Hub-relative positions of each section as Vec3D<double>.
    std::vector<WVPMUtilities::Vec3D<double>>
    HubRelativePositionsAtPsi(double psi) const;

    /// @brief Returns the 3×3 rotation matrix that transforms vectors from
    ///        the world frame into the blade-local frame at azimuth @a psi.
    ///
    /// Equivalent to the product a34 × a23(ψ) × a12.
    ///
    /// @param psi  Blade azimuth angle ψ [rad].
    /// @return     World-to-blade-local rotation matrix.
    WVPMUtilities::SquareMatrix<double> CreateMatrix14(double psi) const;

    /**
     *
     */
    // std::unique_ptr<BladeInterpolator> &blade() { return blade_; };

    /// @}

    // -----------------------------------------------------------------------
    /// @name Per-section accessors
    /// @{
    // -----------------------------------------------------------------------

    double chord(std::size_t i) const
    {
        return blade_->getBladeSections().at(i)->chord;
    }

    double aeroCentreX(std::size_t i) const
    {
        return blade_->getBladeSections().at(i)->xt4;
    }

    double aeroCentreY(std::size_t i) const
    {
        return blade_->getBladeSections().at(i)->yt4;
    }

    double radius(std::size_t i) const { return blade_->getBladeSections().at(i)->bladeRadius + hub_radius_; } // radii_[i]; }
    // double twist(std::size_t i) const
    double twist(std::size_t i) const
    {
        return blade_->getBladeSections().at(i)->twist;
    }
    double RotorRadius() const { return blade_->getBladeSections().back()->bladeRadius + hub_radius_; } // radii_.back(); }
    std::size_t num_sections() const { return num_sections_; }

    double hub_height() const { return hub_height_; }

    // Polar lookup — solver calls this, never touches BladeInterpolator directly
    void InterpForCoeff(std::size_t sec, double Re, double Ma, double alpha,
                        double *Cl, double *Cd, double *Cm) const;

    // Blade count — set once from config, read by solver
    void set_number_of_blades(int number_of_blades) { num_blades_ = number_of_blades; }
    int num_blades() const { return num_blades_; }
    double num_blades_as_double() const { return static_cast<double>(num_blades_); }

    // /// @brief Returns the chord length [m] at section @a i.
    // double chord(std::size_t i) const { return chords_[i]; }

    // /// @brief Returns the radial position [m] at section @a i.
    // double radius(std::size_t i) const { return radii_[i]; }

    // /// @brief Returns the aerodynamic twist angle [deg] at section @a i.
    // double twist(std::size_t i) const { return twists_[i]; }

    // /// @brief Returns the absolute section thickness [m] at section @a i.
    // double thickness(std::size_t i) const { return thicknesses_[i]; }

    // /// @brief Returns the X-coordinate of the aerodynamic centre at section @a i.
    // double aero_centre_x(std::size_t i) const { return aero_centres_x_[i]; }

    // /// @brief Returns the Y-coordinate of the aerodynamic centre at section @a i.
    // double aero_centre_y(std::size_t i) const { return aero_centres_y_[i]; }

    // /// @brief Returns the relative radial position at section @a i as a
    // ///        percentage of the tip radius (r/R × 100).
    // double relative_radius(std::size_t i) const
    // {
    //     return radii_[i] / radii_.back() * 100.0;
    // }

    // /// @brief Returns the relative thickness at section @a i as a percentage
    // ///        of the local chord (t/c × 100).
    // double relative_thickness(std::size_t i) const
    // {
    //     return thicknesses_[i] / chords_[i] * 100.0;
    // }

    /// @}

    // -----------------------------------------------------------------------
    /// @name Whole-rotor accessors
    /// @{
    // -----------------------------------------------------------------------

    // /// @brief Returns the rotor tip radius [m] (last element of @ref radii_).
    // double RotorRadius() const { return radii_.back(); }

    // /// @brief Returns the hub height above ground [m].
    // double hub_height() const { return hub_height_; }

    // /// @brief Returns the number of aerodynamic sections.
    // std::size_t num_sections() const { return num_sections_; }

    // /// @brief Returns a copy of the radial position vector [m].
    // std::vector<double> radii() const { return radii_; }

    // /// @brief Returns a copy of the section thickness vector [m].
    // std::vector<double> thicknesses() const { return thicknesses_; }

    // /// @brief Returns a copy of the airfoil identifier vector.
    // std::vector<std::string> airfoils() const { return airfoils_; }

    /// @}

private:
    // -----------------------------------------------------------------------
    /// @name Azimuth-dependent transformation matrices
    ///
    /// These matrices depend on the instantaneous blade azimuth angle ψ and
    /// are computed on demand rather than cached.
    /// @{
    // -----------------------------------------------------------------------

    /// @brief Rotation matrix from hub-shaft frame to blade-azimuth frame
    ///        at angle ψ.  Represents a rotation about the rotor axis.
    /// @param psi  Blade azimuth angle ψ [rad].
    WVPMUtilities::SquareMatrix<double> CreateMatrix23(double psi) const;

    /// @brief Rotation matrix from blade-root frame to hub-shaft frame at ψ.
    ///        Computed as transpose(a23(ψ)) × a43.
    /// @param psi  Blade azimuth angle ψ [rad].
    WVPMUtilities::SquareMatrix<double> CreateMatrix42(double psi) const;

    /// @brief Rotation matrix from world frame to blade-azimuth frame at ψ.
    ///        Computed as a23(ψ) × a12.
    /// @param psi  Blade azimuth angle ψ [rad].
    WVPMUtilities::SquareMatrix<double> CreateMatrix13(double psi) const;

    /// @}

    // -----------------------------------------------------------------------
    /// @name Fixed rotation matrix factories
    ///
    /// These helpers build the three matrices that depend only on the fixed
    /// macro-geometry angles set via @ref SetMacroProperties.
    /// @{
    // -----------------------------------------------------------------------

    /// @brief Builds the yaw rotation matrix (world → yawed frame).
    WVPMUtilities::SquareMatrix<double> MakeYawMatrix() const;

    /// @brief Builds the shaft-tilt rotation matrix (yawed → tilted frame).
    WVPMUtilities::SquareMatrix<double> MakeTiltMatrix() const;

    /// @brief Builds the cone rotation matrix (hub-shaft → blade-root frame).
    WVPMUtilities::SquareMatrix<double> MakeConeMatrix() const;

    /// @brief Returns a transposed copy of @a m without modifying the original.
    /// @param m  Matrix to transpose (passed by value to allow move).
    WVPMUtilities::SquareMatrix<double>
    Transposed(WVPMUtilities::SquareMatrix<double> m) const;

    /// @}

    // -----------------------------------------------------------------------
    /// @name Position helpers
    /// @{
    // -----------------------------------------------------------------------

    /// @brief Builds the tower-offset vectors in world coordinates for all
    ///        sections.
    ///
    /// Each vector equals (-tower_dist, 0, 0) rotated into the world frame
    /// via a21_.
    ///
    /// @return  Vector of per-section tower offsets in the world frame.
    std::vector<WVPMUtilities::Vec3D<double>> ComputeTowerOffsets() const;

    /// @brief Rotates the radial positions of all sections by @a mat.
    ///
    /// Each section radius is placed at (0, 0, r) in blade-local coordinates
    /// and then rotated into the target frame by @a mat.
    ///
    /// @param mat  3×3 rotation matrix to apply.
    /// @return     Rotated positions for every section.
    std::vector<WVPMUtilities::Vec3D<double>>
    RotateRadiiByMatrix(WVPMUtilities::SquareMatrix<double> const &mat) const;

    /// @}

    // -----------------------------------------------------------------------
    /// @name Blade section data (immutable after construction)
    // -----------------------------------------------------------------------
    // std::vector<double> const chords_;         ///< Chord lengths [m].
    std::vector<double> radii_; ///< Radial positions [m].
    // std::vector<double> const thicknesses_;    ///< Absolute thicknesses [m].
    // std::vector<double> const twists_;         ///< Twist angles [deg].
    // std::vector<double> const aero_centres_x_; ///< Aero centre x-coords.
    // std::vector<double> const aero_centres_y_; ///< Aero centre y-coords.
    // std::vector<std::string> const airfoils_;  ///< Airfoil identifiers.
    int num_sections_; ///< Number of sections.
    int num_blades_;   ///< Number of blades.

    std::unique_ptr<BladeInterpolator> blade_;

    // -----------------------------------------------------------------------
    /// @name Macro geometry (set via SetMacroProperties)
    // -----------------------------------------------------------------------
    double hub_radius_; ///< Rotor hub radius [m].
    double cone_;       ///< Rotor cone half-angle [rad].
    double yaw_;        ///< Nacelle yaw angle [rad].
    double tilt_;       ///< Shaft tilt angle [rad].
    double tower_dist_; ///< Hub-to-tower distance [m].
    double hub_height_; ///< Hub height above ground [m].

    // -----------------------------------------------------------------------
    /// @name Pre-computed fixed rotation matrices
    ///
    /// Populated by @ref PreComputeRotationMatrices.
    // -----------------------------------------------------------------------
    WVPMUtilities::SquareMatrix<double> a1_;  ///< World → yawed frame.
    WVPMUtilities::SquareMatrix<double> a2_;  ///< Yawed → tilted frame.
    WVPMUtilities::SquareMatrix<double> a12_; ///< World → hub-shaft frame.
    WVPMUtilities::SquareMatrix<double> a21_; ///< Hub-shaft → world.
    WVPMUtilities::SquareMatrix<double> a34_; ///< Hub-shaft → blade-root.
    WVPMUtilities::SquareMatrix<double> a43_; ///< Blade-root → hub-shaft.
};

#endif // CC_TURBINEGEOMETRY_H_

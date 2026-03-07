#pragma once
/**
 * @file AirfoilDataAdapter.h
 * @brief Converts SolidTurbine AirfoilGeometryData → bladenoise::io::AirfoilData.
 *
 * This is the only translation unit that includes both AirfoilGeometryData.h
 * and bladenoise/io/IOTypes.h.  It lives in the bridge layer alongside
 * BladeNoiseConfigBuilder.
 *
 * ## Coordinate convention
 * bladenoise expects Selig format: TE → upper surface → LE → lower surface → TE
 * (counter-clockwise when viewed from the right, x normalised 0…1).
 * AirfoilGeometryData stores coordinates in exactly this order after
 * orientationToDefaultCounterClockwiseOrientation() is called by the parser.
 *
 * ## SOLID
 *  S – one responsibility: translate the coordinate representation.
 *  D – BladeNoiseConfigBuilder depends on this adapter, not on
 *      AirfoilGeometryData directly.
 */
#include "AirfoilGeometryData.h"
#include "bladenoise/io/IOTypes.h"

class AirfoilDataAdapter
{
public:
    /**
     * @brief Convert SolidTurbine airfoil geometry to bladenoise AirfoilData.
     *
     * Walks the full coordinate list in storage order (already CCW after
     * parser normalisation) and extracts the x/y pairs.
     *
     * @param src  SolidTurbine airfoil geometry (normalised x/y, range 0…1).
     * @param dst  Output bladenoise AirfoilData.
     */
    static void Convert(AirfoilGeometryData const &src,
                        bladenoise::io::AirfoilData &dst)
    {
        dst.clear();
        dst.name = src.getName();

        auto const &coords = src.getCoordinates();
        dst.x.reserve(coords.size());
        dst.y.reserve(coords.size());

        for (auto const &c : coords)
        {
            dst.x.push_back(c.x);
            dst.y.push_back(c.y);
        }

        dst.num_points = dst.x.size();

        // Mark as closed if first and last points coincide (TE-to-TE wrap)
        dst.is_closed = (dst.num_points >= 2 &&
                         std::abs(dst.x.front() - dst.x.back()) < 1e-9 &&
                         std::abs(dst.y.front() - dst.y.back()) < 1e-9);
    }

    /**
     * @brief Returns true if the airfoil has enough points for
     *        XfoilBoundaryLayerCalculator (requires >= 10 points).
     */
    static bool IsUsableForXfoil(AirfoilGeometryData const &src)
    {
        return src.getCoordinates().size() >= 10;
    }
};

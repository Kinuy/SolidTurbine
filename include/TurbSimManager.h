#pragma once
/**
 * @file TurbSimManager.h
 * @brief Refactored TurbSimManager — pure orchestrator for TurbSim operations.
 *
 * SOLID compliance:
 *  S – Responsible only for coordinating load → query → diagnostics.
 *      No file parsing, no interpolation arithmetic, no timing math.
 *  O – New file formats (HDF5, HAWC2) or interpolation schemes (bicubic)
 *      are injected; this class never changes for them.
 *  L – Fully satisfies the VelocityAt / hub_velocity / grid contract
 *      used by InletVelocityProviders (TurbSimInletProvider).
 *  I – IInletVelocityProvider consumers see only VelocityAt() and
 *      hub_velocity() via that interface; they are not coupled to the
 *      diagnostic / iteration count methods here.
 *  D – Depends on ITurbSimReader and ITurbSimInterpolator abstractions.
 *
 * Migration note
 * ──────────────
 * Old code:
 *   TurbSimManager tsm;
 *   tsm.Read("file.wnd");
 *   tsm.VelocityAt(point, it);
 *
 * New code (drop-in):
 *   BladedBinaryReader reader;
 *   BilinearTurbSimInterpolator interp;
 *   TurbSimManager tsm(&reader, &interp);
 *   tsm.Load("file.wnd");
 *   tsm.VelocityAt(point, it);
 */
#include <string>
#include <stdexcept>
#include <tuple>
#include <vector>
#include <memory>

#include "ITurbSimReader.h"
#include "ITurbSimInterpolator.h"
#include "TurbSimGrid.h"
#include "TurbSimVelocityData.h"
#include "TurbSimTimingInfo.h"

class TurbSimManager
{
public:
    // ── Construction ─────────────────────────────────────────────────────────
    /**
     * @brief Construct with injected reader and interpolator.
     *
     * @param reader      Strategy for parsing a TurbSim file.
     * @param interpolator Strategy for interpolating the velocity grid.
     */
    TurbSimManager(ITurbSimReader const *reader,
                   ITurbSimInterpolator const *interpolator);

    // Non-copyable (large velocity data).
    TurbSimManager(TurbSimManager const &) = delete;
    TurbSimManager &operator=(TurbSimManager const &) = delete;
    TurbSimManager(TurbSimManager &&) = default;
    TurbSimManager &operator=(TurbSimManager &&) = default;

    // ── Loading ───────────────────────────────────────────────────────────────
    /**
     * @brief Parse a TurbSim file using the injected reader.
     *
     * Must be called before any VelocityAt() or diagnostic queries.
     * Replaces the original Read() method.
     *
     * @throws std::runtime_error on I/O or parse failure.
     */
    void Load(std::string const &filename);

    // ── Velocity query (IInletVelocityProvider compatible API) ────────────────
    /**
     * @brief Return the interpolated 3-D wind velocity at a point and iteration.
     *
     * @param point      3-D global position [m].  Only y and z are used.
     * @param iteration  User-facing (non-padded) time-step index.
     */
    WVPMUtilities::Vec3D<double> VelocityAt(
        WVPMUtilities::Vec3D<double> const &point,
        unsigned iteration) const;

    // ── Accessors ─────────────────────────────────────────────────────────────
    double hub_velocity() const;
    double timestep() const;
    unsigned requested_num_iterations() const;
    TurbSimGrid grid() const;

    /**
     * @brief Return all velocity components for one user-facing iteration
     *        as three separate vectors (u, v, w), matching the original API.
     */
    std::tuple<std::vector<double>,
               std::vector<double>,
               std::vector<double>>
    velocity_data_not_padded_as_vectors(unsigned iteration) const;

private:
    // ── Injected dependencies (non-owning) ───────────────────────────────────
    ITurbSimReader const *reader_;
    ITurbSimInterpolator const *interpolator_;

    // ── Loaded data ───────────────────────────────────────────────────────────
    TurbSimGrid grid_;
    TurbSimVelocityData velocity_;
    TurbSimTimingInfo *timing_{nullptr}; // optional until Load() is called
    double hub_velocity_{0.0};
    bool loaded_{false};

    // Stable storage for timing (avoids optional<> overhead)
    std::unique_ptr<TurbSimTimingInfo> timing_store_;

    void AssertLoaded() const;
};

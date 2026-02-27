/**
 * @file LossModels.cpp
 * @brief Implementation of aerodynamic loss models.
 *
 * All three loss models (PrandtlTipLoss, PrandtlHubLoss, CombinedLoss, NoLoss)
 * are header-only since they are short and templated on the interface.
 * This .cpp exists as a compilation unit for:
 *   - Any future non-trivial loss model logic.
 *   - Explicit template instantiations if needed.
 */
#include "LossModels.h"

// ─────────────────────────────────────────────────────────────────────────────
// All implementations are inline in LossModels.h.
// ─────────────────────────────────────────────────────────────────────────────

// ─────────────────────────────────────────────────────────────────────────────
// MakeCombinedLoss — convenience factory that keeps ownership clear.
//
// Returns a tuple of {tip, hub, combined} so all three can be kept alive
// as long as the solver runs.
// ─────────────────────────────────────────────────────────────────────────────
struct OwnedCombinedLoss
{
    std::unique_ptr<PrandtlTipLoss> tip;
    std::unique_ptr<PrandtlHubLoss> hub;
    std::unique_ptr<CombinedLoss> combined;
};

OwnedCombinedLoss MakePrandtlCombinedLoss()
{
    OwnedCombinedLoss o;
    o.tip = std::make_unique<PrandtlTipLoss>();
    o.hub = std::make_unique<PrandtlHubLoss>();
    o.combined = std::make_unique<CombinedLoss>(o.tip.get(), o.hub.get());
    return o;
}

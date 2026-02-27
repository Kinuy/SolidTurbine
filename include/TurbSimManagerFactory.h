#pragma once
/**
 * @file TurbSimManagerFactory.h
 * @brief Factory that assembles a TurbSimManager with concrete dependencies.
 *
 * Migration path (drop-in replacement for the original Read() pattern):
 *
 *   Old code:
 *     TurbSimManager tsm;
 *     tsm.Read("wind.wnd");
 *
 *   New code:
 *     auto tsm = TurbSimManagerFactory{}.Build("wind.wnd");
 *     // tsm is a std::unique_ptr<TurbSimManager>, ready to use.
 *
 * The factory owns the concrete strategy objects; the manager borrows them.
 * The factory must outlive the manager — the same lifetime as a local scope
 * or the class that owns both.
 */
#include <memory>
#include <string>

#include "TurbSimManager.h"
#include "BladedBinaryReader.h"
#include "BilinearTurbSimInterpolator.h"

class TurbSimManagerFactory
{
public:
    /**
     * @brief Build and load a TurbSimManager using the default concrete
     *        strategies: BladedBinaryReader + BilinearTurbSimInterpolator.
     *
     * @param filename  Path to a Bladed/AeroDyn .wnd file.
     * @return          Ready-to-use TurbSimManager.
     */
    std::unique_ptr<TurbSimManager> Build(std::string const &filename)
    {
        auto reader = std::make_unique<BladedBinaryReader>();
        auto interp = std::make_unique<BilinearTurbSimInterpolator>();

        owned_readers_.push_back(std::move(reader));
        owned_interps_.push_back(std::move(interp));

        auto mgr = std::make_unique<TurbSimManager>(
            owned_readers_.back().get(),
            owned_interps_.back().get());

        mgr->Load(filename);
        return mgr;
    }

    /**
     * @brief Build a TurbSimManager with custom strategies.
     *
     * Useful when injecting a mock reader in unit tests, or a bicubic
     * interpolator for higher accuracy.
     */
    std::unique_ptr<TurbSimManager> BuildCustom(
        std::string const &filename,
        ITurbSimReader const *reader,
        ITurbSimInterpolator const *interpolator)
    {
        auto mgr = std::make_unique<TurbSimManager>(reader, interpolator);
        mgr->Load(filename);
        return mgr;
    }

private:
    // Factory retains ownership of all strategies it creates.
    std::vector<std::unique_ptr<BladedBinaryReader>> owned_readers_;
    std::vector<std::unique_ptr<BilinearTurbSimInterpolator>> owned_interps_;
};

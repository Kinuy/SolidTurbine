#include "AirfoilPolarData.h"


AirfoilPolarData::AirfoilPolarData()
{
}

AirfoilPolarData::AirfoilPolarData(std::string n)
    : name(n), interpolationStrategy(std::make_unique<LinearInterpolationStrategy>()) {
}

std::vector<std::vector<double>> AirfoilPolarData::buildMachGrid(
    const std::vector<double>& reynolds,
    const std::vector<double>& machs) const {

    std::vector<std::vector<double>> machGrid(reynolds.size());
    for (size_t i = 0; i < reynolds.size(); ++i) {
        machGrid[i] = machs;  // Same Mach numbers for all Reynolds
    }
    return machGrid;
}


std::vector<std::vector<std::vector<double>>> AirfoilPolarData::buildAlphaGrid(
    const std::vector<double>& reynolds,
    const std::vector<double>& machs,
    const std::vector<double>& alphas) const {

    std::vector<std::vector<std::vector<double>>> alphaGrid(reynolds.size());
    for (size_t i = 0; i < reynolds.size(); ++i) {
        alphaGrid[i].resize(machs.size());
        for (size_t j = 0; j < machs.size(); ++j) {
            alphaGrid[i][j] = alphas;  // Same alphas for all Re/Mach combinations
        }
    }
    return alphaGrid;
}

// void AirfoilPolarData::buildInterpolationGrids(
//     const std::vector<double>& reynolds,
//     const std::vector<double>& machs,
//     const std::vector<double>& alphas,
//     std::vector<std::vector<std::vector<double>>>& clGrid,
//     std::vector<std::vector<std::vector<double>>>& cdGrid,
//     std::vector<std::vector<std::vector<double>>>& cmGrid) const {

//     // Initialize grids
//     clGrid.resize(reynolds.size());
//     cdGrid.resize(reynolds.size());
//     cmGrid.resize(reynolds.size());

//     for (size_t r = 0; r < reynolds.size(); ++r) {
//         clGrid[r].resize(machs.size());
//         cdGrid[r].resize(machs.size());
//         cmGrid[r].resize(machs.size());

//         for (size_t m = 0; m < machs.size(); ++m) {
//             clGrid[r][m].resize(alphas.size());
//             cdGrid[r][m].resize(alphas.size());
//             cmGrid[r][m].resize(alphas.size());

//             for (size_t a = 0; a < alphas.size(); ++a) {
//                 AirfoilOperationCondition condition(reynolds[r], machs[m], alphas[a]);
//                 auto coeffs = findOrInterpolateCoefficients(condition);

//                 clGrid[r][m][a] = coeffs.cl;
//                 cdGrid[r][m][a] = coeffs.cd;
//                 cmGrid[r][m][a] = coeffs.cm;
//             }
//         }
//     }
// }

void AirfoilPolarData::buildInterpolationGrids(
    const std::vector<double> &reynolds,
    const std::vector<double> &machs,
    const std::vector<double> &alphas,
    std::vector<std::vector<std::vector<double>>> &clGrid,
    std::vector<std::vector<std::vector<double>>> &cdGrid,
    std::vector<std::vector<std::vector<double>>> &cmGrid) const
{
    // ── Allocate grids first ──────────────────────────────────────────────────
    clGrid.assign(reynolds.size(), std::vector<std::vector<double>>(
                                       machs.size(), std::vector<double>(alphas.size(), 0.0)));
    cdGrid.assign(reynolds.size(), std::vector<std::vector<double>>(
                                       machs.size(), std::vector<double>(alphas.size(), 0.0)));
    cmGrid.assign(reynolds.size(), std::vector<std::vector<double>>(
                                       machs.size(), std::vector<double>(alphas.size(), 0.0)));

    // ── Fill with direct lookups ──────────────────────────────────────────────
    for (size_t r = 0; r < reynolds.size(); ++r)
        for (size_t m = 0; m < machs.size(); ++m)
            for (size_t a = 0; a < alphas.size(); ++a)
            {
                AirfoilOperationCondition cond(reynolds[r], machs[m], alphas[a]);
                auto it = std::find_if(polarData.begin(), polarData.end(),
                                       [&cond](const AirfoilPolarPoint &p)
                                       {
                                           return std::abs(p.condition.reynolds - cond.reynolds) < 1e-6 &&
                                                  std::abs(p.condition.mach - cond.mach) < 1e-6 &&
                                                  std::abs(p.condition.alpha - cond.alpha) < 1e-6;
                                       });

                clGrid[r][m][a] = (it != polarData.end()) ? it->coefficients.cl : 0.0;
                cdGrid[r][m][a] = (it != polarData.end()) ? it->coefficients.cd : 0.0;
                cmGrid[r][m][a] = (it != polarData.end()) ? it->coefficients.cm : 0.0;
            }
}

// AirfoilAeroCoefficients AirfoilPolarData::findOrInterpolateCoefficients(const AirfoilOperationCondition& condition) const {
//     // Find exact match first
//     auto it = std::find_if(polarData.begin(), polarData.end(),
//                            [&condition](const AirfoilPolarPoint &point)
//                            {
//                                return std::abs(point.condition.reynolds - condition.reynolds) < 1e-6 &&
//                                       std::abs(point.condition.mach - condition.mach) < 1e-6 &&
//                                       std::abs(point.condition.alpha - condition.alpha) < 1e-6;
//                            });

//     if (it != polarData.end()) {
//         return it->coefficients;
//     }

//     // TODO: fix: Return zero coefficients for missing data (could be improved with nearest neighbor)
//     return AirfoilAeroCoefficients(0.0, 0.0, 0.0);
// }

// AirfoilAeroCoefficients AirfoilPolarData::findOrInterpolateCoefficients(
//     const AirfoilOperationCondition &condition) const
// {
//     if (polarData.empty())
//         throw std::runtime_error(
//             "AirfoilPolarData '" + name + "': no polar data available");

//     // ── Exact match ───────────────────────────────────────────────────────────
//     auto it = std::find_if(polarData.begin(), polarData.end(),
//                            [&condition](const AirfoilPolarPoint &p)
//                            {
//                                return std::abs(p.condition.reynolds - condition.reynolds) < 1e-6 &&
//                                       std::abs(p.condition.mach - condition.mach) < 1e-6 &&
//                                       std::abs(p.condition.alpha - condition.alpha) < 1e-6;
//                            });

//     if (it != polarData.end())
//         return it->coefficients;

//     // ── Boundary checks — throw if outside data range ─────────────────────────
//     auto reynolds = getReynoldsNumbers(); // sorted ascending (via std::set)
//     auto machs = getMachNumbers();
//     auto alphas = getAnglesOfAttack();

//     if (condition.reynolds < reynolds.front() || condition.reynolds > reynolds.back())
//         throw std::out_of_range(
//             "AirfoilPolarData '" + name + "': Reynolds " +
//             std::to_string(condition.reynolds) +
//             " is outside data range [" +
//             std::to_string(reynolds.front()) + ", " +
//             std::to_string(reynolds.back()) + "]");

//     if (condition.mach < machs.front() || condition.mach > machs.back())
//         throw std::out_of_range(
//             "AirfoilPolarData '" + name + "': Mach " +
//             std::to_string(condition.mach) +
//             " is outside data range [" +
//             std::to_string(machs.front()) + ", " +
//             std::to_string(machs.back()) + "]");

//     if (condition.alpha < alphas.front() || condition.alpha > alphas.back())
//         throw std::out_of_range(
//             "AirfoilPolarData '" + name + "': alpha " +
//             std::to_string(condition.alpha) +
//             " is outside data range [" +
//             std::to_string(alphas.front()) + ", " +
//             std::to_string(alphas.back()) + "]");

//     // ── Interpolate ───────────────────────────────────────────────────────────
//     return performTrilinearInterpolation(condition);
// }

AirfoilAeroCoefficients AirfoilPolarData::findOrInterpolateCoefficients(
    const AirfoilOperationCondition &condition) const
{

    if (!interpolationStrategy)
        throw std::runtime_error("interpolationStrategy is null in '" + name + "'");

    if (polarData.empty())
        throw std::runtime_error(
            "AirfoilPolarData '" + name + "': no polar data available");

    // ── Exact match ───────────────────────────────────────────────────────────
    auto it = std::find_if(polarData.begin(), polarData.end(),
                           [&condition](const AirfoilPolarPoint &p)
                           {
                               return std::abs(p.condition.reynolds - condition.reynolds) < 1e-6 &&
                                      std::abs(p.condition.mach - condition.mach) < 1e-6 &&
                                      std::abs(p.condition.alpha - condition.alpha) < 1e-6;
                           });

    if (it != polarData.end())
        return it->coefficients;

    // ── Build clamped condition (Re, Mach) — throw only for alpha ─────────────
    auto reynolds = getReynoldsNumbers(); // sorted ascending
    auto machs = getMachNumbers();
    auto alphas = getAnglesOfAttack();

    double re = condition.reynolds;
    double mach = condition.mach;

    if (re < reynolds.front())
    {
        // std::cout << "[AirfoilPolarData '" << name << "'] Re clamped: "
        //           << re << " -> " << reynolds.front() << " (below range)\n";
        re = reynolds.front();
    }
    else if (re > reynolds.back())
    {
        // std::cout << "[AirfoilPolarData '" << name << "'] Re clamped: "
        //           << re << " -> " << reynolds.back() << " (above range)\n";
        re = reynolds.back();
    }

    if (mach < machs.front())
    {
        // std::cout << "[AirfoilPolarData '" << name << "'] Mach clamped: "
        //           << mach << " -> " << machs.front() << " (below range)\n";
        mach = machs.front();
    }
    else if (mach > machs.back())
    {
        // std::cout << "[AirfoilPolarData '" << name << "'] Mach clamped: "
        //           << mach << " -> " << machs.back() << " (above range)\n";
        mach = machs.back();
    }

    if (condition.alpha < alphas.front() || condition.alpha > alphas.back())
        throw std::out_of_range(
            "AirfoilPolarData '" + name + "': alpha " +
            std::to_string(condition.alpha) +
            " is outside data range [" +
            std::to_string(alphas.front()) + ", " +
            std::to_string(alphas.back()) + "]");

    return performTrilinearInterpolation({re, mach, condition.alpha});
}

AirfoilAeroCoefficients AirfoilPolarData::interpolateCoefficients(const AirfoilOperationCondition& target) const {
    if (polarData.empty()) {
        throw std::runtime_error("No polar data available for interpolation");
    }

    // For exact matches, return directly
    auto exactMatch = std::find_if(polarData.begin(), polarData.end(),
                                   [&target](const AirfoilPolarPoint &point)
                                   {
                                       return std::abs(point.condition.reynolds - target.reynolds) < 1e-6 &&
                                              std::abs(point.condition.mach - target.mach) < 1e-6 &&
                                              std::abs(point.condition.alpha - target.alpha) < 1e-6;
                                   });

    if (exactMatch != polarData.end()) {
        return exactMatch->coefficients;
    }

    // Use trilinear interpolation for non-exact matches
    return performTrilinearInterpolation(target);
}


AirfoilAeroCoefficients AirfoilPolarData::performTrilinearInterpolation(const AirfoilOperationCondition& target) const {
    auto reynolds = getReynoldsNumbers();
    auto machs = getMachNumbers();
    auto alphas = getAnglesOfAttack();

    // Extract coefficient vectors for interpolation
    std::vector<double> clData, cdData, cmData;
    std::vector<std::vector<std::vector<double>>> clGrid, cdGrid, cmGrid;

    buildInterpolationGrids(reynolds, machs, alphas, clGrid, cdGrid, cmGrid);

    // Use existing trilinear interpolation
    double cl = MathUtility::triLinearInterpolation(
        target.reynolds,
        target.mach,
        target.alpha,
        reynolds,
        buildMachGrid(reynolds, machs),
        buildAlphaGrid(reynolds, machs, alphas), clGrid);

    double cd = MathUtility::triLinearInterpolation(
        target.reynolds,
        target.mach,
        target.alpha,
        reynolds,
        buildMachGrid(reynolds, machs),
        buildAlphaGrid(reynolds, machs, alphas), cdGrid);

    double cm = MathUtility::triLinearInterpolation(
        target.reynolds,
        target.mach,
        target.alpha,
        reynolds,
        buildMachGrid(reynolds, machs),
        buildAlphaGrid(reynolds, machs, alphas), cmGrid);

    return AirfoilAeroCoefficients(cl, cd, cm);
}


std::unique_ptr<AirfoilPolarData> AirfoilPolarData::interpolateBetweenPolars(
    const AirfoilPolarData& leftPolar,
    const AirfoilPolarData& rightPolar,
    double targetThickness) {

    auto result = std::make_unique<AirfoilPolarData>("interpolated_T" + std::to_string(targetThickness));
    result->relativeThickness = targetThickness;

    // Create combined operating condition space
    auto combinedConditions = createCombinedConditions(leftPolar, rightPolar);

    // Calculate interpolation factor
    double leftThickness = leftPolar.getRelativeThickness();
    double rightThickness = rightPolar.getRelativeThickness();
    double factor = (targetThickness - leftThickness) / (rightThickness - leftThickness);

    // Interpolate coefficients for each operating condition
    for (const auto& condition : combinedConditions) {
        auto leftCoeffs = leftPolar.interpolateCoefficients(condition);
        auto rightCoeffs = rightPolar.interpolateCoefficients(condition);

        // Linear interpolation between left and right coefficients
        AirfoilAeroCoefficients interpolatedCoeffs(
            leftCoeffs.cl + factor * (rightCoeffs.cl - leftCoeffs.cl),
            leftCoeffs.cd + factor * (rightCoeffs.cd - leftCoeffs.cd),
            leftCoeffs.cm + factor * (rightCoeffs.cm - leftCoeffs.cm)
        );

        result->polarData.emplace_back(condition, interpolatedCoeffs);
    }

    return result;
}


std::vector<AirfoilOperationCondition> AirfoilPolarData::createCombinedConditions(
    const AirfoilPolarData& left,
    const AirfoilPolarData& right) {

    // Combine all unique operating conditions
    std::set<std::tuple<double, double, double>> uniqueConditions;

    for (const auto& point : left.polarData) {
        uniqueConditions.emplace(point.condition.reynolds,
                                 point.condition.mach,
                                 point.condition.alpha);
    }

    for (const auto& point : right.polarData) {
        uniqueConditions.emplace(point.condition.reynolds,
                                 point.condition.mach,
                                 point.condition.alpha);
    }

    std::vector<AirfoilOperationCondition> result;
    for (const auto& [re, mach, alpha] : uniqueConditions) {
        result.emplace_back(re, mach, alpha);
    }

    return result;
}

const std::string& AirfoilPolarData::getName() const { return name; }

void AirfoilPolarData::setRelativeThickness(double thicknes) { relativeThickness = thicknes; }


void AirfoilPolarData::addHeader(const std::string& header) {
    headers.push_back(header);
}

void AirfoilPolarData::addPolarPoint(const AirfoilOperationCondition& condition, const AirfoilAeroCoefficients& coefficients) {
    polarData.emplace_back(condition, coefficients);
}


std::vector<double> AirfoilPolarData::getReynoldsNumbers() const {
    std::set<double> uniqueRe;
    for (const auto& point : polarData) {
        uniqueRe.insert(point.condition.reynolds);
    }
    return { uniqueRe.begin(), uniqueRe.end() };
}


std::vector<double> AirfoilPolarData::getMachNumbers() const {
    std::set<double> uniqueMach;
    for (const auto& point : polarData) {
        uniqueMach.insert(point.condition.mach);
    }
    return { uniqueMach.begin(), uniqueMach.end() };
}


std::vector<double> AirfoilPolarData::getAnglesOfAttack() const {
    std::set<double> uniqueAlpha;
    for (const auto& point : polarData) {
        uniqueAlpha.insert(point.condition.alpha);
    }
    return { uniqueAlpha.begin(), uniqueAlpha.end() };
}

void AirfoilPolarData::setName(const std::string& refNum) { name = refNum; }

double AirfoilPolarData::getDepang() const
{
    return depang;
}

int AirfoilPolarData::getNAlpha() const
{
    return nAlpha;
}

int AirfoilPolarData::getNVals() const
{
    return nVals;
}


double AirfoilPolarData::getRelativeThickness() const {
    return relativeThickness;
}


const std::vector<AirfoilPolarPoint>& AirfoilPolarData::getPolarData() const {
    return polarData;
}


size_t AirfoilPolarData::getRowCount() const { 
    return polarData.size(); 
}

std::string AirfoilPolarData::getTypeName() const {
    return "AirfoilPerformance"; 
}

void AirfoilPolarData::setXa(double x)
{
    xa = x;
}

void AirfoilPolarData::setDepang(double depa)
{
    depang = depa;
}

void AirfoilPolarData::setNAlpha(int nAlph)
{
    nAlpha = nAlph;
}

void AirfoilPolarData::setNVals(int nVal)
{
    nVals = nVal;
}

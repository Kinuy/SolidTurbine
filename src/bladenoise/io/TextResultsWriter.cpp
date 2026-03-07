#include "bladenoise/io/TextResultsWriter.h"
#include <fstream>
#include <iomanip>

namespace bladenoise
{
    namespace io
    {

        bool TextResultsWriter::write(
            const std::string &filename,
            const RealVector &frequencies,
            const CombinedNoiseResults &results)
        {
            std::ofstream file(filename);
            if (!file.is_open())
            {
                error_message_ = "Could not open output file: " + filename;
                return false;
            }

            // Write header
            file << std::string(120, '=') << "\n";
            file << "BLADE NOISE PREDICTION RESULTS\n";
            file << std::string(120, '=') << "\n\n";

            file << "                                ONE-THIRD OCTAVE SOUND PRESSURE LEVELS (dB)\n\n";

            file << std::setw(14) << "FREQUENCY"
                 << std::setw(14) << "PRESSURE"
                 << std::setw(14) << "SUCTION"
                 << std::setw(14) << "SEPARATION"
                 << std::setw(14) << "LAMINAR"
                 << std::setw(14) << "BLUNTNESS"
                 << std::setw(14) << "INFLOW"
                 << std::setw(14) << "TOTAL" << "\n";

            file << std::setw(14) << "(Hz)"
                 << std::setw(14) << "SIDE TBL"
                 << std::setw(14) << "SIDE TBL"
                 << std::setw(14) << "SIDE TBL"
                 << std::setw(14) << ""
                 << std::setw(14) << ""
                 << std::setw(14) << ""
                 << std::setw(14) << "SPL" << "\n";

            file << std::string(112, '-') << "\n";

            // Write data
            file << std::fixed << std::setprecision(2);

            const size_t num_bands = frequencies.size();
            for (size_t i = 0; i < num_bands; ++i)
            {
                file << std::setw(14) << frequencies[i]
                     << std::setw(14) << results.tbl_pressure_side.spl[i]
                     << std::setw(14) << results.tbl_suction_side.spl[i]
                     << std::setw(14) << results.separation.spl[i]
                     << std::setw(14) << results.laminar_vortex.spl[i]
                     << std::setw(14) << results.bluntness.spl[i]
                     << std::setw(14) << results.turbulent_inflow.spl[i]
                     << std::setw(14) << results.total.spl[i] << "\n";
            }

            file << std::string(112, '-') << "\n";

            // Write overall SPL
            file << "\nOVERALL SPL (dB):\n";
            file << "  TBL Pressure Side: " << results.tbl_pressure_side.overall_spl << "\n";
            file << "  TBL Suction Side:  " << results.tbl_suction_side.overall_spl << "\n";
            file << "  Separation:        " << results.separation.overall_spl << "\n";
            file << "  Laminar VS:        " << results.laminar_vortex.overall_spl << "\n";
            file << "  Bluntness:         " << results.bluntness.overall_spl << "\n";
            file << "  Inflow:            " << results.turbulent_inflow.overall_spl << "\n";
            file << "  TOTAL:             " << results.total.overall_spl << "\n";

            return true;
        }

        std::unique_ptr<IResultsWriter> create_results_writer()
        {
            return std::make_unique<TextResultsWriter>();
        }

    } // namespace io
} // namespace bladenoise

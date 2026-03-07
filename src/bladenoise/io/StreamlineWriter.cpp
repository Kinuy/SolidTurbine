#include "bladenoise/io/StreamlineWriter.h"
#include <fstream>
#include <iomanip>
#include <sstream>

namespace bladenoise
{
    namespace io
    {

        bool StreamlineWriter::write_text(const std::string &filename,
                                          const StreamlineData &data) const
        {
            std::ofstream file(filename);
            if (!file.is_open())
            {
                error_message_ = "Could not open file: " + filename;
                return false;
            }

            file << "# Streamline Data (Guidati TI Noise Method)\n";
            file << "# Number of streamlines (npath): " << data.num_streamlines << "\n";
            file << "# Points per streamline (nstr): " << data.points_per_streamline << "\n";
            file << "# Streamline spacing (dpath): " << data.streamline_spacing << " m\n";
            file << "# Time step (deltat): " << data.time_step << "\n";
            file << "# Stagnation point: (" << data.stagnation_x << ", " << data.stagnation_y << ")\n";
            file << "#\n";
            file << "# Format: time  x  y  potential\n";
            file << "# Use with gnuplot: plot 'file.dat' index N with lines\n";
            file << "#\n";

            file << std::scientific << std::setprecision(8);

            for (int ipath = 0; ipath < data.num_streamlines; ++ipath)
            {
                file << "# Streamline " << ipath + 1 << " of " << data.num_streamlines << "\n";

                for (int istr = 0; istr < data.points_per_streamline; ++istr)
                {
                    Real t = (istr < static_cast<int>(data.time.size())) ? data.time[istr] : static_cast<Real>(istr);
                    Real x = data.x[ipath][istr];
                    Real y = data.y[ipath][istr];
                    Real pot = data.potential[ipath][istr];

                    file << std::setw(16) << t
                         << std::setw(16) << x
                         << std::setw(16) << y
                         << std::setw(16) << pot << "\n";
                }
                file << "\n"; // Blank line between streamlines (gnuplot index separator)
            }

            return true;
        }

        bool StreamlineWriter::write_tecplot(const std::string &filename,
                                             const StreamlineData &data) const
        {
            std::ofstream file(filename);
            if (!file.is_open())
            {
                error_message_ = "Could not open file: " + filename;
                return false;
            }

            file << "TITLE = \"Streamline Data - Guidati TI Noise\"\n";
            file << "VARIABLES = \"X\" \"Y\" \"Potential\" \"Time\"\n";

            file << std::scientific << std::setprecision(8);

            for (int ipath = 0; ipath < data.num_streamlines; ++ipath)
            {
                file << "ZONE T=\"Streamline " << ipath + 1 << "\", I="
                     << data.points_per_streamline << ", F=POINT\n";

                for (int istr = 0; istr < data.points_per_streamline; ++istr)
                {
                    Real t = (istr < static_cast<int>(data.time.size())) ? data.time[istr] : static_cast<Real>(istr);
                    file << data.x[ipath][istr] << " "
                         << data.y[ipath][istr] << " "
                         << data.potential[ipath][istr] << " "
                         << t << "\n";
                }
            }

            return true;
        }

        bool StreamlineWriter::write_csv(const std::string &filename,
                                         const StreamlineData &data) const
        {
            std::ofstream file(filename);
            if (!file.is_open())
            {
                error_message_ = "Could not open file: " + filename;
                return false;
            }

            // Header
            file << "streamline_id,point_id,time,x,y,potential\n";

            file << std::scientific << std::setprecision(8);

            for (int ipath = 0; ipath < data.num_streamlines; ++ipath)
            {
                for (int istr = 0; istr < data.points_per_streamline; ++istr)
                {
                    Real t = (istr < static_cast<int>(data.time.size())) ? data.time[istr] : static_cast<Real>(istr);

                    file << ipath + 1 << ","
                         << istr + 1 << ","
                         << t << ","
                         << data.x[ipath][istr] << ","
                         << data.y[ipath][istr] << ","
                         << data.potential[ipath][istr] << "\n";
                }
            }

            return true;
        }

        bool StreamlineWriter::write_vtk(const std::string &filename,
                                         const StreamlineData &data) const
        {
            std::ofstream file(filename);
            if (!file.is_open())
            {
                error_message_ = "Could not open file: " + filename;
                return false;
            }

            int total_points = data.num_streamlines * data.points_per_streamline;
            int total_lines = data.num_streamlines;
            int total_line_size = data.num_streamlines * (data.points_per_streamline + 1);

            // VTK header
            file << "# vtk DataFile Version 3.0\n";
            file << "Streamline Data - Guidati TI Noise Method\n";
            file << "ASCII\n";
            file << "DATASET POLYDATA\n";

            // Points
            file << "POINTS " << total_points << " double\n";
            file << std::scientific << std::setprecision(8);

            for (int ipath = 0; ipath < data.num_streamlines; ++ipath)
            {
                for (int istr = 0; istr < data.points_per_streamline; ++istr)
                {
                    file << data.x[ipath][istr] << " "
                         << data.y[ipath][istr] << " "
                         << "0.0\n"; // Z = 0 for 2D streamlines
                }
            }

            // Lines (polylines connecting streamline points)
            file << "LINES " << total_lines << " " << total_line_size << "\n";

            int point_offset = 0;
            for (int ipath = 0; ipath < data.num_streamlines; ++ipath)
            {
                file << data.points_per_streamline;
                for (int istr = 0; istr < data.points_per_streamline; ++istr)
                {
                    file << " " << point_offset + istr;
                }
                file << "\n";
                point_offset += data.points_per_streamline;
            }

            // Point data - Potential values
            file << "POINT_DATA " << total_points << "\n";
            file << "SCALARS Potential double 1\n";
            file << "LOOKUP_TABLE default\n";

            for (int ipath = 0; ipath < data.num_streamlines; ++ipath)
            {
                for (int istr = 0; istr < data.points_per_streamline; ++istr)
                {
                    file << data.potential[ipath][istr] << "\n";
                }
            }

            // Cell data - Streamline ID
            file << "CELL_DATA " << total_lines << "\n";
            file << "SCALARS StreamlineID int 1\n";
            file << "LOOKUP_TABLE default\n";
            for (int ipath = 0; ipath < data.num_streamlines; ++ipath)
            {
                file << ipath + 1 << "\n";
            }

            return true;
        }

        bool StreamlineWriter::write(const std::string &filename,
                                     const StreamlineData &data,
                                     const std::string &format) const
        {
            if (data.empty())
            {
                error_message_ = "Streamline data is empty";
                return false;
            }

            if (format == "text" || format == "dat" || format == "txt")
            {
                return write_text(filename, data);
            }
            else if (format == "csv")
            {
                return write_csv(filename, data);
            }
            else if (format == "tecplot" || format == "plt")
            {
                return write_tecplot(filename, data);
            }
            else if (format == "vtk")
            {
                return write_vtk(filename, data);
            }
            else
            {
                error_message_ = "Unknown format: " + format + ". Supported: text, csv, tecplot, vtk";
                return false;
            }
        }

    } // namespace io
} // namespace bladenoise

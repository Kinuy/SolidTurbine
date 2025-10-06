#include "VTKFileWriter.h"

const char* VTKFileWriter::fileExtension() const
{
    return ".vtk";
}


bool VTKFileWriter::write(
    const std::shared_ptr<Surface>& surface,
    const std::string& filename,
    int node_offset,
    int panel_offset,
    const std::vector<std::string>& view_names,
    const std::vector<Eigen::MatrixXd,
    Eigen::aligned_allocator<Eigen::MatrixXd> >& view_data)
{
    std::cout << "Surface " << surface->id << "\t: Saving to " << filename << "." << std::endl;

    // Save surface to VTK file:
    std::ofstream f;
    f.open(filename.c_str());

    f << "# vtk DataFile Version 2.0" << std::endl;
    f << "FieldData" << std::endl;
    f << "ASCII" << std::endl;
    f << "DATASET UNSTRUCTURED_GRID" << std::endl;
    f << "POINTS " << surface->getNumberOfNodes() << " double" << std::endl;

    for (int i = 0; i < surface->getNumberOfNodes(); i++) {
        for (int j = 0; j < 3; j++) {
            if (j > 0)
                f << ' ';
            f << surface->nodes[i](j);
        }
        f << std::endl;
    }

    f << std::endl;

    int size = 0;
    for (int i = 0; i < surface->getNumberOfPanels(); i++)
        size += (int)surface->panelNodes[i].size() + 1;

    f << "CELLS " << surface->getNumberOfPanels() << " " << size << std::endl;

    for (int i = 0; i < surface->getNumberOfPanels(); i++) {
        f << surface->panelNodes[i].size();

        for (int j = 0; j < (int)surface->panelNodes[i].size(); j++) {
            f << ' ';
            f << surface->panelNodes[i][j];
        }

        f << std::endl;
    }

    f << std::endl;

    f << "CELL_TYPES " << surface->getNumberOfPanels() << std::endl;

    for (int i = 0; i < surface->getNumberOfPanels(); i++) {
        int cell_type;
        switch (surface->panelNodes[i].size()) {
        case 3:
            cell_type = 5;
            break;
        case 4:
            cell_type = 9;
            break;
        default:
            std::cerr << "Surface " << surface->id << ": Unknown polygon at panel " << i << "." << std::endl;
            continue;
        }

        f << cell_type << std::endl;
    }

    f << std::endl;

    f << "CELL_DATA " << surface->getNumberOfPanels() << std::endl;

    for (int k = 0; k < (int)view_names.size(); k++) {
        if (view_data[k].cols() == 1) {
            f << "SCALARS " << view_names[k] << " double 1" << std::endl;
            f << "LOOKUP_TABLE default" << std::endl;
        }
        else
            f << "VECTORS " << view_names[k] << " double" << std::endl;

        for (int i = 0; i < surface->getNumberOfPanels(); i++) {
            for (int j = 0; j < view_data[k].cols(); j++) {
                if (j > 0)
                    f << ' ';

                f << view_data[k](i, j);
            }

            f << std::endl;
        }

        f << std::endl;
    }

    f.close();

    // Done:
    return true;
}
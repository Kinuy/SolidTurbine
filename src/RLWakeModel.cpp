#include "RLWakeModel.h"
#include "LiftingSurfaceParameters.h"



void RLWakeModel::addLayer()
{
    // Add layer:
    this->Wake::addLayer();

    // Add R-L data:
    if (getNumberOfPanels() >= liftingSurface->getSpanwisePanelsCount()) {
        for (int k = 0; k < liftingSurface->getSpanwisePanelsCount(); k++) {
            int panel = getNumberOfPanels() - liftingSurface->getSpanwisePanelsCount() + k;

            // Add initial vortex core radii. 
            std::vector<double> panel_vortex_core_radii;
            for (int i = 0; i < 4; i++)
                panel_vortex_core_radii.push_back(LiftingSurfaceParameters::rlInitialVortexCoreRadius);
            vortexCoreRadii.push_back(panel_vortex_core_radii);

            // Store base edge lengths.
            std::vector<double> edge_lengths;
            for (int i = 0; i < 4; i++) {
                int prev_idx;
                if (i == 0)
                    prev_idx = 3;
                else
                    prev_idx = i - 1;

                const Eigen::Vector3d& node_a = nodes[panelNodes[panel][prev_idx]];
                const Eigen::Vector3d& node_b = nodes[panelNodes[panel][i]];

                Eigen::Vector3d edge = node_b - node_a;
                edge_lengths.push_back(edge.norm());
            }
            baseEdgeLengths.push_back(edge_lengths);
        }
    }
}

Eigen::Vector3d RLWakeModel::vortexRingUnitVelocity(const Eigen::Vector3d& x, int panel) const
{
    if (panel >= getNumberOfPanels() - liftingSurface->getSpanwisePanelsCount()) {
        // This panel is contained in the latest row of wake panels.  To satisfy the Kutta condition
        // exactly, we use the unmodified vortex ring unit velocity here.
        return this->Surface::vortexRingUnitVelocity(x, panel);
    }

    // Compute vortex Reynolds number:                          
    double vortex_reynolds_number = doubletCoefficients[panel] / LiftingSurfaceParameters::rlFluidKinematicViscosity;

    // Interpolate Ramasamy-Leishman series values piecewise-linearly:
    int less_than_idx;
    for (less_than_idx = 0; less_than_idx < 12; less_than_idx++) {
        LiftingSurfaceParameters::ramasamy_leishman_data_row& row = LiftingSurfaceParameters::ramasamy_leishman_data[less_than_idx];

        if (vortex_reynolds_number < row.vortex_reynolds_number)
            break;
    }

    double a[3];
    double b[3];
    if (less_than_idx == 0) {
        a[0] = LiftingSurfaceParameters::ramasamy_leishman_data[0].a_1;
        a[1] = LiftingSurfaceParameters::ramasamy_leishman_data[0].a_2;

        b[0] = LiftingSurfaceParameters::ramasamy_leishman_data[0].b_1;
        b[1] = LiftingSurfaceParameters::ramasamy_leishman_data[0].b_2;
        b[2] = LiftingSurfaceParameters::ramasamy_leishman_data[0].b_3;
    }
    else if (less_than_idx == 12) {
        a[0] = LiftingSurfaceParameters::ramasamy_leishman_data[11].a_1;
        a[1] = LiftingSurfaceParameters::ramasamy_leishman_data[11].a_2;

        b[0] = LiftingSurfaceParameters::ramasamy_leishman_data[11].b_1;
        b[1] = LiftingSurfaceParameters::ramasamy_leishman_data[11].b_2;
        b[2] = LiftingSurfaceParameters::ramasamy_leishman_data[11].b_3;
    }
    else {
        double one_over_delta_vortex_reynolds_number =
            1.0 / (LiftingSurfaceParameters::ramasamy_leishman_data[less_than_idx].vortex_reynolds_number - LiftingSurfaceParameters::ramasamy_leishman_data[less_than_idx - 1].vortex_reynolds_number);
        double x = vortex_reynolds_number - LiftingSurfaceParameters::ramasamy_leishman_data[less_than_idx - 1].vortex_reynolds_number;
        double slope;

        slope = (LiftingSurfaceParameters::ramasamy_leishman_data[less_than_idx].a_1 - LiftingSurfaceParameters::ramasamy_leishman_data[less_than_idx - 1].a_1) * one_over_delta_vortex_reynolds_number;
        a[0] = LiftingSurfaceParameters::ramasamy_leishman_data[less_than_idx - 1].a_1 + slope * x;

        slope = (LiftingSurfaceParameters::ramasamy_leishman_data[less_than_idx].a_2 - LiftingSurfaceParameters::ramasamy_leishman_data[less_than_idx - 1].a_2) * one_over_delta_vortex_reynolds_number;
        a[1] = LiftingSurfaceParameters::ramasamy_leishman_data[less_than_idx - 1].a_2 + slope * x;

        slope = (LiftingSurfaceParameters::ramasamy_leishman_data[less_than_idx].b_1 - LiftingSurfaceParameters::ramasamy_leishman_data[less_than_idx - 1].b_1) * one_over_delta_vortex_reynolds_number;
        b[0] = LiftingSurfaceParameters::ramasamy_leishman_data[less_than_idx - 1].b_1 + slope * x;

        slope = (LiftingSurfaceParameters::ramasamy_leishman_data[less_than_idx].b_2 - LiftingSurfaceParameters::ramasamy_leishman_data[less_than_idx - 1].b_2) * one_over_delta_vortex_reynolds_number;
        b[1] = LiftingSurfaceParameters::ramasamy_leishman_data[less_than_idx - 1].b_2 + slope * x;

        slope = (LiftingSurfaceParameters::ramasamy_leishman_data[less_than_idx].b_3 - LiftingSurfaceParameters::ramasamy_leishman_data[less_than_idx - 1].b_3) * one_over_delta_vortex_reynolds_number;
        b[2] = LiftingSurfaceParameters::ramasamy_leishman_data[less_than_idx - 1].b_3 + slope * x;
    }

    a[2] = 1 - a[0] - a[1];

    // Compute velocity:
    Eigen::Vector3d velocity(0, 0, 0);

    for (int i = 0; i < (int)panelNodes[panel].size(); i++) {
        int previous_idx;
        if (i == 0)
            previous_idx = (int)panelNodes[panel].size() - 1;
        else
            previous_idx = i - 1;

        const Eigen::Vector3d& node_a = nodes[panelNodes[panel][previous_idx]];
        const Eigen::Vector3d& node_b = nodes[panelNodes[panel][i]];

        Eigen::Vector3d r_0 = node_b - node_a;
        Eigen::Vector3d r_1 = node_a - x;
        Eigen::Vector3d r_2 = node_b - x;

        double r_0_norm = r_0.norm();
        double r_1_norm = r_1.norm();
        double r_2_norm = r_2.norm();

        Eigen::Vector3d r_1xr_2 = r_1.cross(r_2);
        double r_1xr_2_sqnorm = r_1xr_2.squaredNorm();
        double r_1xr_2_norm = sqrt(r_1xr_2_sqnorm);

        if (r_0_norm < LiftingSurfaceParameters::zeroThreshold ||
            r_1_norm < LiftingSurfaceParameters::zeroThreshold ||
            r_2_norm < LiftingSurfaceParameters::zeroThreshold ||
            r_1xr_2_sqnorm < LiftingSurfaceParameters::zeroThreshold)
            continue;

        double d = r_1xr_2_norm / r_0_norm;

        double dr = pow(d / vortexCoreRadii[panel][i], 2);

        double sum = 0;
        for (int j = 0; j < 3; j++)
            sum += a[j] * exp(-b[j] * dr);

        velocity += (1 - sum) * r_1xr_2 / r_1xr_2_sqnorm * r_0.dot(r_1 / r_1_norm - r_2 / r_2_norm);
    }

    return MathUtility::getOneOver4Pi() * velocity;
}

void RLWakeModel::updateVortexRingRadii(int panel, double dt)
{
    for (int i = 0; i < 4; i++) {
        int prev_idx;
        if (i == 0)
            prev_idx = 3;
        else
            prev_idx = i - 1;

        double vortex_reynolds_number = fabs(doubletCoefficients[panel]) / LiftingSurfaceParameters::rlFluidKinematicViscosity;

        double t_multiplier = 4 * LiftingSurfaceParameters::rlLambsConstant *
            (1 + vortex_reynolds_number * LiftingSurfaceParameters::rlAPrime) *
            LiftingSurfaceParameters::rlFluidKinematicViscosity;

        double t = (pow(vortexCoreRadii[panel][i], 2) - pow(LiftingSurfaceParameters::rlInitialVortexCoreRadius, 2)) / t_multiplier;

        double vortex_core_size_0 = sqrt(pow(LiftingSurfaceParameters::rlInitialVortexCoreRadius, 2) + t_multiplier * (t + dt));

        Eigen::Vector3d node_a = nodes[panelNodes[panel][prev_idx]];
        Eigen::Vector3d node_b = nodes[panelNodes[panel][i]];

        Eigen::Vector3d edge = node_b - node_a;

        double strain = (edge.norm() - baseEdgeLengths[panel][i]) / baseEdgeLengths[panel][i];

        vortexCoreRadii[panel][i] = fmax(LiftingSurfaceParameters::rlMinVortexCoreRadius, vortex_core_size_0 / sqrt(1 + strain));
    }
}

void RLWakeModel::updateProperties(double dt)
{
    int i;

#pragma omp parallel
    {
#pragma omp for schedule(dynamic, 1)
        for (i = 0; i < getNumberOfPanels(); i++)
            updateVortexRingRadii(i, dt);
    }
}
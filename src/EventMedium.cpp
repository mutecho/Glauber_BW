#include "blastwave/EventMedium.h"

namespace blastwave {

  // Construct the event medium in two named stages. The current identity
  // evolution keeps initial and emission-stage density/geometry equal while
  // making the future expansion insertion point explicit.
  EventMedium buildEventMedium(const std::vector<WeightedTransversePoint> &points, const EventMediumParameters &parameters) {
    EventMedium medium;
    medium.participantPoints = points;
    medium.participantGeometry = computeFlowEllipseInfo(points);
    medium.initialDensity = buildGaussianPointCloudDensityField(points, parameters.densitySigma);

    switch (parameters.densityEvolutionMode) {
      case DensityEvolutionMode::None:
        medium.emissionDensity = medium.initialDensity;
        medium.emissionGeometry = medium.participantGeometry;
        break;
    }

    return medium;
  }

}  // namespace blastwave

#include "signal/effect_settings.h"

#include <iostream>
#include <limits>

namespace {

bool Check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
    }
    return condition;
}

} // namespace

int main() {
    using fpv::signal::ClampIntensity;
    using fpv::signal::EffectSettings;

    bool passed = true;
    const EffectSettings defaults{};

    passed &= Check(!defaults.masterEnabled, "master toggle defaults to off");
    passed &= Check(!defaults.analogNoiseEnabled, "analog noise defaults to off");
    passed &= Check(!defaults.horizontalBandsEnabled, "horizontal bands default to off");
    passed &= Check(!defaults.colorDistortionEnabled, "color distortion defaults to off");
    passed &= Check(!defaults.chromaticShiftEnabled, "chromatic shift defaults to off");

    passed &= Check(ClampIntensity(-0.2F) == 0.0F, "negative intensity clamps to zero");
    passed &= Check(ClampIntensity(0.4F) == 0.4F, "valid intensity remains unchanged");
    passed &= Check(ClampIntensity(1.2F) == 1.0F, "high intensity clamps to one");
    passed &= Check(
        ClampIntensity(std::numeric_limits<float>::quiet_NaN()) == 0.0F,
        "NaN intensity is normalized");

    if (!passed) {
        return 1;
    }

    std::cout << "All core tests passed.\n";
    return 0;
}

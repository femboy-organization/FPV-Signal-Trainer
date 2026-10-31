#pragma once

#include <algorithm>
#include <cmath>

namespace fpv::signal {

struct EffectSettings final {
    bool masterEnabled{false};

    bool analogNoiseEnabled{false};
    float analogNoiseIntensity{0.0F};

    bool horizontalBandsEnabled{false};
    float horizontalBandsIntensity{0.0F};

    bool colorDistortionEnabled{false};
    float colorDistortionIntensity{0.0F};

    bool chromaticShiftEnabled{false};
    float chromaticShiftIntensity{0.0F};
};

[[nodiscard]] inline float ClampIntensity(float value) noexcept {
    if (!std::isfinite(value)) {
        return 0.0F;
    }
    return std::clamp(value, 0.0F, 1.0F);
}

} // namespace fpv::signal

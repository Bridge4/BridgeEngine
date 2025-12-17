#include "OrbitCamera.h"
#define _USE_MATH_DEFINES
#include <math.h>

void OrbitCamera::RotateAzimuth(const float radians) {
    float targetAzimuth = radians + m_azimuthAngle;
    m_azimuthAngle = glm::mix(m_azimuthAngle, targetAzimuth, 0.1f);

    // Keep azimuth angle within range <0..2PI) - it's not necessary, just to
    // have it nicely output
    const auto fullCircle = 2.0f * M_PI;
    m_azimuthAngle = fmodf(m_azimuthAngle, fullCircle);
    if (m_azimuthAngle < 0.0f) {
        m_azimuthAngle = fullCircle + m_azimuthAngle;
    }
}

void OrbitCamera::RotatePolar(const float radians) {
    float targetPolar = radians + m_polarAngle;
    m_polarAngle = glm::mix(m_polarAngle, targetPolar, 0.1f);

    // Check if the angle hasn't exceeded quarter of a circle to prevent flip,
    // add a bit of epsilon like 0.001 radians
    const auto polarCap = glm::radians(89.0f);
    if (m_polarAngle > polarCap) {
        m_polarAngle = polarCap;
    }

    if (m_polarAngle < -polarCap) {
        m_polarAngle = -polarCap;
    }
}

void OrbitCamera::Zoom(const float by) {
    m_radius -= by;
    if (m_radius < m_minRadius) {
        m_radius = m_minRadius;
    }
}

glm::vec3 OrbitCamera::GetEye() const {
    // Calculate sines / cosines of angles
    const auto sineAzimuth = sin(m_azimuthAngle);
    const auto cosineAzimuth = cos(m_azimuthAngle);
    const auto sinePolar = sin(m_polarAngle);
    const auto cosinePolar = cos(m_polarAngle);

    // Calculate eye position out of them
    const auto x = m_radius * cosinePolar * cosineAzimuth;
    const auto y = m_radius * sinePolar;
    const auto z = m_radius * cosinePolar * sineAzimuth;

    return glm::vec3(x, y, z);
}

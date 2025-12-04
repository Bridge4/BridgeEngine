#ifndef ORBITCAMERA_H
#define ORBITCAMERA_H
// Source: https://www.mbsoftworks.sk/tutorials/opengl4/026-camera-pt3-orbit-camera/#:~:text=Orbit%20camera%20(also%20known%20as,just%20floating%20around%20the%20sphere.

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <../glm/glm.hpp>
class OrbitCamera
{
public:
	OrbitCamera(const glm::vec3& center, const glm::vec3& upVector, float radius, float minRadius, float azimuthAngle, float polarAngle) {
        m_center = center;
        m_upVector = upVector;
        m_radius = radius;
        m_minRadius = minRadius;
        m_azimuthAngle = azimuthAngle;
        m_polarAngle = polarAngle;
    };

    void RotateAzimuth(const float radians, float deltaTime);
    void RotatePolar(const float radians, float deltaTime);
	void Zoom(const float by);

	void moveHorizontal(const float distance);
	void moveVertical(const float distance);

    glm::mat4 getViewMatrix() const;
    glm::vec3 GetEye() const;
    glm::vec3 getViewPoint() const;
    glm::vec3 getUpVector() const;
    glm::vec3 getNormalizedViewVector() const;
    float getAzimuthAngle() const;
    float getPolarAngle() const;
    float getRadius() const;

private:
    glm::vec3 m_center; // Center of the orbit camera sphere (the point upon which the camera looks)
    glm::vec3 m_upVector; // Up vector of the camera
    float m_radius; // Radius of the orbit camera sphere
    float m_minRadius; // Minimal radius of the orbit camera sphere (cannot fall below this value)
    float m_azimuthAngle; // Azimuth angle on the orbit camera sphere
    float m_polarAngle; // Polar angle on the orbit camera sphere
};
#endif

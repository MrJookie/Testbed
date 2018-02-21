#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

namespace Testbed {
    class Camera {
        public:
            static constexpr float YAW = -90.0f;
            static constexpr float PITCH = 0.0f;
            static constexpr float SPEED = 20.0f;
            static constexpr float SENSITIVITY = 0.25f;
            static constexpr float ZOOM = 45.0f;
            
            enum MoveDirection {
                FORWARD,
                BACKWARD,
                LEFT,
                RIGHT
            };
    
            Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);
            Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);
            ~Camera();

            void ProcessKeyboard(MoveDirection direction, float dt);
            void ProcessMouseMovement(float offsetX, float offsetY, bool constrainPitch = true);
            void ProcessMouseScroll(float offsetY);
            void SetPosition(glm::vec3 position);
            float GetZoom() const;
            glm::mat4 GetViewMatrix() const;
            glm::vec3 GetPosition() const;
            void SetYaw(float yaw);
			void SetPitch(float pitch);
			float GetYaw() const;
			float GetPitch() const;
            void ToggleLockY();
            void updateCameraVectors();
             
            void ExtractFrustumPlanes(glm::mat4& view, glm::mat4& projection);
            bool PointInFrustum(glm::vec3& center);
            bool SphereInFrustum(glm::vec3& center, float radius);
            //bool CubeInFrustum(float x, float y, float z, float size);
            bool AABBIntersectsFrustum(glm::vec3& mins, glm::vec3& maxs);
            void PrintFrustumVerticesPositions();
            void DrawFrustum(glm::mat4 model, glm::mat4 view, glm::mat4 projection, GLuint shader);
            
            glm::vec3 m_frustum_vertices[8];
            
        private:
            glm::vec3 threePlanesIntersectionPoint(glm::vec4 a, glm::vec4 b, glm::vec4 c);  
            
            glm::vec3 m_position;
            glm::vec3 m_front;
            glm::vec3 m_up;
            glm::vec3 m_right;
            glm::vec3 m_world_up;
            
            float m_yaw;
            float m_pitch;
            float m_movement_speed;
            float m_mouse_sensitivity;
            float m_zoom;
            
            bool m_lock_y;
            
            glm::vec4 m_frustum_planes[6];
    };  
}

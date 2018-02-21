//opengl
#include <GL/glew.h>

//sdl
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

//glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/type_ptr.hpp>

//stl
#include <unordered_map>
#include <chrono>

//common
#include "Asset.hpp"
#include "Camera.hpp"

#include "Assimp_Model_Animator/Model.hpp"

#include <BulletCollision/CollisionShapes/btBox2dShape.h>


namespace tb = Testbed;

class App {
    public:
        App();
        ~App();
        
		void main_loop();
		void cleanup();
    private:
		SDL_Window* m_window;
		SDL_GLContext m_glContext;
		
		tb::Camera m_camera;
		tb::Asset m_asset;
		
        void init();
        void loop(); 
        void showFPS();
        int getSizeX() const;
        int getSizeY() const;
        void setSizeX(int sizeX);
        void setSizeY(int sizeY);
        double getDeltaTime() const;
        double getTimeElapsed() const;
        
        bool m_toggleMouseRelative;
        
        void process_input();
        
        int m_sizeX;
        int m_sizeY;
        
        int m_ticks_previous;
        int m_ticks_current;
        int m_frames_current;
        int m_frames_elapsed;

        int m_ticks_then;
        double m_delta_time;
        
        int m_skipMouseResolution;
        double m_mouseScroll;
        bool m_toggleWireframe;
        bool m_toggleFullscreen;
        bool m_running;
        
        std::chrono::high_resolution_clock::time_point m_chrono_start;
        double m_chrono_elapsed;
};  

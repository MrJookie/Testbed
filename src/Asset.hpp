#ifndef ASSET_HPP
#define ASSET_HPP

//opengl
#define GLEW_STATIC
#include <GL/glew.h>

//sdl
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

//stl
#include <unordered_map>
#include <set>
#include <vector>

//glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define SHADER_PATH "../Assets/Shaders/"
#define TEXTURE_PATH "../Assets/Textures/"
#define MUSIC_PATH "../Assets/Music/"
#define SOUND_PATH "../Assets/Sound/"

namespace Testbed {
	class Asset {
		public:
			Asset();
			~Asset();
			
			struct Texture {
				std::string fileName;
				GLuint id;
				glm::vec2 size;
				//SDL_Surface* image;
			};
			
			struct Shader {
				std::string fileName;
				GLuint id;
			};
			
			void LoadTexture(std::string fileName, bool isModelTexture);
			Texture GetTexture(std::string fileName, bool isModelTexture);
			
			void LoadShader(std::string shaderName, std::string vertexShaderFile, std::string fragmentShaderFile, std::string geometryShaderFile = "");
			Shader GetShader(std::string fileName);
			void UseShader(std::string fileName);
			void UnuseShader();
			
			Mix_Music* GetMusic(std::string fileName);
			Mix_Chunk* GetSound(std::string fileName);
			
			void FreeAssets();

		private:
			GLuint readShader(std::string shaderFile, GLenum shaderType);
			
			std::unordered_map<std::string, Texture> m_textures;
			std::unordered_map<std::string, Shader> m_shaders;
			std::unordered_map<std::string, Mix_Music*> m_musics;
			std::unordered_map<std::string, Mix_Chunk*> m_sounds;
	};
}

#endif

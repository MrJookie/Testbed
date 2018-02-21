#include <GL/glew.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>

//remove sdl and move sdl surface image to shaders?
#include <SDL2/SDL_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//bullet physics
#include "BulletGLDebugger.hpp"
#include <memory> //smart ptr
#include <btBulletDynamicsCommon.h>
#include "BulletCollision/CollisionShapes/btShapeHull.h"

#define MAX_BONES 64
#define MAX_BONES_PER_VERTEX 4

/* TODO:
 * - move texture loading logic to Asset (resource) manager, because now textures are being kept by model only, not by scene.
 */

namespace Helix {
	class Mesh {
        public:
            struct Vertex {
                glm::vec3 Position;
                glm::vec3 Normal;
                glm::vec2 TexCoords;
                //glm::vec3 Color;
                
                int boneIDs[MAX_BONES_PER_VERTEX] = {0};
				float boneWeights[MAX_BONES_PER_VERTEX] = {0.0};
            };
            
            //move textures to asset manager
            struct Texture {
                GLuint id;
                std::string type;
                aiString path; //ugly
            };
            
            Mesh();
            Mesh(std::string meshName, std::vector<Vertex>& vertices, std::vector<GLuint>& indices, std::vector<Texture>& textures);
            ~Mesh();
            
            void Draw(GLuint shader);
            
            std::string m_meshName;
            std::vector<Vertex> m_vertices;
            std::vector<GLuint> m_indices;
            std::vector<Texture> m_textures;
            
        private:
            void setupMesh();
            
            GLuint m_VAO, m_VBO, m_EBO;
    };
    
    class Model {
        public:
		    //bone info per vertex
            struct VertexBoneData {
				int boneIDs[MAX_BONES_PER_VERTEX] = {0};
				float boneWeights[MAX_BONES_PER_VERTEX] = {0.0};

				void add(int boneID, float weight) {
					for(int i = 0; i < MAX_BONES_PER_VERTEX; ++i) {
						if(boneWeights[i] == 0.0f) {
							boneIDs[i] = boneID;
							boneWeights[i] = weight;
							
							return;
						}
					}
				}
			};

			struct Bone {
				std::string name;
				aiMatrix4x4 offset;
				aiMatrix4x4 finalTransformation;

				Bone() {
					offset = aiMatrix4x4();
					finalTransformation = aiMatrix4x4();
				};
			};
				
            Model();
            Model(std::string fileName);
            ~Model();
            
            Mesh* GetMesh(std::string meshName);
            //bulletphysics
            btConvexShape* GetCollisionTriangleShape();
            btConvexHullShape* GetCollisionHullShape(bool advancedModel = false);
            //~bulletphysics
						
			void SetAnimIndex(int animIndex);
			void SetAnimPlay(bool play);
			void SetAnimLoop(bool loop);
			void SetAnimSpeed(double speed);

			void Update(double dt);
			double TimeToFrame(double time);
			double FrameToTime(double frame);
			glm::mat4 GetBoneMatrix(std::string name);
						
			void Draw(GLuint shader, glm::mat4 model, glm::mat4 view, glm::mat4 projection, double dt, glm::mat4 lightModelMat = glm::mat4());
			void DrawSkeletonBones(GLuint shader, glm::mat4 model, glm::mat4 view, glm::mat4 projection);
			void DrawSkeletonJoints(GLuint shader, glm::mat4 model, glm::mat4 view, glm::mat4 projection);

        private:
            void loadModel(std::string path);
            void processNode(aiNode* node);
            Mesh processMesh(aiMesh* mesh);
            std::vector<Mesh::Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
            GLuint textureFromFile(std::string path);
        
            std::vector<Mesh::Texture> m_texturesLoaded; 
            std::vector<Mesh> m_meshes;
            
            Assimp::Importer m_importer;
            const aiScene* m_scene;
            
            std::string m_directory;
            
            //skinning
            int m_modelVertexCount;
            
            bool m_hasAnimations;
			bool m_animPlay;
			bool m_animLoop;
			float m_animSpeed;
			float m_animTime;
			float m_animStartTime;
			float m_animEndTime;
			bool m_drawingSkeletonBones;
            aiMatrix4x4t<float> m_GlobalInverseTransform;
            
            aiMatrix4x4 interpolateTranslation(float time, const aiNodeAnim* nodeAnim);
			aiMatrix4x4 interpolateRotation(float time, const aiNodeAnim* nodeAnim);
			aiMatrix4x4 interpolateScale(float time, const aiNodeAnim* nodeAnim);
			void updateHierarchy(float animTime, const aiNode* node, const aiMatrix4x4& parentTransformation);
            
            std::vector<glm::vec4> m_skeletonBonesVerts;
            
			std::map<std::string, int> m_boneMapping;
			std::vector<Bone> m_bones;
			std::vector<VertexBoneData> m_vertexBoneData;
			std::unordered_map<std::string, aiNodeAnim*> m_channels;
			aiAnimation* m_activeAnimation;
			
			//bulletphysics
			std::unique_ptr<btConvexShape> m_collisionTriangleShape;
			std::shared_ptr<btConvexHullShape> m_collisionHullShape;
			std::unique_ptr<btTriangleIndexVertexArray> m_collisionShapeIndexedVertexArray;
			//~bulletphysics
    };
}

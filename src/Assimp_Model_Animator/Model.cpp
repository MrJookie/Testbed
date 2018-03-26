#include "Model.hpp"

namespace Helix {
	Mesh::Mesh() {}
    
    Mesh::Mesh(std::string meshName, std::vector<Vertex>& vertices, std::vector<GLuint>& indices, std::vector<Texture>& textures) {
		m_meshName = meshName;
        m_vertices = vertices;
        m_indices = indices;
        m_textures = textures;

        this->setupMesh();
    }
    
    Mesh::~Mesh() {}
    
    void Mesh::Draw(GLuint shader) {
        GLuint diffuseNr = 1;
        GLuint specularNr = 1;
        
        for(int i = 0; i < m_textures.size(); i++) {
            //N is texture_diffuseN
            std::stringstream ss;
            std::string number;
            std::string name = m_textures[i].type;
            if(name == "texture_diffuse") {
                ss << diffuseNr++;
            } else if(name == "texture_specular") {
                ss << specularNr++;
            }
            number = ss.str();
            
            glActiveTexture(GL_TEXTURE0 + i);
            glUniform1i(glGetUniformLocation(shader, (name + number).c_str()), i);
            glBindTexture(GL_TEXTURE_2D, m_textures[i].id);
        }
        
        glBindVertexArray(m_VAO);
        
        /*
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_STATIC_DRAW); //update only if verts were manipulated
        */

		glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        for(int i = 0; i < m_textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
    
    void Mesh::setupMesh() {
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_EBO);

        glBindVertexArray(m_VAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_STATIC_DRAW); //struct sequential memory layout

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(GLuint), &m_indices[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);   
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Position));

        glEnableVertexAttribArray(1);   
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));

        glEnableVertexAttribArray(2);   
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, TexCoords));
       
        glEnableVertexAttribArray(3);   
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, boneIDs));
        
        glEnableVertexAttribArray(4);   
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, boneWeights));
        
        glBindVertexArray(0);
    }
    
    Model::Model() {}
    
    Model::Model(std::string fileName) {
		m_modelVertexCount = 0;
		m_drawingSkeletonBones = false;
		m_hasAnimations = false;
		m_animPlay = true;
		m_animLoop = false;
		m_animSpeed = 1.0;
		m_animTime = 0;
		m_animStartTime = 0;
		m_animEndTime = 0;
		
        this->loadModel(fileName);
    }
    
    Model::~Model() {}
    
    void Model::loadModel(std::string path) {
		m_scene = m_importer.ReadFile(path.c_str(), 
					aiProcess_ImproveCacheLocality |
					aiProcess_Triangulate |
					aiProcess_OptimizeMeshes |
					aiProcess_JoinIdenticalVertices |
				  //  aiProcess_SplitLargeMeshes |
					//aiProcess_PreTransformVertices |
					aiProcess_LimitBoneWeights |
				  //  aiProcess_GenNormals |
					aiProcess_GenSmoothNormals |
					aiProcess_FlipUVs
			);


        if(!m_scene || m_scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !m_scene->mRootNode) {
            throw std::string("Assimp error: ") + m_importer.GetErrorString();
        }

        m_directory = path.substr(0, path.find_last_of('/'));
        
        m_GlobalInverseTransform = m_scene->mRootNode->mTransformation;
        m_GlobalInverseTransform.Inverse();
        
        if(m_scene->HasAnimations()) {
			m_hasAnimations = true;
			this->SetAnimIndex(0);
			
			for(int i = 0; i < m_activeAnimation->mNumChannels; ++i) {
				aiNodeAnim* nodeAnim = m_activeAnimation->mChannels[i];
				std::string nodeName = std::string(nodeAnim->mNodeName.data);
				
				if(m_channels.count(nodeName) == 0) {
					m_channels[std::string(nodeAnim->mNodeName.data)] = nodeAnim;
				}
			}
		} else {
			m_hasAnimations = false;
		}
        
        this->processNode(m_scene->mRootNode);
    }
    
    void Model::processNode(aiNode* node) {
        for(int i = 0; i < node->mNumMeshes; ++i) {
            aiMesh* mesh = m_scene->mMeshes[node->mMeshes[i]]; 
            m_meshes.push_back(this->processMesh(mesh));         
        }

        for(int i = 0; i < node->mNumChildren; ++i) {
            this->processNode(node->mChildren[i]);
        }
    }
    
    Mesh Model::processMesh(aiMesh* mesh) {
        std::vector<Mesh::Vertex> vertices;
        std::vector<GLuint> indices;
        std::vector<Mesh::Texture> textures;
        
        if(!mesh->HasNormals()) {
            throw std::string("Loading model failed! Missing normals. Try adding flag: aiProcess_GenNormals or aiProcess_GenSmoothNormals");
        }
        
        //bones (must be first!)
		if(mesh->HasBones()) {
			m_vertexBoneData.resize(m_vertexBoneData.size() + mesh->mNumVertices);
			
			for(int i = 0; i < mesh->mNumBones; ++i) {
				int index = 0;

				assert(mesh->mNumBones <= MAX_BONES);

				std::string boneName(mesh->mBones[i]->mName.data);
				
				if(m_boneMapping.find(boneName) == m_boneMapping.end()) { //bone does not exist yet, add new one
					index = m_bones.size();
					m_boneMapping[boneName] = index;
					
					Bone bone;
					bone.name = boneName;
					bone.offset = mesh->mBones[i]->mOffsetMatrix;
					m_bones.push_back(bone);
					
					//std::cout << "bone name: " << boneName << std::endl;
				} else {
					index = m_boneMapping[boneName];
				}
				
				for(int j = 0; j < mesh->mBones[i]->mNumWeights; ++j) {
					int vertexID = m_modelVertexCount + mesh->mBones[i]->mWeights[j].mVertexId;
					m_vertexBoneData[vertexID].add(index, mesh->mBones[i]->mWeights[j].mWeight);
					
					//std::cout << "vertexID: " << vertexID << " weight: " << mesh->mBones[i]->mWeights[j].mWeight << std::endl;
				}
			}
		}
		
		//vertices
        for(int i = 0; i < mesh->mNumVertices; ++i) {
            Mesh::Vertex vertex;
            glm::vec3 vector;

            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;

            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;

            if(mesh->HasTextureCoords(0)) {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x; 
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }
            
            //vertex.Color = (mesh->HasVertexColors(0)) ? glm::make_vec3(&mesh->mColors[0][i].r) : glm::vec3(1.0f);
            
            if(mesh->HasBones()) {
				for(int j = 0; j < MAX_BONES_PER_VERTEX; ++j) {
					vertex.boneWeights[j] = m_vertexBoneData[m_modelVertexCount + i].boneWeights[j];
					vertex.boneIDs[j] = m_vertexBoneData[m_modelVertexCount + i].boneIDs[j];
				}
			}

            vertices.push_back(vertex);
        }
        
        m_modelVertexCount += mesh->mNumVertices;
        
		//indices
        for(int i = 0; i < mesh->mNumFaces; ++i) {
            aiFace face = mesh->mFaces[i];

            for(int j = 0; j < face.mNumIndices; ++j) {
                indices.push_back(face.mIndices[j]);
            }
        }

		//materials
        if(mesh->mMaterialIndex >= 0) {
            aiMaterial* material = m_scene->mMaterials[mesh->mMaterialIndex];

			std::vector<Mesh::Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

			std::vector<Mesh::Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        }
        
        //if scene is missing textures
        if(!m_scene->HasTextures()) {
        	aiString str;
			
			Mesh::Texture texture;
			texture.id = textureFromFile("missing_texture.jpg");
			texture.type = aiTextureType_DIFFUSE;
			texture.path = str;
			
			textures.push_back(texture);
			m_texturesLoaded.push_back(texture);
		}
		
		std::string meshName(mesh->mName.data);
        
        return Mesh(meshName, vertices, indices, textures);
    }
    
    //pass asset/resource manager ptr here with all the textures
    std::vector<Mesh::Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName) {
        std::vector<Mesh::Texture> textures;

		for(int i = 0; i < mat->GetTextureCount(type); ++i) {
			aiString str;
			mat->GetTexture(type, i, &str);
			bool skip = false;
			
			for(int j = 0; j < m_texturesLoaded.size(); ++j) {
				if(m_texturesLoaded[j].path == str) {
					textures.push_back(m_texturesLoaded[j]);
					skip = true; 
					break;
				}
			}
			
			//load texture if not loaded yet
			if(!skip) {
				Mesh::Texture texture;
				texture.id = textureFromFile(str.C_Str());
				texture.type = typeName;
				texture.path = str;
				
				//std::cout << "texture path: " << std::string(str.C_Str()) << std::endl;
				
				textures.push_back(texture);
				m_texturesLoaded.push_back(texture);
			}
		}
        
        return textures;
    }
    
    GLuint Model::textureFromFile(std::string path) {
        std::string filename = m_directory + '/' + path;
 
        SDL_Surface* image = IMG_Load(filename.c_str());
        if(!image) {
            throw std::string("Error loading image: ") + IMG_GetError();
        }
        
        GLint colorMode;
        if(image->format->BytesPerPixel == 4) {
            if(image->format->Rmask == 0x000000ff) {
                colorMode = GL_RGBA;
            } else {
                colorMode = GL_BGRA;
            }
        } else if(image->format->BytesPerPixel == 3) {
            if(image->format->Rmask == 0x000000ff) {
                colorMode = GL_RGB;
            } else {
                colorMode = GL_BGR;
            }
        } else {
             throw std::string("Image is not truecolor!");
        }
        
        GLuint textureID;
        glGenTextures(1, &textureID);
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 0, colorMode, GL_UNSIGNED_BYTE, image->pixels);
            
        glGenerateMipmap(GL_TEXTURE_2D);   
         
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        glBindTexture(GL_TEXTURE_2D, 0);
        
        SDL_FreeSurface(image);
        
        return textureID;
    }

    double Model::TimeToFrame(double time) {
		 //float inverseTicksPerSecond = 1.0 / (float)(scene->mAnimations[0]->mTicksPerSecond != 0 ? scene->mAnimations[0]->mTicksPerSecond : 25.0f);
		 double inverseTicksPerSecond = 1.0 / 24.0f;
		 return time / inverseTicksPerSecond;
	}

	double Model::FrameToTime(double frame) {
		 double inverseTicksPerSecond = 1.0 / 24.0f;
		 return frame * inverseTicksPerSecond;
	}
	
	void Model::SetAnimIndex(int animIndex) {
		assert(animIndex < m_scene->mNumAnimations);
		
		m_activeAnimation = m_scene->mAnimations[animIndex];
		m_animEndTime = m_activeAnimation->mDuration - 0.0001;
	}
	
	void Model::SetAnimPlay(bool play) {
		m_animPlay = play;
	}
	
	void Model::SetAnimLoop(bool loop) {
		m_animLoop = loop;
	}
	
	void Model::SetAnimSpeed(double speed) {
		m_animSpeed = speed;
	}
	
	//recursive bone transformation for given animation time
	void Model::Update(double dt) {
		if(m_hasAnimations && m_animPlay) {
			m_animTime += dt * m_animSpeed;
			
			if(m_animTime < m_animStartTime) {
				m_animTime = m_animStartTime;
			}
			
			if(m_animTime > m_animEndTime) {
				if(m_animLoop) {
					m_animTime = m_animStartTime;
				} else { //stop animation
					m_animTime = m_animEndTime;
				}
			}

			aiMatrix4x4 identity = aiMatrix4x4();
			updateHierarchy(m_animTime, m_scene->mRootNode, identity);
		}
	}
	   
    Mesh* Model::GetMesh(std::string meshName) {
		for(int i = 0; i < m_meshes.size(); ++i) {
			if(m_meshes[i].m_meshName == meshName) {
				return &m_meshes[i];
			}
		}
		
		return nullptr;
	}
	
	glm::mat4 Model::GetBoneMatrix(std::string name) {
		for(int i = 0; i < m_bones.size(); ++i) {
			if(m_bones[i].name == name) {
				aiMatrix4x4 offset = m_bones[i].offset;
				aiMatrix4x4 transformation = m_bones[i].finalTransformation * offset.Inverse();
				
				return glm::transpose(glm::make_mat4(&transformation.a1));
			}
		}
		
		return glm::mat4();
	}

	aiMatrix4x4 Model::interpolateTranslation(float time, const aiNodeAnim* nodeAnim) {
		aiVector3D translation;

		if(nodeAnim->mNumPositionKeys == 1) {
			translation = nodeAnim->mPositionKeys[0].mValue;
		} else {
			int frameIndex = 0;
			for(int i = 0; i < nodeAnim->mNumPositionKeys - 1; i++) {
				if(time < (float)nodeAnim->mPositionKeys[i + 1].mTime) {
					frameIndex = i;
					break;
				}
			}

			aiVectorKey currentFrame = nodeAnim->mPositionKeys[frameIndex];
			aiVectorKey nextFrame = nodeAnim->mPositionKeys[(frameIndex + 1) % nodeAnim->mNumPositionKeys];

			float delta = (time - (float)currentFrame.mTime) / (float)(nextFrame.mTime - currentFrame.mTime);

			const aiVector3D& start = currentFrame.mValue;
			const aiVector3D& end = nextFrame.mValue;

			translation = (start + delta * (end - start));
		}

		aiMatrix4x4 mat;
		aiMatrix4x4::Translation(translation, mat);
		return mat;
	}

	aiMatrix4x4 Model::interpolateRotation(float time, const aiNodeAnim* nodeAnim) {
		aiQuaternion rotation;

		if(nodeAnim->mNumRotationKeys == 1) {
			rotation = nodeAnim->mRotationKeys[0].mValue;
		} else {
			int frameIndex = 0;
			for(int i = 0; i < nodeAnim->mNumRotationKeys - 1; i++) {
				if(time < (float)nodeAnim->mRotationKeys[i + 1].mTime) {
					frameIndex = i;
					break;
				}
			}

			aiQuatKey currentFrame = nodeAnim->mRotationKeys[frameIndex];
			aiQuatKey nextFrame = nodeAnim->mRotationKeys[(frameIndex + 1) % nodeAnim->mNumRotationKeys];

			float delta = (time - (float)currentFrame.mTime) / (float)(nextFrame.mTime - currentFrame.mTime);

			const aiQuaternion& start = currentFrame.mValue;
			const aiQuaternion& end = nextFrame.mValue;

			aiQuaternion::Interpolate(rotation, start, end, delta);
			rotation.Normalize();
		}

		aiMatrix4x4 mat(rotation.GetMatrix());
		return mat;
	}

	aiMatrix4x4 Model::interpolateScale(float time, const aiNodeAnim* nodeAnim) {
		aiVector3D scale;

		if(nodeAnim->mNumScalingKeys == 1) {
			scale = nodeAnim->mScalingKeys[0].mValue;
		} else {
			int frameIndex = 0;
			for(int i = 0; i < nodeAnim->mNumScalingKeys - 1; i++) {
				if(time < (float)nodeAnim->mScalingKeys[i + 1].mTime) {
					frameIndex = i;
					break;
				}
			}

			aiVectorKey currentFrame = nodeAnim->mScalingKeys[frameIndex];
			aiVectorKey nextFrame = nodeAnim->mScalingKeys[(frameIndex + 1) % nodeAnim->mNumScalingKeys];

			float delta = (time - (float)currentFrame.mTime) / (float)(nextFrame.mTime - currentFrame.mTime);

			const aiVector3D& start = currentFrame.mValue;
			const aiVector3D& end = nextFrame.mValue;

			scale = (start + delta * (end - start));
		}

		aiMatrix4x4 mat;
		aiMatrix4x4::Scaling(scale, mat);
		return mat;
	}

	void Model::updateHierarchy(float animTime, const aiNode* node, const aiMatrix4x4& parentTransformation) {
		std::string nodeName(node->mName.data);
		
		aiMatrix4x4 nodeTransformation = aiMatrix4x4();

		const aiNodeAnim* nodeAnim = m_channels[nodeName];
		if(nodeAnim) {
			//interpolate matrices between current and next frame
			aiMatrix4x4 matScale = interpolateScale(animTime, nodeAnim);
			aiMatrix4x4 matRotation = interpolateRotation(animTime, nodeAnim);
			aiMatrix4x4 matTranslation = interpolateTranslation(animTime, nodeAnim);

			nodeTransformation = matTranslation * matRotation * matScale;
		} else {
			nodeTransformation = aiMatrix4x4(node->mTransformation);
		}

		aiMatrix4x4 globalTransformation = parentTransformation * nodeTransformation;

		if(m_boneMapping.find(nodeName) != m_boneMapping.end()) {
			int boneIndex = m_boneMapping[nodeName];
			m_bones[boneIndex].finalTransformation = m_GlobalInverseTransform * globalTransformation * m_bones[boneIndex].offset;
			
			//draw skeleton
			if(this->m_drawingSkeletonBones) {
				if(node->mParent != nullptr) {
					std::string nodeNameParent(node->mParent->mName.data);
					
					if(m_boneMapping.find(nodeNameParent) != m_boneMapping.end()) {
						int boneIndexParent = m_boneMapping[nodeNameParent];
						
						aiMatrix4x4 invParent = m_bones[boneIndexParent].offset;
						invParent = m_bones[boneIndexParent].finalTransformation * invParent.Inverse();
						
						aiMatrix4x4 invCurrent = m_bones[boneIndex].offset;
						invCurrent = m_bones[boneIndex].finalTransformation * invCurrent.Inverse();
						
						//beginning - parent node
						m_skeletonBonesVerts.push_back( glm::vec4( glm::transpose(glm::make_mat4(&invParent.a1)) * glm::vec4(0.0, 0.0, 0.0, 1.0)) );
						
						//end - current node
						m_skeletonBonesVerts.push_back( glm::vec4( glm::transpose(glm::make_mat4(&invCurrent.a1)) * glm::vec4(0.0, 0.0, 0.0, 1.0)) );
					}
				}
			}
		}	

		for(int i = 0; i < node->mNumChildren; i++) {
			updateHierarchy(animTime, node->mChildren[i], globalTransformation);
		}
	}
	 
    void Model::Draw(GLuint shader, glm::mat4 model, glm::mat4 view, glm::mat4 projection, double dt, glm::mat4 lightModelMat, glm::vec3 viewPos) {
		glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        
		glUseProgram(shader);
		
		//light
		float moveX = sin(dt * 2) * 1.8f;
        float moveY = cos(dt * 2) * 1.8f;
        
        glm::vec3 lightPos = glm::vec3(lightModelMat[3]);
        
        //glm::vec3 lightPos(2.7f * moveX, 0.2f, 2.0f * moveY);
        glUniform3f(glGetUniformLocation(shader, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(glGetUniformLocation(shader, "viewPos"), viewPos.x, viewPos.y, viewPos.z);
		//~light
		
		glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		
		if(m_hasAnimations) {
			glm::mat4 bones[MAX_BONES];
			for(int i = 0; i < m_bones.size(); ++i) {
				aiMatrix4x4 finalTransformation = m_bones[i].finalTransformation;
				
				bones[i] = glm::transpose(glm::make_mat4(&finalTransformation.a1));
			}
			
			glUniformMatrix4fv(glGetUniformLocation(shader, "boneTransformation"), sizeof(bones), GL_FALSE, glm::value_ptr(bones[0]));
		}
		
        for(int i = 0; i < m_meshes.size(); ++i) {
			m_meshes[i].Draw(shader);
        }
        
        glUseProgram(0);
        
        glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
    }
	
	void Model::DrawSkeletonBones(GLuint shader, glm::mat4 model, glm::mat4 view, glm::mat4 projection) {
		if(!m_hasAnimations) {
			return;
		}
		
		this->m_drawingSkeletonBones = true;
		
		if(m_skeletonBonesVerts.size() > 1) {
			for(int i = 0; i < m_skeletonBonesVerts.size(); i += 2) {
				glUseProgram(shader);
				glEnable(GL_LINE_SMOOTH);
				glLineWidth(2.0);

				GLuint vao_0, vbo_0, vbo_1;
				glGenVertexArrays(1, &vao_0);
				glBindVertexArray(vao_0);
				glGenBuffers(1, &vbo_0);
				
				glBindBuffer(GL_ARRAY_BUFFER, vbo_0);
				glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * 2, &m_skeletonBonesVerts[i], GL_STATIC_DRAW);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
				
				glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
				glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
				
				glDrawArrays(GL_LINES, 0, 2);
				
				glBindVertexArray(0);

				glDeleteVertexArrays(1, &vao_0);
				glDeleteBuffers(1, &vbo_0);

				glLineWidth(1.0);
				glDisable(GL_LINE_SMOOTH);
				glUseProgram(0);
			}
		}
		
		m_skeletonBonesVerts.clear();
	}
	
	void Model::DrawSkeletonJoints(GLuint shader, glm::mat4 model, glm::mat4 view, glm::mat4 projection) {
		if(!m_hasAnimations) {
			return;
		}
		
		glm::mat4 bones[MAX_BONES];
		
		for(int i = 0; i < m_bones.size(); ++i) {
			aiMatrix4x4 offset = m_bones[i].offset;
			aiMatrix4x4 finalTransformation = m_bones[i].finalTransformation * offset.Inverse();
			
			bones[i] = glm::transpose(glm::make_mat4(&finalTransformation.a1));
		}
			
		glUseProgram(shader);
		glEnable(GL_PROGRAM_POINT_SIZE);

		GLuint vao_0, vbo_0, vbo_1;
		glGenVertexArrays(1, &vao_0);
		glBindVertexArray(vao_0);
		glGenBuffers(1, &vbo_0);
		
		glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		
		glUniformMatrix4fv(glGetUniformLocation(shader, "boneTransformation"), sizeof(bones), GL_FALSE, glm::value_ptr(bones[0]));

		glDrawArrays(GL_POINTS, 0, m_bones.size());
		
		glBindVertexArray(0);

		glDeleteVertexArrays(1, &vao_0);
		glDeleteBuffers(1, &vbo_0);

		glDisable(GL_PROGRAM_POINT_SIZE);
		glUseProgram(0);
	}

	btConvexShape* Model::GetBulletTriangleShape() {
		if(!m_collisionTriangleShape) {
			m_collisionShapeIndexedVertexArray = std::make_unique<btTriangleIndexVertexArray>();

			for(const auto& mesh : m_meshes) {
				btIndexedMesh indexedMesh;

				indexedMesh.m_numTriangles = mesh.m_indices.size() / 3;
				indexedMesh.m_triangleIndexBase = (const unsigned char *)&mesh.m_indices[0];
				indexedMesh.m_triangleIndexStride = 3 * sizeof(GLuint);
				indexedMesh.m_indexType = PHY_INTEGER;

				indexedMesh.m_numVertices = mesh.m_vertices.size();
				indexedMesh.m_vertexBase = (const unsigned char *)(&mesh.m_vertices[0] + offsetof(Mesh::Vertex, Position));
				indexedMesh.m_vertexStride = sizeof(Mesh::Vertex);
				indexedMesh.m_vertexType = PHY_FLOAT;

				m_collisionShapeIndexedVertexArray->addIndexedMesh(indexedMesh);
			}
			
			m_collisionTriangleShape = std::make_unique<btConvexTriangleMeshShape>(m_collisionShapeIndexedVertexArray.get());
			//m_collisionTriangleShape = std::make_unique<btBvhTriangleMeshShape>(m_collisionShapeIndexedVertexArray.get(), true, true);
		}
		
		return m_collisionTriangleShape.get();
	}
	
	btConvexHullShape* Model::GetBulletConvexHullShape(bool advancedModel) {
		if(!m_collisionHullShape) {
			m_collisionHullShape = std::make_shared<btConvexHullShape>();

			for(int i = 0; i < m_meshes.size(); ++i) {
				for(int j = 0; j < m_meshes[i].m_vertices.size(); ++j) {
					glm::vec3 vertexPosition = m_meshes[i].m_vertices[j].Position;
					
					m_collisionHullShape->addPoint(btVector3(vertexPosition.x, vertexPosition.y, vertexPosition.z));
				}
			}
			
			if(advancedModel) {
				std::shared_ptr<btShapeHull> hull = std::make_shared<btShapeHull>(m_collisionHullShape.get());
				hull->buildHull(0.0f); //parameter is ignored by buildHull, reset margin?
				//hull->buildHull(m_collisionHullShape->getMargin());
				std::shared_ptr<btConvexHullShape> convexHullShape = std::make_shared<btConvexHullShape>((const btScalar*)hull->getVertexPointer(), hull->numVertices(), sizeof(btVector3));

				convexHullShape->optimizeConvexHull(); //removes inner unwanted verts
				convexHullShape->initializePolyhedralFeatures(); //makes rendering nicer, but doesn't affect collision detection at all
				
				m_collisionHullShape = convexHullShape;
			} else {
				m_collisionHullShape->optimizeConvexHull();
				m_collisionHullShape->initializePolyhedralFeatures();
			}
			
			/* Finally, if you really want, you can also enable the separating axis test (SAT) based collision detection.
			 * If enabled AND you generated polyhedral features, it will be used instead of GJK/EPA.
			 * To enable this, use world->getDispatchInfo().m_enableSatConvex=true;
			 */
		}
		
		return m_collisionHullShape.get();
	}
	
	std::vector<float> Model::GetModelVertices() {
		std::vector<float> vertices;
		
		for(int i = 0; i < m_meshes.size(); ++i) {
			for(int j = 0; j < m_meshes[i].m_vertices.size(); ++j) {
				glm::vec3 vertexPosition = m_meshes[i].m_vertices[j].Position;
				
				vertices.push_back(vertexPosition.x);
				vertices.push_back(vertexPosition.y);
				vertices.push_back(vertexPosition.z);
			}
		}
		
		return vertices;
	}
	
	std::vector<unsigned int> Model::GetModelIndices() {
		std::vector<unsigned int> indices;
		
		for(int i = 0; i < m_meshes.size(); ++i) {
			for(int j = 0; j < m_meshes[i].m_indices.size(); ++j) {
				indices.push_back(m_meshes[i].m_indices[j]);
			}
		}
		
		return indices;
	}
	
	btCompoundShape* Model::CreateShapeHACD() {
		std::vector<float> vertices = this->GetModelVertices();
		std::vector<unsigned int> indices = this->GetModelIndices();
		
		btVector3 centroid(0, 0, 0);
		btAlignedObjectArray<btVector3> convexHullCentroids;
		//int baseCount = 0;

		std::vector<HACD::Vec3<HACD::Real>> Points;
		std::vector<HACD::Vec3<long>> Triangles;

		for(int i = 0; i < vertices.size(); i += 3) {
			HACD::Vec3<HACD::Real> Vertex(vertices[i], vertices[i + 1], vertices[i + 2]);
			Points.push_back(Vertex);
		}
		
		for(int i = 0; i < indices.size(); i += 3) {
			HACD::Vec3<long> Triangle(indices[i], indices[i + 1], indices[i + 2]);
			Triangles.push_back(Triangle);
		}

		HACD::HACD myHACD; // http://kmamou.blogspot.cz/2011/11/hacd-parameters.html
		myHACD.SetPoints(&Points[0]);
		myHACD.SetNPoints(Points.size());
		myHACD.SetTriangles(&Triangles[0]);
		myHACD.SetNTriangles(Triangles.size());
		myHACD.SetCompacityWeight(0.1);
		myHACD.SetVolumeWeight(0.0);

		myHACD.SetNClusters(0);
		myHACD.SetNVerticesPerCH(100);
		myHACD.SetConcavity(200);
		myHACD.SetAddExtraDistPoints(false);
		myHACD.SetAddNeighboursDistPoints(false);
		myHACD.SetAddFacesPoints(false);

		myHACD.Compute();

		for(int c = 0; c < myHACD.GetNClusters(); ++c) {
			int numPoints = myHACD.GetNPointsCH(c);
			int numTriangles = myHACD.GetNTrianglesCH(c);

			float* Vertices = new float[numPoints * 3];
			//unsigned int* Triangles = new unsigned int[numTriangles * 3];

			HACD::Vec3<HACD::Real>* PointsCH = new HACD::Vec3<HACD::Real>[numPoints];
			HACD::Vec3<long>* TrianglesCH = new HACD::Vec3<long>[numTriangles];
			
			myHACD.GetCH(c, PointsCH, TrianglesCH);

			for(int v = 0; v < numPoints; v++) {
				Vertices[3 * v] = PointsCH[v].X();
				Vertices[3 * v + 1] = PointsCH[v].Y();
				Vertices[3 * v + 2] = PointsCH[v].Z();
			}
			
			/*
			for(int f = 0; f < numTriangles; f++) {
				Triangles[3 * f] = TrianglesCH[f].X();
				Triangles[3 * f + 1] = TrianglesCH[f].Y();
				Triangles[3 * f + 2] = TrianglesCH[f].Z();
			}
			*/

			delete[] PointsCH;
			delete[] TrianglesCH;

			centroid.setValue(0, 0, 0);

			btAlignedObjectArray<btVector3> h_Vertices;

			for(int i = 0; i < numPoints; ++i) {
				btVector3 Vertex(Vertices[i * 3], Vertices[i * 3 + 1], Vertices[i * 3 + 2]);
				centroid += Vertex;
			}

			centroid *= 1.0f / (float)numPoints;

			for(int i = 0; i < numPoints; ++i) {
				btVector3 Vertex(Vertices[i * 3], Vertices[i * 3 + 1], Vertices[i * 3 + 2]);
				Vertex -= centroid;
				h_Vertices.push_back(Vertex);
			}

			/*
			const unsigned int *src = Triangles;
			for(int i = 0; i < nTriangles; ++i) {
				unsigned int index0 = *src++;
				unsigned int index1 = *src++;
				unsigned int index2 = *src++;

				btVector3 vertex0(Vertices[index0 * 3], Vertices[index0 * 3 + 1], Vertices[index0 * 3 + 2]);
				btVector3 vertex1(Vertices[index1 * 3], Vertices[index1 * 3 + 1], Vertices[index1 * 3 + 2]);
				btVector3 vertex2(Vertices[index2 * 3], Vertices[index2 * 3 + 1], Vertices[index2 * 3 + 2]);

				vertex0 -= centroid;
				vertex1 -= centroid;
				vertex2 -= centroid;

				TriMesh->addTriangle(vertex0, vertex1, vertex2);

				index0 += baseCount;
				index1 += baseCount;
				index2 += baseCount;
			}
			*/
				
			std::shared_ptr<btConvexHullShape> convexHullShape = std::make_shared<btConvexHullShape>(&h_Vertices[0].getX(), h_Vertices.size());
			convexHullShape->setMargin(0.01f);
			
			convexHullShape->optimizeConvexHull(); //removes inner unwanted verts
			convexHullShape->initializePolyhedralFeatures(); //makes rendering nicer, but doesn't affect collision detection at all
			
			m_convexHullShapes.push_back(convexHullShape);
			convexHullCentroids.push_back(centroid);
			
			//baseCount += nPoints;
						
			delete[] Vertices;
			//delete[] Triangles;
		}
		
		m_compoundShape = std::make_unique<btCompoundShape>();
		
		btTransform transform;
		transform.setIdentity();

		for(int i = 0; i < m_convexHullShapes.size(); ++i) {
			transform.setOrigin(convexHullCentroids[i]);
			m_compoundShape->addChildShape(transform, m_convexHullShapes[i].get());
		}

		return m_compoundShape.get();
	}
	
	btCompoundShape* Model::GetBulletVHACDShape() {
		std::vector<float> vertices = this->GetModelVertices();
		std::vector<unsigned int> indices = this->GetModelIndices();
		
		btVector3 centroid(0, 0, 0);
		btAlignedObjectArray<btVector3> convexHullCentroids;
		
		VHACD::IVHACD::Parameters params; // http://kmamou.blogspot.cz/2014/12/v-hacd-20-parameters-description.html
		params.m_maxNumVerticesPerCH = 32;
		params.m_concavity = 0.0025;
		params.m_mode = 0;
		
		VHACD::IVHACD* interfaceVHACD = VHACD::CreateVHACD();

		bool res = interfaceVHACD->Compute(&vertices[0], (unsigned int)vertices.size() / 3, &indices[0], (unsigned int)indices.size() / 3, params);
		if(res) {
			VHACD::IVHACD::ConvexHull ch;

			for(int i = 0; i < interfaceVHACD->GetNConvexHulls(); ++i) {
				interfaceVHACD->GetConvexHull(i, ch);

				centroid.setValue(0, 0, 0);
				btAlignedObjectArray<btVector3> h_Vertices;

				for(int v = 0, idx = 0; v < ch.m_nPoints; ++v, idx += 3) {
					btVector3 Vertex(ch.m_points[idx], ch.m_points[idx+1], ch.m_points[idx+2]);
					//Centroid += Vertex - btVector3(0, 3.0, 0);
					h_Vertices.push_back(Vertex);
				}
				
				centroid *= 1.0f / (float)ch.m_nPoints;

				std::shared_ptr<btConvexHullShape> convexHullShape = std::make_shared<btConvexHullShape>(&h_Vertices[0].getX(), h_Vertices.size());
				convexHullShape->setMargin(0.01f);
				
				convexHullShape->optimizeConvexHull(); //removes inner unwanted verts
				convexHullShape->initializePolyhedralFeatures(); //makes rendering nicer, but doesn't affect collision detection at all
				
				m_convexHullShapes.push_back(convexHullShape);
				convexHullCentroids.push_back(centroid);      
			}
		}
		
		interfaceVHACD->Clean();
		interfaceVHACD->Release();
		
		m_compoundShape = std::make_unique<btCompoundShape>();
		
		btTransform transform;
		transform.setIdentity();
		
		for(int i = 0; i < m_convexHullShapes.size(); ++i) {
			transform.setOrigin(convexHullCentroids[i]);
			m_compoundShape->addChildShape(transform, m_convexHullShapes[i].get());
		}
		
		return m_compoundShape.get();
	}
}

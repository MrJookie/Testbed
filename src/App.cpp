#include "App.hpp"

//examples
int Init_Example_Skinning_Bullet_VHACD(Helix::Model& pyroModel);
void Render_Example_Skinning_Bullet_VHACD(Testbed::Camera& camera, Testbed::Asset& asset, double dt, double timeElapsed, glm::mat4 view, glm::mat4 projection, glm::vec3 viewPos, Helix::Model& bobModel, Helix::Model& bobModel2, Helix::Model& pyroModel, Helix::Model& chickenModel);

App::App() {
	m_sizeX = 800;
	m_sizeY = 600;

	m_ticks_previous = SDL_GetTicks();
	m_ticks_current = 0;
	m_frames_current = 0;
	m_frames_elapsed = 0;

	m_delta_time = 0;
	m_ticks_then = 0;

	m_chrono_start = std::chrono::high_resolution_clock::now();
	m_chrono_elapsed = 0;
	
	m_toggleMouseRelative = false;
	m_mouseScroll = 0.0;
	m_skipMouseResolution = 0;
	m_toggleWireframe = true;
    m_toggleFullscreen = true;
	m_running = true;

	this->init();
}

App::~App() {
	m_asset.FreeAssets();
    Mix_CloseAudio();
    Mix_Quit();	
}

void App::init() {
	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		throw std::string("Failed to initialize SDL: ") + SDL_GetError();
	}

	m_window = SDL_CreateWindow("Testbed", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, this->getSizeX(), this->getSizeY(), SDL_WINDOW_OPENGL);
	if(m_window == nullptr) {
		throw std::string("Failed to create window: ") + SDL_GetError();
	}
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	// MSAA
	// SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	// SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

	m_glContext = SDL_GL_CreateContext(m_window);
	if(m_glContext == nullptr) {
		throw std::string("Failed to create GLContext: ") + SDL_GetError();
	}

	SDL_GL_SetSwapInterval(0); //vsync
	SDL_GL_MakeCurrent(m_window, m_glContext);

	glewExperimental = GL_TRUE;
	GLenum glew = glewInit();
	if(GLEW_OK != glew) {
		throw std::string("Failed to initialize GLEW");
	}

	std::cout << "Vendor:     " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer:   " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version:    " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL:       " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1", SDL_HINT_OVERRIDE);
	// SDL_SetWindowGrab(m_window, SDL_TRUE);

	if(m_toggleMouseRelative) {
		SDL_SetRelativeMouseMode(SDL_TRUE);
	}

	m_camera = tb::Camera(glm::vec3(0.0f, 0.0f, 10.0f));
	
	//m_asset.LoadTexture("skin.png");
	m_asset.LoadShader("static_and_skinned_model", "static_and_skinned_model.vs", "static_and_skinned_model.fs");
	m_asset.LoadShader("static_and_skinned_model_normals", "static_and_skinned_model_normals.vs", "static_and_skinned_model_normals.fs", "static_and_skinned_model_normals.gs");
	m_asset.LoadShader("skeleton_bones", "skeleton_bones.vs", "skeleton_bones.fs");
	m_asset.LoadShader("skeleton_joints", "skeleton_joints.vs", "skeleton_joints.fs");
	m_asset.LoadShader("debug_bullet_physics", "debug_bullet_physics.vs", "debug_bullet_physics.fs");
}

void App::main_loop() {
	Helix::Model bobModel("../Assets/Models/guard/boblampclean.md5mesh");
	Helix::Model bobModel2("../Assets/Models/guard/boblampclean.md5mesh");
	Helix::Model pyroModel("../Assets/Models/Pyro/Pyro.obj");
	Helix::Model chickenModel("../Assets/Models/chicken_anim6.dae");
	
	//examples
	Init_Example_Skinning_Bullet_VHACD(pyroModel);
	
	//~examples
	
	bool togglePhysicsPause = true;

	while(m_running) {
		this->loop();

		SDL_Event e;

		while(SDL_PollEvent(&e)) {
			if(e.type == SDL_QUIT) {
				m_running = false;
			} else if(e.type == SDL_KEYDOWN) {
				switch(e.key.keysym.sym) {
					case SDLK_ESCAPE:
						m_running = false;
						break;
						
						case SDLK_p:
						{
							if(togglePhysicsPause) {
								togglePhysicsPause = false;
							}
							else {
								togglePhysicsPause = true;
							}
						}
						break;
						
					    case SDLK_e:
						{
							if(m_toggleWireframe) {
								glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
								m_toggleWireframe = false;
							}
							else {
								glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
								m_toggleWireframe = true;
							}
						}
						break;
						
						case SDLK_f:
						{       
							if(m_toggleFullscreen) {
								SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
								
								int w, h;
								SDL_GetWindowSize(m_window, &w, &h);
								this->setSizeX(w);
								this->setSizeY(h);
								
								//std::cout << this->getSizeX() << "x" << this->getSizeY() << std::endl;
								
								m_toggleFullscreen = false;
							} else {
								SDL_SetWindowFullscreen(m_window, 0);
								//SDL_SetWindowDisplayMode(window, 0);

								int w, h;
								SDL_GetWindowSize(m_window, &w, &h); //?
								
								this->setSizeX(800);
								this->setSizeY(600);
								
								//std::cout << this->getSizeX() << "x" << this->getSizeY() << std::endl;
								
								m_toggleFullscreen = true;
							}
						}
						break;

					default:
						break;
				}
			} else if(e.type == SDL_MOUSEMOTION) {
				//xpos = e.motion.x;
				//ypos = e.motion.y;
				//std::cout << "x: " << xpos << " y: " << ypos << std::endl;
			} else if(e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_RIGHT) {
				SDL_ShowCursor(SDL_DISABLE);
				SDL_SetRelativeMouseMode(SDL_TRUE);
				m_toggleMouseRelative = true;

				m_skipMouseResolution = 5;
			} else if(e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_RIGHT) {
				SDL_ShowCursor(SDL_ENABLE);
				SDL_SetRelativeMouseMode(SDL_FALSE);
				m_toggleMouseRelative = false;

				m_skipMouseResolution = 5;
			}
		}

		int numDrawnObjects = 0;
		glViewport(0, 0, this->getSizeX(), this->getSizeY());

		process_input();
		
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		{ // mouse
			int xpos;
			int ypos;
			SDL_GetRelativeMouseState(&xpos, &ypos);
			if(m_skipMouseResolution > 0 && (xpos != 0 || ypos != 0)) {
				m_skipMouseResolution--;
			} else {
				if(m_toggleMouseRelative) {
					m_camera.ProcessMouseMovement(xpos, ypos);
					m_camera.ProcessMouseScroll(m_mouseScroll);
					m_mouseScroll = 0;
				} else {
					SDL_GetMouseState(&xpos, &ypos);
					//std::cout << "x: " << xpos << " y: " << ypos << std::endl;
				}
			}
		}

		glm::mat4 view = m_camera.GetViewMatrix();
		// glm::mat4 projection = glm::perspective(glm::radians(m_camera.GetZoom()), this->getSizeX()/(float)this->getSizeY(), 0.1f, 1000.0f);
		glm::mat4 projection = glm::tweakedInfinitePerspective(glm::radians(m_camera.GetZoom()), this->getSizeX()/(float)this->getSizeY(), 0.5f);
		glm::vec3 viewPos = m_camera.GetPosition();
		
		//examples
		Render_Example_Skinning_Bullet_VHACD(m_camera, m_asset, this->getDeltaTime(), this->getTimeElapsed(), view, projection, viewPos, bobModel, bobModel2, pyroModel, chickenModel);
		
		//~examples
		
		SDL_GL_SwapWindow(m_window);

		this->showFPS();

		SDL_Delay(1);
	}
}

void App::loop() {
	// fps
	m_frames_elapsed++;
	m_ticks_current = SDL_GetTicks();

	// delta time
	m_delta_time = (m_ticks_current - m_ticks_then) / 1000.0f;
	m_ticks_then = m_ticks_current;

	// time elapsed
	auto m_chrono_now = std::chrono::high_resolution_clock::now();
	m_chrono_elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(m_chrono_now - m_chrono_start).count();
}

void App::showFPS() {
	if(m_ticks_previous < m_ticks_current - 1000) {
		m_ticks_previous = SDL_GetTicks();
		m_frames_current = m_frames_elapsed;
		m_frames_elapsed = 0;

		if(m_frames_current < 1) {
			m_frames_current = 1;
		}

		 std::cout << "FPS: " << m_frames_current << std::endl;
	}
}

int App::getSizeX() const {
	return m_sizeX;
}

int App::getSizeY() const {
	return m_sizeY;
}

void App::setSizeX(int sizeX) {
	m_sizeX = sizeX;
}

void App::setSizeY(int sizeY) {
	m_sizeY = sizeY;
}

double App::getDeltaTime() const {
	return m_delta_time;
}

double App::getTimeElapsed() const {
	return m_chrono_elapsed;
}

void App::cleanup() {
	SDL_DestroyWindow(m_window);

	SDL_Quit();
}

void App::process_input() {
	float speed = 2.0;
	
	const Uint8* state = SDL_GetKeyboardState(NULL);
	if(state[SDL_SCANCODE_W]) {
		m_camera.ProcessKeyboard(tb::Camera::MoveDirection::FORWARD, this->getDeltaTime() * speed);
	}

	if(state[SDL_SCANCODE_S]) {
		m_camera.ProcessKeyboard(tb::Camera::MoveDirection::BACKWARD, this->getDeltaTime() * speed);
	}

	if(state[SDL_SCANCODE_A]) {
		m_camera.ProcessKeyboard(tb::Camera::MoveDirection::LEFT, this->getDeltaTime() * speed);
	}

	if(state[SDL_SCANCODE_D]) {
		m_camera.ProcessKeyboard(tb::Camera::MoveDirection::RIGHT, this->getDeltaTime() * speed);
	}
}

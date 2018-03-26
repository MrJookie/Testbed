link64bit := -Llib/linux/64bit -lSDL2 -lSDL2_image -lSDL2_mixer \
				-Wl,-rpath="../lib/linux/64bit/",-rpath="../lib/linux" -Llib/linux -lGL -lGLEW -lassimp
link32bit := -Llib/linux/32bit -lSDL2 -lSDL2_image -lSDL2_mixer \
				-Wl,-rpath="../lib/linux/32bit/",-rpath="../lib/linux" -Llib/linux -lGL -lGLEW -lassimp
				
bulletLink := -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath
hacdLink := -lHACD
vhacdLink := -Llibs/vhacd -lvhacd -lpthread -fopenmp

link := $(link64bit) $(bulletLink) $(hacdLink) $(vhacdLink)

arch := 

includes := -I/usr/include/bullet  						\


hpp :=
		
cpp := 	\
		src/Main.cpp									\
		src/App.cpp										\
		src/Asset.cpp									\
		src/Camera.cpp									\
		src/Assimp_Model_Animator/Model.cpp				\
		src/Assimp_Model_Animator/BulletGLDebugger.cpp	\
		src/example_skinning_bullet_vhacd.cpp			\

exe := release/Testbed

build := build
flags :=

obj := $(addprefix $(build)/, $(patsubst %.cpp,%.o,$(cpp)))

.phony: make_dirs

all: $(exe)

clean:
	find $(build) -type f -name *.o -exec rm {} \;

make_dirs:
	@mkdir -p $(build)
	@mkdir -p $(build)/src/

$(exe): $(obj)
	g++ $^ -o $(exe) $(link) $(arch) -g

$(build)/%.o: %.cpp
	g++ -c $< -o $@ -std=c++14 $(arch) $(flags) $(includes) -g

#include "App.hpp"
#include <BulletCollision/CollisionShapes/btBox2dShape.h>

/* TODO:
 * properly delete all bulletphysics allocations, also rigid bodies -> change the rest to smartptrs
 */
 
/*
case SDLK_b:
{
	for(int i = 0; i < 5; ++i) {
		for(int j = 0; j < 5; ++j) {
			for(int k = 0; k < 5; ++k) {
				btTransform startTransform;
				
				startTransform.setIdentity();
				startTransform.setOrigin(btVector3(
				btScalar(i * 10),
				btScalar(j * 10),
				btScalar(k * 10)));

				btRigidBody* fallRigidBody = new btRigidBody(fallRigidBodyCI);
				
				btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
				fallRigidBody->setMotionState(motionState);
				
				//fallRigidBody->setWorldTransform(startTransform);
				
				dynamicsWorld->addRigidBody(fallRigidBody);
				//fallRigidBody->setRestitution(0.5);
				
				rigidBodies.push_back(fallRigidBody);
			}
		}
	}
}
break;

case SDLK_r:
{
	for(int i = 0; i < rigidBodies.size(); ++i) {
		if(i % 2 == 0) {
			rigidBodies[i]->setActivationState(0);
			//dynamicsWorld->removeRigidBody(rigidBodies[i]);
		}
	}
}
break;

case SDLK_t:
{
	for(int i = 0; i < rigidBodies.size(); ++i) {
		rigidBodies[i]->setActivationState(1);
		//dynamicsWorld->addRigidBody(rigidBodies[i]);
	}
}
break;
*/

btBroadphaseInterface* broadphase;
btDefaultCollisionConfiguration* collisionConfiguration;
btCollisionDispatcher* dispatcher;
btSequentialImpulseConstraintSolver* solver;
btDiscreteDynamicsWorld* dynamicsWorld;
BulletGLDebugger* dbg;

std::vector<btRigidBody*> rigidBodies;

//glm::vec3 CurrentLocation;

int Init_Example_Skinning_Bullet_VHACD(Helix::Model& pyroModel) {
	//CurrentLocation = camera.GetPosition();
	
	//bulletphysics
	//~ std::unique_ptr<btBroadphaseInterface> broadphase = std::make_unique<btDbvtBroadphase>();
	//~ std::unique_ptr<btDefaultCollisionConfiguration> collisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();
	//~ std::unique_ptr<btCollisionDispatcher> dispatcher = std::make_unique<btCollisionDispatcher>(collisionConfiguration.get());
	//~ std::unique_ptr<btSequentialImpulseConstraintSolver> solver = std::make_unique<btSequentialImpulseConstraintSolver>();
	//~ std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(dispatcher.get(), broadphase.get(), solver.get(), collisionConfiguration.get());

	broadphase = new btDbvtBroadphase();
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	solver = new btSequentialImpulseConstraintSolver;
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);

	//~ std::unique_ptr<BulletGLDebugger> dbg = std::make_unique<BulletGLDebugger>();
	dbg = new BulletGLDebugger();
	dbg->setDebugMode(btIDebugDraw::DBG_MAX_DEBUG_DRAW_MODE | btIDebugDraw::DBG_DrawAabb);
	dynamicsWorld->setDebugDrawer(dbg);

	dynamicsWorld->setGravity(btVector3(0, -9.81, 0));
	
	btCollisionShape* groundShape = new btBoxShape(btVector3(1, 1, 1));
	//btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);
	
	btConvexHullShape* convexHull = new btConvexHullShape();
	convexHull->addPoint(btVector3(-100, -2, 100));
	convexHull->addPoint(btVector3(100, -2, 100));
	convexHull->addPoint(btVector3(100, -2, -100));
	convexHull->addPoint(btVector3(-100, -2, -100));
	
	groundShape = convexHull;

	//btConvexShape* fallShape = pyroModel.GetBulletTriangleShape();
	//btConvexHullShape* fallShape = pyroModel.GetBulletConvexHullShape();
	
	//btCompoundShape* fallShape = pyroModel.CreateShapeHACD();
	btCompoundShape* fallShape = pyroModel.GetBulletVHACDShape();
	
	
	//btConvexShape* fallShape = new btBoxShape(btVector3(1, 1, 1));
	//fallShape->setMargin(0.00001);
	
	//btSphereShape* fallShape = new btSphereShape(btScalar(1.0f));

	btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
	btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
	btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
	dynamicsWorld->addRigidBody(groundRigidBody);


	btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
	btScalar mass = 1;
	btVector3 fallInertia(0, 0, 0);
	fallShape->calculateLocalInertia(mass, fallInertia);
	btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, fallShape, fallInertia);
	

	btTransform startTransform2;
	startTransform2.setIdentity();
	btVector3 position(0,0,0);
	startTransform2.setOrigin(position);
	
	btRigidBody* fallRigidBody = new btRigidBody(fallRigidBodyCI);
	fallRigidBody->setWorldTransform(startTransform2);
	
	
	/// https://pybullet.org/Bullet/phpBB3/viewtopic.php?p=6244&f=9&t=1506
	/// You need to create a collision shape class that can handle a transform, and a motion state that undoes the transform.
	/// No, you should not use btGImpactShape, and don't use a single btConvexHullShape.

	
	
	dynamicsWorld->addRigidBody(fallRigidBody);
	rigidBodies.push_back(fallRigidBody);
	
	groundRigidBody->setRestitution(0.0);
	fallRigidBody->setRestitution(0.0);
	
	//groundRigidBody->setActivationState(DISABLE_DEACTIVATION);
	fallRigidBody->setActivationState(DISABLE_DEACTIVATION);
}

void Render_Example_Skinning_Bullet_VHACD(Testbed::Camera& camera, Testbed::Asset& asset, double dt, double timeElapsed, glm::mat4 view, glm::mat4 projection, glm::vec3 viewPos, Helix::Model& bobModel, Helix::Model& bobModel2, Helix::Model& pyroModel, Helix::Model& chickenModel) {
	// LOAD MODEL
	glm::mat4 model = glm::mat4();

	//bob
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(1.5, -2.0, -2.0));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.07f, 0.07f, 0.07f));

	bobModel.SetAnimPlay(true);
	bobModel.SetAnimLoop(true);
	bobModel.SetAnimSpeed(30.0);
	bobModel.Update(dt);

	//mount	
	glm::mat4 modelMatLamp = bobModel.GetBoneMatrix("lamp");
	modelMatLamp = glm::translate(modelMatLamp, glm::vec3(-4.0, 10.0, 0.0));

	glm::mat4 finalModel = model * modelMatLamp;
	finalModel = glm::translate(finalModel, glm::vec3(1.5, -2.0, -2.0));
	finalModel = glm::rotate(finalModel, glm::radians(180.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
	finalModel = glm::scale(finalModel, glm::vec3(5.7f, 5.7f, 5.7f));

	pyroModel.Draw(asset.GetShader("static_and_skinned_model").id, finalModel, view, projection, timeElapsed, finalModel, viewPos);
	//~get bone

	bobModel.Draw(asset.GetShader("static_and_skinned_model").id, model, view, projection, timeElapsed, finalModel, viewPos);

	//pyro
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(-1.5, -2.0, -2.0));

	pyroModel.Draw(asset.GetShader("static_and_skinned_model").id, model, view, projection, timeElapsed, finalModel, viewPos);

	//bob2
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(-4.5, -2.0, -2.5));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.07f, 0.07f, 0.07f));

	bobModel2.SetAnimPlay(true);
	bobModel2.SetAnimLoop(true);
	bobModel2.SetAnimSpeed(30.0);
	bobModel2.Update(dt);

	/*
	for(int j = 0; j < 10; j++) {
		for(int i = 0; i < 10; i++) {
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(i * 2.5, -2.0, j * -2.5));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.07f, 0.07f, 0.07f));
			
			bobModel2.Draw(asset.GetShader("test_03").id, model, view, projection, this->getTimeElapsed());
		}
	}
	*/

	bobModel2.Draw(asset.GetShader("static_and_skinned_model").id, model, view, projection, timeElapsed, finalModel, viewPos);
	bobModel2.DrawSkeletonBones(asset.GetShader("skeleton_bones").id, model, view, projection);
	bobModel2.DrawSkeletonJoints(asset.GetShader("skeleton_joints").id, model, view, projection);

	//chicken
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(10.0, -2.0, -2.0));

	chickenModel.SetAnimPlay(true);
	chickenModel.SetAnimLoop(true);
	chickenModel.SetAnimSpeed(1.0);
	chickenModel.Update(dt);

	//get mesh and update it's verts
	glm::mat4 model2;
	//model2 = glm::translate(model2, glm::vec3(0.0, 10.2, 0.0));
	model2 = glm::rotate(model2, glm::radians(0.1f), glm::vec3(0.0f, 1.0f, 0.0f));

	Helix::Mesh* mesh = chickenModel.GetMesh("polySurfaceShape7");
	if(mesh != nullptr) {
		for(auto& vertex : mesh->m_vertices) {
			vertex.Position = model2 * glm::vec4(vertex.Position, 1.0);
		}
	}
	//~get mesh

	//chickenModel.Draw(asset.GetShader("static_and_skinned_model").id, model, view, projection, this->getTimeElapsed(), finalModel);
	//chickenModel.DrawSkeletonBones(asset.GetShader("skeleton_bones").id, model, view, projection);
	//chickenModel.DrawSkeletonJoints(asset.GetShader("skeleton_joints").id, model, view, projection);

	//if(togglePhysicsPause) {
		dynamicsWorld->stepSimulation((float)dt, 7);
	//}

	glm::mat4 pyroFromBulletModelMat;

	for(int i = 0; i < rigidBodies.size(); ++i) {
		//get model matrix
		//glm::mat4 pyroFromBulletModelMat;
		
		btTransform trans;
		rigidBodies[i]->getMotionState()->getWorldTransform(trans);
		trans.getOpenGLMatrix(glm::value_ptr(pyroFromBulletModelMat));
		
		//shift graphics
		//pyroFromBulletModelMat = glm::translate(pyroFromBulletModelMat, glm::vec3(0.0, -2.0, 0.0));
		
		pyroModel.Draw(asset.GetShader("static_and_skinned_model").id, pyroFromBulletModelMat, view, projection, timeElapsed, finalModel, viewPos);
	}

	/*
	glm::vec3 FinalLocation = pyroFromBulletModelMat[3];
	CurrentLocation += (FinalLocation - CurrentLocation) * dt * 0.007f;
	camera.SetPosition(CurrentLocation);
	*/

	//~ //pyroModel.m_convexShapes.size()
	//~ for(int i = 0; i < 1; ++i) {
		//~ //std::cout << "verts: " << pyroModel.m_convexShapes[i]->getNumVertices() << std::endl;
		
		//~ std::vector<glm::vec3> vertices;
		//~ std::vector<glm::vec3> colors;
		
		
		//~ const unsigned int* idx = ((btShapeHull*)pyroModel.m_convexShapes[i])->getIndexPointer();
		//~ const btVector3* vtx = ((btShapeHull*)pyroModel.m_convexShapes[i])->getVertexPointer();

		
		//~ int index = 0;
		//~ for (int g = 0; g < 1; g++)
		//~ {
			//~ int i1 = index++;
			//~ int i2 = index++;
			//~ int i3 = index++;

			//~ int index1 = idx[i1];
			//~ int index2 = idx[i2];
			//~ int index3 = idx[i3];

			//~ btVector3 v1 = vtx[index1];
			//~ btVector3 v2 = vtx[index2];
			//~ btVector3 v3 = vtx[index3];
			
			//~ vertices.push_back( glm::vec3(v1.getX(), v1.getY(), v1.getZ()) );
			//~ vertices.push_back( glm::vec3(v2.getX(), v2.getY(), v2.getZ()) );
			//~ vertices.push_back( glm::vec3(v3.getX(), v3.getY(), v3.getZ()) );
			
			//~ colors.push_back( glm::vec3(1.0, 1.0, 1.0) );
			//~ colors.push_back( glm::vec3(1.0, 1.0, 1.0) );
			//~ colors.push_back( glm::vec3(1.0, 1.0, 1.0) );

		//~ }
		
		
		//~ GLuint vao, vbo[2], ebo;

		//~ glGenVertexArrays(1, &vao);
		//~ glGenBuffers(2, vbo);
		
		//~ glUseProgram(asset.GetShader("debug_bullet_physics").id);
		//~ glBindVertexArray(vao);

		//~ glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		//~ glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
		//~ glEnableVertexAttribArray(0);    
		//~ glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		
		//~ glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		//~ glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * colors.size(), &colors[0], GL_STATIC_DRAW);
		//~ glEnableVertexAttribArray(1);    
		//~ glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		//~ glUniformMatrix4fv(glGetUniformLocation(asset.GetShader("debug_bullet_physics").id, "model"), 1, GL_FALSE, glm::value_ptr(pyroFromBulletModelMat));
		//~ glUniformMatrix4fv(glGetUniformLocation(asset.GetShader("debug_bullet_physics").id, "view"), 1, GL_FALSE, glm::value_ptr(view));
		//~ glUniformMatrix4fv(glGetUniformLocation(asset.GetShader("debug_bullet_physics").id, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		
		//~ glDrawArrays(GL_TRIANGLES, 0, vertices.size());
		
		//~ glBindVertexArray(0);
		//~ glUseProgram(0);
		
		//~ glDeleteBuffers(2, vbo);
		//~ glDeleteVertexArrays(1, &vao);
		
		//~ vertices.clear();
		//~ colors.clear();
	//~ }


	/*
	if(!m_toggleWireframe) {
		dynamicsWorld->debugDrawWorld();
		dbg->Draw(asset.GetShader("debug_bullet_physics").id, glm::mat4(1.0), view, projection);
	}
	*/
}

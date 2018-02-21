#ifndef BULLET_GL_DEBUGGER_HPP
#define BULLET_GL_DEBUGGER_HPP

#include <LinearMath/btIDebugDraw.h>

#include <GL/glew.h>

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class BulletGLDebugger : public btIDebugDraw
{
public:
  BulletGLDebugger(int sizeX, int sizeY);
  ~BulletGLDebugger() {};

  virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor);
  virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
  virtual void drawSphere (const btVector3& p, btScalar radius, const btVector3& color);
  virtual void drawBox (const btVector3& boxMin, const btVector3& boxMax, const btVector3& color, btScalar alpha);
  virtual void drawTriangle(const btVector3& a, const btVector3& b, const btVector3& c, const btVector3& color, btScalar alpha);
  virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color);
  virtual void reportErrorWarning(const char*);
  virtual void draw3dText(const btVector3&, const char*);
  virtual void setDebugMode(int m) { m_debugMode = m; }
  virtual int getDebugMode() const { return m_debugMode; }
  
  void Draw(GLuint shader, glm::mat4 model, glm::mat4 view, glm::mat4 projection);

private:
	int m_sizeX;
	int m_sizeY;
	int m_debugMode;
	GLuint m_vao;
	GLuint m_vbo[2];
	GLuint m_ebo;
	
	std::vector<glm::vec3> m_vertices;
	std::vector<glm::vec3> m_colors;
};

#endif //BULLET_GL_DEBUGGER_HPP

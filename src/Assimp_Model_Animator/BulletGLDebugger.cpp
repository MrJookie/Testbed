#include "BulletGLDebugger.hpp"

/*
unsigned int currentIndex = 0;
std::vector<GLuint> m_indices;
*/

BulletGLDebugger::BulletGLDebugger(int sizeX, int sizeY)
: m_debugMode(0), m_sizeX(sizeX), m_sizeY(sizeY)
{
	glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    
    glGenBuffers(2, m_vbo);
    glGenBuffers(1, &m_ebo);
    
    glBindVertexArray(0);
}

void BulletGLDebugger::Draw(GLuint shader, glm::mat4 model, glm::mat4 view, glm::mat4 projection) {
	GLuint vao, vbo[2], ebo;
	
	glGenVertexArrays(1, &vao);
	glGenBuffers(2, vbo);
	
	glUseProgram(shader);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * m_vertices.size(), &m_vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);    
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * m_colors.size(), &m_colors[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);    
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	
	
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	
	glDrawArrays(GL_LINES, 0, m_vertices.size());
	
	//glDrawElements(GL_LINES, m_indices.size(), GL_UNSIGNED_INT, (void*)&m_indices[0]);

    glBindVertexArray(0);
    glUseProgram(0);
    
    glDeleteBuffers(2, vbo);
    glDeleteVertexArrays(1, &vao);
    
    m_vertices.clear();
    m_colors.clear();
    /*
    m_indices.clear();
    currentIndex = 0;
    */
}

void BulletGLDebugger::drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor) {
	glm::vec3 vertexFrom = glm::vec3(from.getX(), from.getY(), from.getZ());
	glm::vec3 vertexTo = glm::vec3(to.getX(), to.getY(), to.getZ());
	
	glm::vec3 colorFrom = glm::vec3(fromColor.getX(), fromColor.getY(), fromColor.getZ());
	glm::vec3 colorTo = glm::vec3(toColor.getX(), toColor.getY(), toColor.getZ());
	
	m_vertices.push_back(vertexFrom);
	m_vertices.push_back(vertexTo);
	
	m_colors.push_back(colorFrom);
	m_colors.push_back(colorTo);
	
	/*
	m_indices.push_back(currentIndex);
	m_indices.push_back(currentIndex + 1);
	currentIndex += 2;
	*/
}

void BulletGLDebugger::drawLine (const btVector3& from, const btVector3& to, const btVector3& color) {
  drawLine(from, to, color, color);
}

void BulletGLDebugger::drawSphere (const btVector3& p, btScalar radius, const btVector3& color) {
	std::cout << "drawSphere" << std::endl;
}

void BulletGLDebugger::drawBox (const btVector3& boxMin, const btVector3& boxMax, const btVector3& color, btScalar alpha) {
	std::cout << "drawBox" << std::endl;
}

void BulletGLDebugger::drawTriangle (const btVector3& a, const btVector3& b, const btVector3& c, const btVector3& color, btScalar alpha) {
	std::cout << "drawTriangle" << std::endl;
}

void BulletGLDebugger::draw3dText (const btVector3& location, const char* string) {
	std::cout << "draw3dText" << std::endl;
}

void BulletGLDebugger::reportErrorWarning (const char* string) {
	std::cout << "reportErrorWarning" << std::endl;
}

void BulletGLDebugger::drawContactPoint(const btVector3& pointOnB, const btVector3& normalOnB, btScalar distance, int /*lifeTime*/, const btVector3& color) {
	std::cout << "drawContactPoint" << std::endl;
}





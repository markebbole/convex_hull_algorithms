#ifndef PROC_GENA385
#define PROC_GENA385

#include <vector>
#include <map>
#include "ErrorChecking.h"
#include <iostream>
enum {
  kVertexBuffer,
  kIndexBuffer,
  kNormalBuffer,
  kNumVbos
};

class VAO;
uint32_t numObjects = 0;

class VertexInfo {
public:
  glm::vec4 coordinates;
  glm::vec4 normal;
};

struct GeometryData {

  std::vector<glm::vec4> obj_vertices;
  std::vector<glm::uvec3> obj_faces;
  std::vector<glm::uvec2> obj_lines;

  std::vector<glm::vec4> getVertexList() {
  	return obj_vertices;
  }

  std::vector<glm::uvec3> getFaceList() {
  	return obj_faces;
  }
};

GeometryData* createUnitCube() {
	GeometryData* gd = new GeometryData();
	gd->obj_vertices.push_back(glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f));
	gd->obj_vertices.push_back(glm::vec4(0.5f, -0.5f, -0.5f, 1.0f));

	gd->obj_vertices.push_back(glm::vec4(-0.5f, -0.5f, 0.5f, 1.0f));
	gd->obj_vertices.push_back(glm::vec4(0.5f, -0.5f, 0.5f, 1.0f));

	gd->obj_vertices.push_back(glm::vec4(-0.5f, 0.5f, -0.5f, 1.0f));
	gd->obj_vertices.push_back(glm::vec4(0.5f, 0.5f, -0.5f, 1.0f));

	gd->obj_vertices.push_back(glm::vec4(-0.5f, 0.5f, 0.5f, 1.0f));
	gd->obj_vertices.push_back(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));

	return gd;
}

class VAO {
  
  GLuint buffer_objects[kNumVbos];

public:
	GLuint vao_descriptor;
	GeometryData* data;

	//descriptors and vaos have same length
	static void applyDescriptors(int* descriptors, std::vector<VAO*>& vaos) {
		for(int i = 0; i < vaos.size(); ++i) {
			vaos[i]->vao_descriptor = descriptors[i];
		}
	}

	void draw() {

		CHECK_GL_ERROR(glBindVertexArray(vao_descriptor));

		// Render!
		CHECK_GL_ERROR(
		    glDrawElements(GL_TRIANGLES, data->obj_faces.size() * 3, GL_UNSIGNED_INT, 0));
	}

	void drawPoints(double size) {

		CHECK_GL_ERROR(glBindVertexArray(vao_descriptor));
		CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, buffer_objects[kVertexBuffer]));
		glPointSize(size);

		CHECK_GL_ERROR(
		    glDrawArrays(GL_POINTS, 0, data->obj_vertices.size()));

	}

	void drawLines() {

		CHECK_GL_ERROR(glBindVertexArray(vao_descriptor));
		
		// Render!
		CHECK_GL_ERROR(
		    glDrawElements(GL_LINES, data->obj_lines.size() * 2, GL_UNSIGNED_INT, 0));

	}


	//get vertex data from geometrydata object and send to opengl
	void updateVertices(int attribPointer) {
		CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, buffer_objects[kVertexBuffer]));
		CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
		                            sizeof(float) * data->obj_vertices.size() * 4, // total size of the position buffer
		                            &(data->obj_vertices[0]),
		                            GL_STATIC_DRAW));
		
		CHECK_GL_ERROR(glVertexAttribPointer(attribPointer, 4, GL_FLOAT, GL_FALSE, 0, 0));
    	CHECK_GL_ERROR(glEnableVertexAttribArray(attribPointer));

	}

	//get gd->obj_faces from data and send to opengl
	void updateFaces() {

		CHECK_GL_ERROR(
		    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_objects[kIndexBuffer]));
		CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		                            sizeof(uint32_t) * data->obj_faces.size() * 3,
		                            &(data->obj_faces[0]), GL_STATIC_DRAW));


	}

	void updateLines() {
		CHECK_GL_ERROR(
		    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_objects[kIndexBuffer]));
		CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		                            sizeof(uint32_t) * data->obj_lines.size() * 2,
		                            &(data->obj_lines[0]), GL_STATIC_DRAW));
	}

	void generateBuffers() {
		CHECK_GL_ERROR(glGenBuffers(kNumVbos, buffer_objects));
	}

	void clearAllData() {
		data->obj_lines.clear();
		data->obj_faces.clear();
		data->obj_vertices.clear();
		updateVertices(0);
		updateLines();
		updateFaces();
	}



};


class ShaderProgram {

GLuint program_id;
std::map<std::string, GLint> locationToID;

public:

	void addLocation(std::string location_name) {
		GLint loc = 0;
		CHECK_GL_ERROR(loc = glGetUniformLocation(program_id, location_name.c_str()));
		locationToID[location_name] = loc;
	}
	void addUniformMatrix4fv(std::string location_name, float* m) {
		CHECK_GL_ERROR(glUniformMatrix4fv(locationToID[location_name], 1,
		                                  GL_FALSE, m));
	}
	void addUniformFloat(std::string location_name, float f) {
		CHECK_GL_ERROR(glUniform1f(locationToID[location_name], f));
	}
	void addUniformInt(std::string location_name, int x) {
		CHECK_GL_ERROR(glUniform1i(locationToID[location_name], x));
	}
	void addUniform3fv(std::string location_name, float* v) {
		CHECK_GL_ERROR(glUniform3fv(locationToID[location_name], 1, &v[0]));
	}
	//return id of vertex shader
	GLuint createVertexShader(std::string vertex_shader) {
		GLuint vertex_shader_id = 0;
		const char* vertex_source_pointer = vertex_shader.c_str();
		CHECK_GL_ERROR(vertex_shader_id = glCreateShader(GL_VERTEX_SHADER));
		CHECK_GL_ERROR(
		    glShaderSource(vertex_shader_id, 1, &vertex_source_pointer, nullptr));
		glCompileShader(vertex_shader_id);
		CHECK_GL_SHADER_ERROR(vertex_shader_id);

		return vertex_shader_id;

	}
	GLuint createGeometryShader(std::string geometry_shader) {
		GLuint geometry_shader_id = 0;
		const char* geometry_source_pointer = geometry_shader.c_str();
		CHECK_GL_ERROR(geometry_shader_id = glCreateShader(GL_GEOMETRY_SHADER));
		CHECK_GL_ERROR(
		    glShaderSource(geometry_shader_id, 1, &geometry_source_pointer, nullptr));
		glCompileShader(geometry_shader_id);
		CHECK_GL_SHADER_ERROR(geometry_shader_id);

		return geometry_shader_id;
	}
	GLuint createFragmentShader(std::string fragment_shader) {
		GLuint fragment_shader_id = 0;
		const char* fragment_source_pointer = fragment_shader.c_str();
		CHECK_GL_ERROR(fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER));
		CHECK_GL_ERROR(
		    glShaderSource(fragment_shader_id, 1, &fragment_source_pointer, nullptr));
		glCompileShader(fragment_shader_id);
		CHECK_GL_SHADER_ERROR(fragment_shader_id);

		return fragment_shader_id;
	}
	void link(GLuint vs, GLuint gs, GLuint fs) {
		CHECK_GL_ERROR(program_id = glCreateProgram());
		CHECK_GL_ERROR(glAttachShader(program_id, vs));
		CHECK_GL_ERROR(glAttachShader(program_id, gs));
		CHECK_GL_ERROR(glAttachShader(program_id, fs));

		CHECK_GL_ERROR(glBindAttribLocation(program_id, 0, "vertex_position"));
		CHECK_GL_ERROR(glBindFragDataLocation(program_id, 0, "fragment_color"));
		glLinkProgram(program_id);
		CHECK_GL_PROGRAM_ERROR(program_id);
	}

	void use() {
		CHECK_GL_ERROR(glUseProgram(program_id));
	}
};


#endif

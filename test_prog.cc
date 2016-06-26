#include <fstream>
#include <iostream>
#include <sstream>
#include <iterator>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>

// OpenGL library includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "ErrorChecking.h"
#include "ProceduralGeometry.h"
#include "convexhull.h"
#include <chrono>
int window_width = 800, window_height = 600;
const std::string window_title = "Convex Hull Algorithms";
ShaderProgram pointProgram;
ShaderProgram hullProgram2D;
ShaderProgram hullProgram3D;
ShaderProgram InputGeometryProgram;
float last_x = 0.0f, last_y = 0.0f, current_x = 0.0f, current_y = 0.0f;
bool drag_state = false;
int current_button = -1;
glm::mat4 projection_matrix;
glm::mat4 view_matrix;
glm::mat4 view_projection_matrix;
glm::vec3 look;
VAO point_vao;
VAO hull_line_vao;
VAO input_geo_vao;
glm::vec3 eye;
glm::vec3 up;
glm::vec3 center;
bool spinning = false;
float spinAmount = 0.0f;


int numModes = 3;
int mode=0; //0 = 2D, 1 = 3D points, 2 = 3D object
// Some very primitive UI for the program


void LoadObj(const std::string& file, std::vector<glm::vec4>& vertices,
             std::vector<glm::uvec3>& indices) {
  std::ifstream in(file);
  int i = 0, j = 0;
  glm::vec4 vertex = glm::vec4(0.0, 0.0, 0.0, 1.0);
  glm::uvec3 face_indices = glm::uvec3(0, 0, 0);
  while (in.good()) {
    char c = in.get();
    switch (c) {
      case 'v':
        in >> vertex[0] >> vertex[1] >> vertex[2];
        vertices.push_back(vertex);
        break;
      case 'f':
        in >> face_indices[0] >> face_indices[1] >> face_indices[2];
        face_indices -= 1;
        indices.push_back(face_indices);
        break;
      default:
        break;
    }
  }
  in.close();
}

void ErrorCallback(int error, const char* description) {
  std::cerr << "GLFW Error: " << description << "\n";
}


void KeyCallback(GLFWwindow* window, int key, int scancode, int action,
                 int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);


  if(key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
    point_vao.clearAllData();
    hull_line_vao.clearAllData();
  }

  if(key == GLFW_KEY_G && action == GLFW_PRESS) {
    point_vao.clearAllData();
    hull_line_vao.clearAllData();

    if(mode == 1) {
      generateRandom3DPoints(500, point_vao.data->obj_vertices, glm::vec3(-.3,-.3, -.3), glm::vec3(.3,.3, .3));
      point_vao.updateVertices(0);
    } else if(mode == 0){
      generateRandom2DPoints(500, point_vao.data->obj_vertices, glm::vec2(-.3,-.3), glm::vec2(.3,.3));
      point_vao.updateVertices(0);
      //point_vao.updateFaces();
    }
  }

  if(key == GLFW_KEY_H && action == GLFW_PRESS) {
    //point_vao.clearAllData();
    hull_line_vao.clearAllData();


    if(mode == 1) {
      std::vector<glm::uvec3> hullFaces;

      giftWrapping3D(point_vao.data->obj_vertices, hullFaces);

      hull_line_vao.data->obj_vertices = point_vao.data->obj_vertices;
      hull_line_vao.data->obj_faces = hullFaces;
      hull_line_vao.updateVertices(0);
      hull_line_vao.updateFaces();
    } else if(mode == 0){
      hull_line_vao.data->obj_vertices = quickHull2D(point_vao.data->obj_vertices);
      //giftWrapping(point_vao.data->obj_vertices);

      hull_line_vao.updateVertices(0);
      hull_line_vao.updateFaces();
      visualizeGiftWrapping(hull_line_vao, hull_line_vao.data->obj_vertices);
    } else {
      std::vector<glm::uvec3> hullFaces;

      giftWrapping3D(input_geo_vao.data->obj_vertices, hullFaces);
      hull_line_vao.data->obj_vertices = input_geo_vao.data->obj_vertices;
      hull_line_vao.data->obj_faces = hullFaces;

      std::cout << hullFaces.size() << std::endl;

      hull_line_vao.updateVertices(0);
      hull_line_vao.updateFaces();
    }
  }

  if(key == GLFW_KEY_D && action == GLFW_PRESS) {
    mode = (mode + 1) % numModes;
    hull_line_vao.clearAllData();
    point_vao.clearAllData();


    if(mode == 1) {
      generateRandom3DPoints(1000, point_vao.data->obj_vertices, glm::vec3(-.3,-.3, -.3), glm::vec3(.3,.3, .3));
      point_vao.updateVertices(0);

      std::vector<glm::uvec3> hullFaces;

      giftWrapping3D(point_vao.data->obj_vertices, hullFaces);

      hull_line_vao.data->obj_vertices = point_vao.data->obj_vertices;
      hull_line_vao.data->obj_faces = hullFaces;
      hull_line_vao.updateVertices(0);
      hull_line_vao.updateFaces();


    } else if(mode == 0){
      generateRandom2DPoints(500, point_vao.data->obj_vertices, glm::vec2(-.3,-.3), glm::vec2(.3,.3));
      point_vao.updateVertices(0);
      point_vao.updateFaces();

      hull_line_vao.data->obj_vertices = quickHull2D(point_vao.data->obj_vertices);
      //giftWrapping(point_vao.data->obj_vertices);

      hull_line_vao.updateVertices(0);
      hull_line_vao.updateFaces();
      visualizeGiftWrapping(hull_line_vao, hull_line_vao.data->obj_vertices);

    } else if(mode == 2){

      std::vector<glm::uvec3> hullFaces;

      giftWrapping3D(input_geo_vao.data->obj_vertices, hullFaces);
      hull_line_vao.data->obj_vertices = input_geo_vao.data->obj_vertices;
      hull_line_vao.data->obj_faces = hullFaces;

      std::cout << hullFaces.size() << std::endl;



      hull_line_vao.updateVertices(0);
      hull_line_vao.updateFaces();

    }
  }

  if(key == GLFW_KEY_P && action == GLFW_PRESS) {
    float x = 2.0f*current_x / window_width - 1.0f;
    float y = 2.0f*current_y / window_height - 1.0f;
    std::cout << x << " " << y << std::endl;

    glm::vec4 mouse_pos4(x, y, 0.0, 1.0);
    glm::vec4 world_coords = glm::inverse(view_projection_matrix) * mouse_pos4;
    hull_line_vao.clearAllData();

    world_coords = world_coords/world_coords.w;

    //project onto xy plane
    glm::vec3 world_c3(world_coords.x, world_coords.y, world_coords.z);

    glm::vec3 eyevec = glm::normalize(world_c3 - eye);
    glm::vec3 norm(0.0f, 0.0f, 1.0f);
    float num = -eye.x*norm.x - eye.y*norm.y - eye.z*norm.z;
    float denom = eyevec.x*norm.x + eyevec.y*norm.y + eyevec.z*norm.z;
    float t = num/denom;
    glm::vec3 final = eye + t*eyevec;

    point_vao.data->obj_vertices.push_back(glm::vec4(final.x, final.y, final.z, 1.0));

    point_vao.updateVertices(0);
  }

  if(key == GLFW_KEY_R && action == GLFW_PRESS) {
    eye = glm::vec3(0.0f, 0.0f, 1.0f);
    look = glm::vec3(0.0f, 0.0f, -1.0f);
    up = glm::vec3(0.0f, 1.0f, 0.0f);
    center = glm::vec3(0,0,0);

  }

  if(key == GLFW_KEY_LEFT && action != GLFW_RELEASE) {
    glm::vec3 tangent = glm::cross(look, up);
    std::cout << glm::to_string(tangent) << std::endl;
    eye += .01f*tangent;
    center += .01f*tangent;



  }

  if(key == GLFW_KEY_RIGHT && action != GLFW_RELEASE) {
    glm::vec3 tangent = glm::cross(look, up);
    eye -= .01f*tangent;
    center -= .01f*tangent;


  }

  if(key == GLFW_KEY_X && action != GLFW_RELEASE) {
    eye += .01f*look;
  }

  if(key == GLFW_KEY_Z && action != GLFW_RELEASE) {
    eye -= .01f*look;
  }

  if(key == GLFW_KEY_UP && action != GLFW_RELEASE) {
    eye += .01f*up;
    center += .01f*up;
  }

  if(key == GLFW_KEY_DOWN && action != GLFW_RELEASE) {
    eye -= .01f*up;
    center -= .01f*up;
  }

  if(key == GLFW_KEY_S && action == GLFW_PRESS) {
    spinning = !spinning;
  }


}



void MousePosCallback(GLFWwindow* window, double mouse_x, double mouse_y) {
  last_x = current_x;
  last_y = current_y;
  current_x = mouse_x;
  current_y = window_height- mouse_y;
  float delta_x = current_x - last_x;
  float delta_y = -(current_y - last_y);
  if (sqrt(delta_x * delta_x + delta_y * delta_y) < 1e-15) return;

  glm::vec3 mouse_direction =
        glm::normalize(glm::vec3(delta_x, delta_y, 0.0f));
  if (drag_state && current_button == GLFW_MOUSE_BUTTON_LEFT) {
        float x = 2.0f*current_x / window_width - 1.0f;
        float y = 2.0f*current_y / window_height - 1.0f;
        std::cout << x << " " << y << std::endl;

        glm::vec4 mouse_pos4(x, y, 0.0, 1.0);
        glm::vec4 world_coords = glm::inverse(view_projection_matrix) * mouse_pos4;
        hull_line_vao.clearAllData();

        world_coords = world_coords/world_coords.w;

        //project onto xy plane
        glm::vec3 world_c3(world_coords.x, world_coords.y, world_coords.z);

        glm::vec3 eyevec = glm::normalize(world_c3 - eye);
        glm::vec3 norm(0.0f, 0.0f, 1.0f);
        float num = -eye.x*norm.x - eye.y*norm.y - eye.z*norm.z;
        float denom = eyevec.x*norm.x + eyevec.y*norm.y + eyevec.z*norm.z;
        float t = num/denom;
        glm::vec3 final = eye + t*eyevec;

        point_vao.data->obj_vertices.push_back(glm::vec4(final.x, final.y, final.z, 1.0));

        point_vao.updateVertices(0);

    // Fix me.
  } else if (drag_state && current_button == GLFW_MOUSE_BUTTON_RIGHT) {
    glm::vec4 world_coords_direction = glm::inverse(view_projection_matrix) * glm::vec4(mouse_direction.x, mouse_direction.y, mouse_direction.z, 1.0f);
    glm::vec3 wcd(world_coords_direction.x/world_coords_direction.w, world_coords_direction.y/world_coords_direction.w, world_coords_direction.z/world_coords_direction.w);
    
    glm::vec3 axis = glm::normalize(glm::cross(look, wcd - center));

    look = glm::normalize(glm::rotate(look, .05f, glm::normalize(axis)));
    up = glm::normalize(glm::rotate(up, .05f, glm::normalize(axis)));

    float camera_distance = glm::distance(eye, center);
    eye = center - look*camera_distance;
    

  }
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
  drag_state = (action == GLFW_PRESS);
  current_button = button;
}


int main(int argc, char* argv[]) {

  // Set up OpenGL context
  if (!glfwInit()) exit(EXIT_FAILURE);
  glfwSetErrorCallback(ErrorCallback);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow* window = glfwCreateWindow(window_width, window_height,
                                        &window_title[0], nullptr, nullptr);
  CHECK_SUCCESS(window != nullptr);
  glfwMakeContextCurrent(window);
  glewExperimental = GL_TRUE;
  CHECK_SUCCESS(glewInit() == GLEW_OK);
  glGetError();  
  glfwSetKeyCallback(window, KeyCallback);
  glfwSetCursorPosCallback(window, MousePosCallback);
  glfwSetMouseButtonCallback(window, MouseButtonCallback);
  glfwSwapInterval(1);
  const GLubyte* renderer = glGetString(GL_RENDERER);  // get renderer string
  const GLubyte* version = glGetString(GL_VERSION);    // version as a string
  const GLubyte* glsl_version =
      glGetString(GL_SHADING_LANGUAGE_VERSION);  

  std::cout << "Renderer: " << renderer << "\n";
  std::cout << "OpenGL version supported:" << version << "\n";
  std::cout << "GLSL version supported:" << glsl_version << "\n";

  std::string vertex_shader_input =
    "#version 330 core\n"
       "in vec3 vertex_position;" 
       "uniform vec3 light_position;" 
       "out vec3 vs_light_direction;" 
       "void main() {"
          "gl_Position = vec4(vertex_position, 1.0);" 
          "vs_light_direction = light_position - vertex_position;" 
       "}";

  std::string geometry_shader_input =
      "#version 330 core\n"
      "layout (triangles) in;" 
      "layout (triangle_strip, max_vertices = 3) out;" 
      "uniform mat4 projection_view;"
      "in vec3 vs_light_direction[];"
      "out vec3 normal;"
      "out vec3 light_direction;"
      "void main() {"
         "int n = 0;"
         "vec3 one = gl_in[0].gl_Position.xyz;"
         "vec3 two = gl_in[1].gl_Position.xyz;"
         "vec3 three = gl_in[2].gl_Position.xyz;"
         "normal = normalize(cross(two - one, three - one));"
         "for (n = 0; n < gl_in.length(); n++) {" 
            "light_direction = vs_light_direction[n];" 
            "gl_Position = projection_view * gl_in[n].gl_Position;"
            "EmitVertex();"
         "}"
         "EndPrimitive();"
      "}";

  std::string fragment_shader_input =
      "#version 330 core\n"
      "in vec3 normal;"
      "in vec3 light_direction;"
      "out vec4 fragment_color;"
      "void main() {"
         "vec4 color = vec4(1.0, 0.0, 0.0, 1.0);" // Red
         "float dot_nl = dot(normalize(light_direction), normalize(normal));" // Compute brightness based on angle between normal and light
         "dot_nl = clamp(dot_nl, 0.0, 1.0);" // Ignore back-facing triangles
         "fragment_color = clamp(dot_nl * color, 0.0, 1.0);"
      "}";      


  std::string vertex_shader =
      "#version 330 core\n"
      "in vec3 vertex_position;"
      "uniform mat4 projection_view;"
      "uniform vec3 eye_pos;"
      "out vec3 global_pos;"
      "void main() {"
         "global_pos = vertex_position;"
         "gl_Position = projection_view*vec4(vertex_position, 1.0);" // Don't transform the vertices at all
      "}";

std::string point_geom_shader = 
    "#version 330 core\n"
    "layout (points) in;" 
    "layout (points, max_vertices = 1) out;" 
    "void main() {"
       "int n = 0;"
       "vec3 one = gl_in[0].gl_Position.xyz;"
       "gl_Position = gl_in[0].gl_Position;"
       "EmitVertex();"
       "EndPrimitive();"
    "}";

std::string line_geom_shader = 
    "#version 330 core\n"
    "layout (lines) in;"
    "layout (line_strip, max_vertices = 2) out;"
    "void main() {"
       "gl_Position = gl_in[0].gl_Position;"
       "EmitVertex();"
       "gl_Position = gl_in[1].gl_Position;"
       "EmitVertex();"
       "EndPrimitive();"
    "}";

std::string point_frag_shader = 
    "#version 330 core\n"
    "out vec4 fragment_color;"
    "void main() {"
       "vec4 color = vec4(1.0, 0.0, 0.0, 1.0);"
       "fragment_color = color;"
    "}";

std::string blue_frag_shader = 
    "#version 330 core\n"
    "out vec4 fragment_color;" 
    "void main() {"
       "vec4 color = vec4(.9, .9, 1, 1.0);"
       "fragment_color = color;"
    "}";


std::string td_geom_shader =
  "#version 330 core\n"
  "layout (triangles) in;" 
  "layout (line_strip, max_vertices = 4) out;"
   "in vec3 global_pos[];"
   "out vec3 gp;"
  "void main() {"
     "gl_Position = gl_in[0].gl_Position;"
      "gp = global_pos[0];"
     "EmitVertex();"
     "gl_Position = gl_in[1].gl_Position;"
     "gp = global_pos[1];"
     "EmitVertex();"
     "gl_Position = gl_in[2].gl_Position;"
     "gp = global_pos[2];"
     "EmitVertex();"
     "gl_Position = gl_in[0].gl_Position;"
     "gp = global_pos[0];"
     "EmitVertex();"
     "EndPrimitive();"
  "}";

std::string td_frag_shader = 
    "#version 330 core\n"
    "out vec4 fragment_color;"
    "uniform vec3 eye_pos;"
    "in vec3 gp;"
    "void main() {"
        "float dist = distance(eye_pos, gp);"
       "vec4 color = vec4(.1, 0.3, .5*dist, .2);"
       "fragment_color = color;"
    "}";




  // Create Vertex Array Object
  GLuint* descriptors = new GLuint[3];
  CHECK_GL_ERROR(glGenVertexArrays(3, descriptors));
  point_vao.vao_descriptor = descriptors[0];
  hull_line_vao.vao_descriptor = descriptors[1];
  input_geo_vao.vao_descriptor = descriptors[2];
  CHECK_GL_ERROR(glBindVertexArray(point_vao.vao_descriptor));

  point_vao.generateBuffers();
  GeometryData* hullPoints = new GeometryData();
  //generateRandom3DPoints(2000, hullPoints->obj_vertices, glm::vec3(-.3,-.3, -.3), glm::vec3(.3,.3, .3));
  generateRandom2DPoints(500, hullPoints->obj_vertices, glm::vec2(-.3, -.3), glm::vec2(.3, .3));


  std::cout << hullPoints->obj_vertices.size() << std::endl;
  point_vao.data = hullPoints;

  point_vao.updateVertices(0);
  point_vao.updateFaces();


  CHECK_GL_ERROR(glBindVertexArray(input_geo_vao.vao_descriptor));

  input_geo_vao.generateBuffers();
  GeometryData* input_geo = new GeometryData();
  std::string file = std::string(argv[1]);
  std::cout << file << std::endl;

  LoadObj(file, input_geo->obj_vertices, input_geo->obj_faces);
  input_geo_vao.data = input_geo;
  input_geo_vao.updateVertices(0);
  input_geo_vao.updateFaces();

  std::vector<glm::uvec3> hullFaces;
  
  std::vector<glm::vec4> hull = giftWrapping(hullPoints->obj_vertices);

  CHECK_GL_ERROR(glBindVertexArray(hull_line_vao.vao_descriptor));
  hull_line_vao.generateBuffers();
  GeometryData* convexHull = new GeometryData();
  convexHull->obj_vertices = hull;

  hull_line_vao.data = convexHull;
  hull_line_vao.updateVertices(0);

  visualizeGiftWrapping(hull_line_vao, hull);

  GLuint v = pointProgram.createVertexShader(vertex_shader);
  GLuint g = pointProgram.createGeometryShader(point_geom_shader);
  GLuint f = pointProgram.createFragmentShader(point_frag_shader);

  pointProgram.link(v, g, f);

  pointProgram.addLocation("projection_view");

  v = hullProgram2D.createVertexShader(vertex_shader);
  g = hullProgram2D.createGeometryShader(line_geom_shader);
  f = hullProgram2D.createFragmentShader(blue_frag_shader);
  hullProgram2D.link(v,g,f);
  hullProgram2D.addLocation("projection_view");
  hullProgram2D.addLocation("eye_pos");

  v = hullProgram3D.createVertexShader(vertex_shader);
  g = hullProgram3D.createGeometryShader(td_geom_shader);
  f = hullProgram3D.createFragmentShader(td_frag_shader);

  hullProgram3D.link(v,g,f);
  hullProgram3D.addLocation("projection_view");
  hullProgram3D.addLocation("eye_pos");

  v = InputGeometryProgram.createVertexShader(vertex_shader_input);
  g = InputGeometryProgram.createGeometryShader(geometry_shader_input);
  f = InputGeometryProgram.createFragmentShader(fragment_shader_input);

  InputGeometryProgram.link(v, g, f);
  InputGeometryProgram.addLocation("projection_view");
  InputGeometryProgram.addLocation("light_position");
  

  eye = glm::vec3(0, 0.0f, 1.0f);
  look = glm::vec3(0.0f, 0.0f, -1.0f);
  up = glm::vec3(0.0f, 1.0f, 0.0f);
  center = glm::vec3(0,0,0);
  glm::vec3 light_position(2.0, .5, 2.0);
  

  float aspect = static_cast<float>(window_width) / window_height;
  projection_matrix =
      glm::perspective(glm::radians(45.0f), aspect, 0.0001f, 1000.0f);
  
  while (!glfwWindowShouldClose(window)) {

    glfwGetFramebufferSize(window, &window_width, &window_height);
    glViewport(0, 0, window_width, window_height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDepthFunc(GL_LESS);

    glm::mat4 trans_view_projection_matrix = projection_matrix*view_matrix;

    if(spinning) {
      spinAmount += 0.015f;
      trans_view_projection_matrix = trans_view_projection_matrix * glm::rotate(spinAmount, glm::vec3(0,1,0));
    }

    view_matrix = glm::lookAt(eye, look, up);
    view_projection_matrix = projection_matrix * view_matrix;

    if(mode == 0 || mode == 1) {
      pointProgram.use();
      pointProgram.addUniformMatrix4fv("projection_view", &trans_view_projection_matrix[0][0]);
      
      point_vao.drawPoints(2);
    } else {
      InputGeometryProgram.use();
      InputGeometryProgram.addUniformMatrix4fv("projection_view", &trans_view_projection_matrix[0][0]);
      InputGeometryProgram.addUniform3fv("light_position", &light_position[0]);
      input_geo_vao.draw();
    }
 
    if(mode == 1 || mode == 2) {
      hullProgram3D.use();
      hullProgram3D.addUniform3fv("eye_pos", &eye[0]);
      hullProgram3D.addUniformMatrix4fv("projection_view", &trans_view_projection_matrix[0][0]);

      hull_line_vao.draw();
    } else if(mode == 0){

      hullProgram2D.use();
      hullProgram2D.addUniformMatrix4fv("projection_view", &trans_view_projection_matrix[0][0]);
      
      hull_line_vao.drawLines();
      
    }

    glfwPollEvents();
    glfwSwapBuffers(window);
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
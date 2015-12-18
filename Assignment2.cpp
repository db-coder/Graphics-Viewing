/*
  CSX75 Assignment 2

  A program which opens a window and draws the "color cube."

  Modified from An Introduction to OpenGL Programming, 
  Ed Angel and Dave Shreiner, SIGGRAPH 2013

  Written by Parag Chaudhuri, 2015
  Further modified by: Dibyendu Mondal and Anand Bhoraskar
*/

#include "Assignment2.hpp"

GLuint shaderProgram;
GLuint vbo[4], vao[4];

glm::mat4 rotation_matrix;
glm::mat4 projection_matrix;
glm::mat4 c_rotation_matrix;
glm::mat4 lookat_matrix;

glm::mat4 model_matrix[3];
glm::mat4 view_matrix;

glm::mat4 wcsMatrix;
glm::mat4 vcsMatrix;
glm::mat4 ccsMatrix;
glm::mat4 ndcsMatrix;
glm::mat4 dcsMatrix;

glm::mat4 modelview_matrix;
GLuint uModelViewMatrix;


std::vector<glm::vec4> v_positions[4];
std::vector<glm::vec4> v_colors[4];

//-----------------------------------------------------------------

//6 faces, 2 triangles/face, 3 vertices/triangle
const int num_vertices = 36;

int tri_idx=0;

//-----------------------------------------------------------------

void populate_vectors(std::string S,int j)
{
  v_positions[j].clear();
  v_colors[j].clear();
  std::fstream file1;
  file1.open(S.c_str(),std::fstream::in);
  for (int i = 0; !file1.eof() ; ++i)
  {
    float x,y,z,r,g,b;
    file1>>x>>y>>z>>r>>g>>b;
    v_positions[j].push_back(glm::vec4(x,y,z,1.0));
    v_colors[j].push_back(glm::vec4(r,g,b,1.0));

  }
  file1.close();
}

void addFrustumLine(glm::vec4 a,glm::vec4 b,glm::vec4 color){
            v_positions[3].push_back(a);
            v_colors[3].push_back(color);
            v_positions[3].push_back(b);
            v_colors[3].push_back(color);

}

void makeFrustum(float ex,float ey,float ez,float lx,float ly,float lz,float ux,float uy,float uz,float L,float R,float T,float B,float N,float F,int objectNo){
  L = -L;
  B = -B;
  glm::vec3 n = glm::normalize(glm::vec3(ex-lx,ey-ly,ez-lz)),
            u = glm::normalize(glm::cross(glm::vec3(ux,uy,uz),n)),
            v = glm::cross(n,u);


  glm::vec4 eye(ex,ey,ez,1),
            leftBottomNear = glm::vec4( (glm::vec3)eye -N*n - L*u - B*v ,1),
            leftBottomFar = (F/N)*(leftBottomNear-eye) + eye,
            leftTopNear = glm::vec4((glm::vec3)eye -N*n - L*u + T*v ,1),
            leftTopFar = (F/N)*(leftTopNear-eye) + eye,
            rightBottomNear = glm::vec4((glm::vec3)eye -N*n + R*u - B*v ,1),
            rightBottomFar = (F/N)*(rightBottomNear-eye) + eye,
            rightTopNear = glm::vec4((glm::vec3)eye -N*n + R*u + T*v ,1),
            rightTopFar = (F/N)*(rightTopNear-eye) + eye,
            red = glm::vec4(1,0,0,1),
            magenta = glm::vec4(1,0,1,1),
            cyan = glm::vec4(0,1,1,1);

  addFrustumLine(eye,eye,red);
  addFrustumLine(eye,leftBottomNear,magenta);
  addFrustumLine(eye,leftTopNear,magenta);
  addFrustumLine(eye,rightBottomNear,magenta);
  addFrustumLine(eye,rightTopNear,magenta);
  addFrustumLine(leftBottomNear,leftBottomFar,cyan);
  addFrustumLine(leftTopNear,leftTopFar,cyan);
  addFrustumLine(rightBottomNear,rightBottomFar,cyan);
  addFrustumLine(rightTopNear,rightTopFar,cyan);
  addFrustumLine(leftBottomNear,rightBottomNear,cyan);
  addFrustumLine(rightBottomNear,rightTopNear,cyan);
  addFrustumLine(rightTopNear,leftTopNear,cyan);
  addFrustumLine(leftTopNear,leftBottomNear,cyan);
  addFrustumLine(leftBottomFar,rightBottomFar,cyan);
  addFrustumLine(rightBottomFar,rightTopFar,cyan);
  addFrustumLine(rightTopFar,leftTopFar,cyan);
  addFrustumLine(leftTopFar,leftBottomFar,cyan);
}

void setViewMatrices(float ex,float ey,float ez,float lx,float ly,float lz,float ux,float uy,float uz,float L,float R,float T,float B,float N,float F){

glm::vec3 n = glm::normalize(glm::vec3(ex-lx,ey-ly,ez-lz)),
            u = glm::normalize(glm::cross(glm::vec3(ux,uy,uz),n)),
            v = glm::cross(n,u);

wcsMatrix = glm::mat4(1,0,0,0,
                     0,1,0,0,
                     0,0,1,0,
                     0,0,0,1);
vcsMatrix = glm::mat4(u.x,v.x,n.x,0,
                     u.y,v.y,n.y,0,
                     u.z,v.z,n.z,0,
                     -1*glm::dot(u,glm::vec3(ex,ey,ez)),-1*glm::dot(v,glm::vec3(ex,ey,ez)),-1*glm::dot(n,glm::vec3(ex,ey,ez)),1);
ccsMatrix = glm::mat4(2*N/(R-L),0,0,0,
                     0,2*N/(T-B),0,0,
                     (R+L)/(R-L),(T+B)/(T-B),-(F+N)/(F-N),-1,
                     0,0,-2*F*N/(F-N),0);
ndcsMatrix = glm::mat4(1,0,0,0,
                     0,1,0,0,
                     0,0,1,0,
                     0,0,0,1);
dcsMatrix = glm::mat4(2,0,0,0,
                     0,2,0,0,
                     0,0,1,0,
                     0,0,0,1);
}

glm::mat4 getViewMatrix(){
  if(csStage == 1)
    return wcsMatrix;
  else if(csStage == 2){
    return vcsMatrix*wcsMatrix;
  }
  else if(csStage == 3){
    return ccsMatrix*vcsMatrix*wcsMatrix;
  }
  else if(csStage == 4){
    return ndcsMatrix*ccsMatrix*vcsMatrix*wcsMatrix;
  }
  else{ 
    return dcsMatrix*ndcsMatrix*ccsMatrix*vcsMatrix*wcsMatrix;
  }
  return dcsMatrix*ndcsMatrix*ccsMatrix*vcsMatrix*wcsMatrix;
}


void bindObjects(int objectNo, GLuint vPosition, GLuint vColor){
    //Set 0 as the current array to be used by binding it
  glBindVertexArray (vao[objectNo]);
  //Set 0 as the current buffer to be used by binding it
  glBindBuffer (GL_ARRAY_BUFFER, vbo[objectNo]);
  //Copy the points into the current buffer
  glBufferData (GL_ARRAY_BUFFER, sizeof(glm::vec4)*(v_positions[objectNo].size() + v_colors[objectNo].size()), NULL, GL_STATIC_DRAW);
  glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(glm::vec4)*(v_positions[objectNo].size()), &v_positions[objectNo][0] );

  glBufferSubData( GL_ARRAY_BUFFER, sizeof(glm::vec4)*(v_positions[objectNo].size()), sizeof(glm::vec4)*(v_colors[objectNo].size()), &v_colors[objectNo][0]);
  // set up vertex array

  glEnableVertexAttribArray( vPosition);
  glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );

  glEnableVertexAttribArray( vColor );
  glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(glm::vec4)*(v_positions[objectNo].size())) );
}

void drawObject(int objectNo){
  //Creating the lookat and the up vectors for the camera
  c_rotation_matrix = glm::rotate(glm::mat4(1.0f), glm::radians(c_xrot), glm::vec3(1.0f,0.0f,0.0f));
  c_rotation_matrix = glm::rotate(c_rotation_matrix, glm::radians(c_yrot), glm::vec3(0.0f,1.0f,0.0f));
  c_rotation_matrix = glm::rotate(c_rotation_matrix, glm::radians(c_zrot), glm::vec3(0.0f,0.0f,1.0f));

  glm::vec4 c_pos = glm::vec4(c_xpos,c_ypos,c_zpos, 1.0)*c_rotation_matrix;
  glm::vec4 c_up = glm::vec4(c_up_x,c_up_y,c_up_z, 1.0)*c_rotation_matrix;
  //Creating the lookat matrix
  lookat_matrix = glm::lookAt(glm::vec3(c_pos),glm::vec3(0.0),glm::vec3(c_up));

  //creating the projection matrix
  if(enable_perspective)
    projection_matrix = glm::frustum(-1.0, 1.0, -1.0, 1.0, 1.0, 5.0);
    //projection_matrix = glm::perspective(glm::radians(90.0),1.0,0.1,5.0);
  else
    projection_matrix = glm::ortho(-2.0, 2.0, -2.0, 2.0, -5.0, 5.0);

  view_matrix = getViewMatrix();

  modelview_matrix = projection_matrix*lookat_matrix*view_matrix*model_matrix[objectNo];
  glUniformMatrix4fv(uModelViewMatrix, 1, GL_FALSE, glm::value_ptr(modelview_matrix));
  glBindVertexArray (vao[objectNo]);
  glDrawArrays(GL_TRIANGLES, 0, v_positions[objectNo].size());
}

//-------------------------------------------------------------------------------------

void drawFrustum(int objectNo){
  //Creating the lookat and the up vectors for the camera
  c_rotation_matrix = glm::rotate(glm::mat4(1.0f), glm::radians(c_xrot), glm::vec3(1.0f,0.0f,0.0f));
  c_rotation_matrix = glm::rotate(c_rotation_matrix, glm::radians(c_yrot), glm::vec3(0.0f,1.0f,0.0f));
  c_rotation_matrix = glm::rotate(c_rotation_matrix, glm::radians(c_zrot), glm::vec3(0.0f,0.0f,1.0f));

  glm::vec4 c_pos = glm::vec4(c_xpos,c_ypos,c_zpos, 1.0)*c_rotation_matrix;
  glm::vec4 c_up = glm::vec4(c_up_x,c_up_y,c_up_z, 1.0)*c_rotation_matrix;
  //Creating the lookat matrix
  lookat_matrix = glm::lookAt(glm::vec3(c_pos),glm::vec3(0.0),glm::vec3(c_up));

  //creating the projection matrix
  if(enable_perspective)
    projection_matrix = glm::frustum(-1.0, 1.0, -1.0, 1.0, 1.0, 5.0);
    //projection_matrix = glm::perspective(glm::radians(90.0),1.0,0.1,5.0);
  else
    projection_matrix = glm::ortho(-2.0, 2.0, -2.0, 2.0, -5.0, 5.0);

  view_matrix = getViewMatrix();

  modelview_matrix = projection_matrix*lookat_matrix*view_matrix;
  glUniformMatrix4fv(uModelViewMatrix, 1, GL_FALSE, glm::value_ptr(modelview_matrix));
  glBindVertexArray (vao[objectNo]);
  glDrawArrays(GL_LINES, 0, v_positions[objectNo].size());
}
//-------------------------------------------------------------------------------------
glm::mat4 getModelMatrix(float sx,float sy,float sz,float rx,float ry,float rz,float rt,float tx,float ty,float tz){
  glm::mat4 scaleMat(sx,0,0,0,
                    0,sy,0,0,
                    0,0,sz,0,
                    0,0,0,1)

            ,rotateX(1,0,0,0,
                     0,cos(rx*3.14/180),sin(rx*3.14/180),0,
                     0,-sin(rx*3.14/180),cos(rx*3.14/180),0,
                     0,0,0,1)
  
            ,rotateY(cos(ry*3.14/180),0,-sin(ry*3.14/180),0,
                     0,1,0,0,
                     sin(ry*3.14/180),0,cos(ry*3.14/180),0,
                     0,0,0,1)
  
            ,rotateZ(cos(rz*3.14/180),sin(rz*3.14/180),0,0,
                     -sin(rz*3.14/180),cos(rz*3.14/180),0,0,
                     0,0,1,0,
                     0,0,0,1)
  
       ,translateMat(1,0,0,0,
                     0,1,0,0,
                     0,0,1,0,
                     tx,ty,tz,1);
  return translateMat*scaleMat*rotateZ*rotateY*rotateX;          
}

void initBuffersGL(void)
{
  //Filestream Buffer
  std::fstream file;
  file.open("myscene.scn",std::fstream::in);

  std::string S;
  float sx,sy,sz,rx,ry,rz,rt,tx,ty,tz;

  //1st model
  file>>S;
  populate_vectors(S,0);
  file>>sx>>sy>>sz>>rx>>ry>>rz>>tx>>ty>>tz;
  model_matrix[0] = getModelMatrix(sx,sy,sz,rx,ry,rz,rt,tx,ty,tz); 

  //2nd model
  file>>S;
  populate_vectors(S,1);
  file>>sx>>sy>>sz>>rx>>ry>>rz>>tx>>ty>>tz;
  model_matrix[1] = getModelMatrix(sx,sy,sz,rx,ry,rz,rt,tx,ty,tz);
  
  //3rd model
  file>>S;
  populate_vectors(S,2);
  file>>sx>>sy>>sz>>rx>>ry>>rz>>tx>>ty>>tz;
  model_matrix[2] = getModelMatrix(sx,sy,sz,rx,ry,rz,rt,tx,ty,tz);

  //Frustum
  float L,R,T,B,N,F;
  file>>sx>>sy>>sz>>rx>>ry>>rz>>tx>>ty>>tz>>L>>R>>T>>B>>N>>F;
  makeFrustum(sx,sy,sz,rx,ry,rz,tx,ty,tz,L,R,T,B,N,F,3);

  setViewMatrices(sx,sy,sz,rx,ry,rz,tx,ty,tz,L,R,T,B,N,F);

  file.close();

  // Load shaders and use the resulting shader program
  std::string vertex_shader_file("vshader.glsl");
  std::string fragment_shader_file("fshader.glsl");

  std::vector<GLuint> shaderList;
  shaderList.push_back(csX75::LoadShaderGL(GL_VERTEX_SHADER, vertex_shader_file));
  shaderList.push_back(csX75::LoadShaderGL(GL_FRAGMENT_SHADER, fragment_shader_file));

  shaderProgram = csX75::CreateProgramGL(shaderList);
  glUseProgram( shaderProgram );

  // getting the attributes from the shader program
  GLuint vPosition = glGetAttribLocation( shaderProgram, "vPosition" );
  GLuint vColor = glGetAttribLocation( shaderProgram, "vColor" ); 
  uModelViewMatrix = glGetUniformLocation( shaderProgram, "uModelViewMatrix");

  //Ask GL for 4 Vertex Attribute Objects (vao)
  glGenVertexArrays (4, vao);
  //Ask GL for 4 Vertex Buffer Object (vbo)
  glGenBuffers (4, vbo);


  // Object ------------------
  bindObjects(0,vPosition,vColor);
  bindObjects(1,vPosition,vColor);
  bindObjects(2,vPosition,vColor);
  bindObjects(3,vPosition,vColor);
}

void renderGL(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  drawObject(0);
  drawObject(1);
  drawObject(2);
  drawFrustum(3);
}

int main(int argc, char** argv)
{
  //! The pointer to the GLFW window
  GLFWwindow* window;

  //! Setting up the GLFW Error callback
  glfwSetErrorCallback(csX75::error_callback);

  //! Initialize GLFW
  if (!glfwInit())
    return -1;

  //We want OpenGL 4.0
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); 
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
  //This is for MacOSX - can be omitted otherwise
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
  //We don't want the old OpenGL 
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 

  //! Create a windowed mode window and its OpenGL context
  window = glfwCreateWindow(1024, 1024, "CS475/CS675 Assignment 2: Viewing", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    return -1;
  }
  
  //! Make the window's context current 
  glfwMakeContextCurrent(window);

  //Initialize GLEW
  //Turn this on to get Shader based OpenGL
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if (GLEW_OK != err)
  {
    //Problem: glewInit failed, something is seriously wrong.
    std::cerr<<"GLEW Init Failed : %s"<<std::endl;
  }

  //Print and see what context got enabled
  std::cout<<"Vendor: "<<glGetString (GL_VENDOR)<<std::endl;
  std::cout<<"Renderer: "<<glGetString (GL_RENDERER)<<std::endl;
  std::cout<<"Version: "<<glGetString (GL_VERSION)<<std::endl;
  std::cout<<"GLSL Version: "<<glGetString (GL_SHADING_LANGUAGE_VERSION)<<std::endl;

  //Keyboard Callback
  glfwSetKeyCallback(window, csX75::key_callback);
  //Framebuffer resize callback
  glfwSetFramebufferSizeCallback(window, csX75::framebuffer_size_callback);

  // Ensure we can capture the escape key being pressed below
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  //Initialize GL state
  csX75::initGL();
  initBuffersGL();

  // Loop until the user closes the window
  while (glfwWindowShouldClose(window) == 0)
  {     
    // Render here
    renderGL();

    // Swap front and back buffers
    glfwSwapBuffers(window);
    
    // Poll for and process events
    glfwPollEvents();
  }
  
  glfwTerminate();
  return 0;
}
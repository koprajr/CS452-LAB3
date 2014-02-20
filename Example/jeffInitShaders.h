#ifndef INITSHADERS_H_
#define INITSHADERS_H_
//functions for loading in shaders
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <cstdio>
#include <iostream>

using namespace std;

GLuint createShader(GLenum type, const GLchar* shadeSource);
const GLchar* inputShader(const char* filename);
GLuint createProgram(const vector<GLuint> shadeList);
void transform(GLuint program); // NEW

typedef struct{
  GLenum type;
  const char* filename;
} ShaderInfo;

// Create shaders
GLuint initShaders(ShaderInfo* shaders){
	
	vector<GLuint> Shadelist;
	
	for(ShaderInfo* shade=shaders; shade->type != GL_NONE; shade++)
		Shadelist.push_back(createShader(shade->type,inputShader(shade->filename))); 
		// push back shaders onto list

	GLuint program=createProgram(Shadelist); //creates the program linking to all the shaders
		
		glUseProgram(program);

	glm::mat4 view;
	view = glm::lookAt(//position and direction of camera
		glm::vec3(0.0f, 0.0f, 50.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
	
	GLint tempLoc = glGetUniformLocation(program, "viewMatrix");//handles the camera movement
	glUniformMatrix4fv(tempLoc, 1, GL_FALSE, &view[0][0]);
  
	glm::mat4 mainProjMatrix;
	mainProjMatrix = glm::perspective(57.0,1.0,.1,500.0);//Matrix that handle the orthographic or perspective viewing
	tempLoc = glGetUniformLocation(program, "Matrix");
	glUniformMatrix4fv(tempLoc, 1, GL_FALSE, &mainProjMatrix[0][0]);
  
	return program;
}

// Opens and reads shaders
const GLchar* inputShader(const char* filename){

  FILE* fshade = fopen(filename, "rb");//opens file
  
  if(!fshade){//check to see if file is opened
    fprintf(stderr,"unable to open file '%s'\n",filename);
    return NULL;
  }
  
  //neat way to get the length of the file
  fseek(fshade, 0, SEEK_END);
  long filesize=ftell(fshade);
  fseek(fshade, 0, SEEK_SET);
  
  
  //allocates memory for the file and read in the file 
  GLchar* shadingSource= new GLchar[filesize+1];//
  fread(shadingSource, 1, filesize, fshade);
  
  
  if(ftell(fshade) == 0){//checks to see if the file is empty
    fprintf(stderr, "File '%s' is empty.\n",filename);
    return NULL;
  }

  fclose(fshade);//closes file
  
  shadingSource[filesize] = 0;//neat way to set a '\0' at end of file
  
  return const_cast<const GLchar*>(shadingSource);//overloads the const so the value with change per file  
}

// Creates a shader
GLuint createShader(GLenum type, const GLchar* shadeSource){
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &shadeSource, NULL);

	glCompileShader(shader); // Compile a shader

	GLint compileStatus; // compilation status
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

	// Error handling
	if(!compileStatus){
		GLint logSize; //size of the debug info
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);

		GLchar* infoLog = new GLchar[logSize+1];
		glGetShaderInfoLog(shader,logSize,&logSize,infoLog);
		const char *shadeInfo= NULL;
		switch(type){
			case GL_VERTEX_SHADER: shadeInfo = "vertex"; break;
			case GL_GEOMETRY_SHADER_EXT: shadeInfo = "geometric"; break;
			case GL_FRAGMENT_SHADER: shadeInfo = "fragment"; break;
		}
		fprintf(stderr,"\nCompile failure in %u shader: %s\n Error message:\n%s\n",type,shadeInfo,infoLog);//prints information need to debug shaders
		delete[] infoLog;//manage memory
	}
	return shader;
}

GLuint createProgram(const vector<GLuint> shadeList){
	GLuint program = glCreateProgram(); // create program
	
	for(GLuint i=0;i<shadeList.size();i++){
		glAttachShader(program,shadeList[i]);
	} // attach shaders to program
	
	glBindAttribLocation(program, 0, "in_position");
	glBindAttribLocation(program, 1, "in_color");
	glLinkProgram(program);
	GLint linkStatus;
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

	if(!linkStatus){
		GLint logSize;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
		GLchar *infoLog = new GLchar[logSize+1];
		glGetProgramInfoLog(program,logSize,&logSize,infoLog);
		fprintf(stderr,"\nShader linking failed: %s\n",infoLog);
		delete[] infoLog;
		for(GLuint i=0;i<shadeList.size();i++){
			glDeleteShader(shadeList[i]);
		}
	}
	return program;
}
#endif

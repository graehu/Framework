#include "shader.h"
#include <string>
#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/gl.h>

#include <cstdio>

using namespace std; // Include the standard namespace

/*
	textFileRead loads in a standard text file from a given filename and
	then returns it as a string.
*/

ofstream outfile("shaderOutput.txt");

static string textFileRead(const char *fileName) 
{
	string fileString = string(); // A string for storing the file contents
	string line = string(); // A string for holding the current line

	ifstream file(fileName); // Open an input stream with the selected file
	if (file.is_open()) { // If the file opened successfully
		while (!file.eof()) { // While we are not at the end of the file
			getline(file, line); // Get the current line
		  	fileString.append(line); // Append the line to our file string
			fileString.append("\n"); // Appand a new line character
		}
		file.close(); // Close the file
	}

    return fileString; // Return our string
}

/*
	Given a shader and the filename associated with it, validateShader will
	then get information from OpenGl on whether or not the shader was compiled successfully
	and if it wasn't, it will output the file with the problem, as well as the problem.
*/
static void validateShader(GLuint _shader, const char* _file = 0) 
{
    const unsigned int BUFFER_SIZE = 512;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    GLsizei length = 0;
    
    glGetShaderInfoLog(_shader, BUFFER_SIZE, &length, buffer); // Ask OpenGL to give us the log associated with the shader
    if (length > 0) // If we have any information to display
		//fprintf(output,"Shader: %i (%s) compile error: %s\n",shader,file,buffer);
       //cout << "Shader " << shader << " (" << (file?file:"") << ") compile error: " << buffer << endl; // Output the information
	   outfile << "Shader " << _shader << " (" << (_file?_file:"") << ") compile error: " << buffer << endl; // Output the information
}

/*
	Given a shader program, validateProgram will request from OpenGL, any information
	related to the validation or linking of the program with it's attached shaders. It will
	then output any issues that have occurred.
*/
static void validateProgram(GLuint program) 
{
    const unsigned int BUFFER_SIZE = 512;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    GLsizei length = 0;
    
    glGetProgramInfoLog(program, BUFFER_SIZE, &length, buffer); // Ask OpenGL to give us the log associated with the program
    if (length > 0) // If we have any information to display
		//fprintf(output,"Program %i link error: %s\n", program, buffer);
        outfile << "Program " << program << " link error: " << buffer << endl; // Output the information
    
    glValidateProgram(program); // Get OpenGL to try validating the program
    GLint status;
    glGetProgramiv(program, GL_VALIDATE_STATUS, &status); // Find out if the shader program validated correctly
    if (status == GL_FALSE) // If there was a problem validating
		//fprintf(output,"Error validating shader %i\n", program);
		outfile << "Error validating shader " << program << endl; // Output which program had the error
}

/**
	Default constructor for the Shader class, at the moment it does nothing
*/
shader::shader() 
{
	inited = false;
	//output = fopen("shaderOutput.txt","w");
}

/**
	Constructor for a Shader object which creates a GLSL shader based on a given
	vertex and fragment shader file.
*/
shader::shader(const char *vsFile, const char *gsFile, const char *fsFile) 
{
    inited = false; // Declare we have not initialized the shader yet
	//output = fopen("shaderOutput.txt","w");
	//fprintf(output,"opening file\n");
	outfile << "opening file" << endl;
    
    init(vsFile, gsFile, fsFile); // Initialize the shader
}
/**
	init will take a vertex shader file and fragment shader file, and then attempt to create a valid
	shader program from these. It will also check for any shader compilation issues along the way.
*/
void shader::init(const char *vsFile, const char *gsFile, const char *fsFile)
{
    if (inited) // If we have already initialized the shader
        return;
	outfile << "Shader output file." << endl;

    inited = true; // Mark that we have initialized the shader
    
    shader_vp = glCreateShader(GL_VERTEX_SHADER); // Create a vertex shader
	shader_gp = glCreateShader(GL_GEOMETRY_SHADER); // Create a geomtry shader
    shader_fp = glCreateShader(GL_FRAGMENT_SHADER); // Create a fragment shader
    
    string vsText = textFileRead(vsFile); // Read in the vertex shader
	string gsText = textFileRead(gsFile); // Read in the geometry shader
    string fsText = textFileRead(fsFile); // Read in the fragment shader
    
	const char* vertexText = vsText.c_str();
	const char* geometryText = gsText.c_str();
	const char* fragmentText = fsText.c_str();

    if (vertexText == NULL || fragmentText == NULL || geometryText == NULL) // If either the vertex or fragment or geomtry shader wouldn't load
	{ 
        outfile << "Either vertex or fragment or geometry shader file not found." << endl; // Output the error
		//fprintf(output,"Either vertex or fragment or geometry shader file not found.\n");
        return;
    }
    
    glShaderSource(shader_vp, 1, &vertexText, 0); // Set the source for the vertex shader to the loaded text
    glCompileShader(shader_vp); // Compile the vertex shader
    validateShader(shader_vp, vsFile); // Validate the vertex shader

	glShaderSource(shader_gp, 1, &geometryText, 0); // Set the source for the geometry shader to the loaded text
    glCompileShader(shader_gp); // Compile the geometry shader
    validateShader(shader_gp, gsFile); // Validate the geometry shader
    
    glShaderSource(shader_fp, 1, &fragmentText, 0); // Set the source for the fragment shader to the loaded text
    glCompileShader(shader_fp); // Compile the fragment shader
    validateShader(shader_fp, fsFile); // Validate the fragment shader
    
    shader_id = glCreateProgram(); // Create a GLSL program
	glAttachShader(shader_id, shader_vp); // Attach a vertex shader to the program
	glAttachShader(shader_id, shader_gp); // Attach a geometry shader to the program
    glAttachShader(shader_id, shader_fp); // Attach the fragment shader to the program

	glBindAttribLocation(shader_id, 0, "vPosition"); // Bind a constant attribute location for positions of vertices
	//glBindAttribLocation(shader_id, 1, "in_Color"); // Bind another constant attribute location, this time for color
	glBindAttribLocation(shader_id, 1, "vNormal"); // Bind another constant attribute location, this time for normal
	glBindAttribLocation(shader_id, 2, "vTexCoords");

    glLinkProgram(shader_id); // Link the vertex and fragment shaders in the program
    validateProgram(shader_id); // Validate the shader program
	//outfile << " " << shader << " (" << (file?file:"") << ") compile error: " << buffer << endl;
	//fprintf(output,"this does work\n");
}

/**
	Deconstructor for the Shader object which cleans up by detaching the shaders, deleting them
	and finally deleting the GLSL program.
*/
shader::~shader() 
{
	glDetachShader(shader_id, shader_fp); // Detach the fragment shader
	glDetachShader(shader_id, shader_gp); // Detach the fragment shader
    glDetachShader(shader_id, shader_vp); // Detach the vertex shader
    
    glDeleteShader(shader_fp); // Delete the fragment shader
	glDeleteShader(shader_gp); // Delete the fragment shader
    glDeleteShader(shader_vp); // Delete the vertex shader
    glDeleteProgram(shader_id); // Delete the shader program
	//fclose(output);
}

/**
	id returns the integer value associated with the shader program
*/
unsigned int shader::id() {
    return shader_id; // Return the shaders identifier
}

/**
	bind attaches the shader program for use by OpenGL
*/
void shader::bind() { 
    glUseProgram(shader_id);
}

/**
	unbind deattaches the shader program from OpenGL
*/
void shader::unbind() {
    glUseProgram(0);
}
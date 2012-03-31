//texture geometry shader.
#version 150

precision highp float;

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec4 pass1_vPosition[];
in vec4 pass1_vNormal[];
in vec2 pass1_vTexCoords[];

out vec4 pass2_vPosition;
out vec4 pass2_vNormal;
out vec2 pass2_vTexCoords;


void main(void)
{
	int n;
	pass2_vPosition = pass1_vPosition[0];
	gl_Position = pass1_vPosition[0];
	pass2_vNormal = pass1_vNormal[0];
	pass2_vTexCoords = pass1_vTexCoords[0];
	
    for (n = 0; n < gl_in.length(); n++) 
    {
      	pass2_vPosition = pass1_vPosition[n];
      	gl_Position = pass1_vPosition[n];
		pass2_vNormal = pass1_vNormal[n];
      	pass2_vTexCoords = pass1_vTexCoords[n];
        
		EmitVertex();
    }
    EndPrimitive();
}
//test vertex shader
#version 150 core



uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexCoords;

out vec4 pass1_vPosition;
out vec4 pass1_vNormal;
out vec2 pass1_vTexCoords;

void main(void)
{

    pass1_vPosition = projectionMatrix *viewMatrix * modelMatrix * vec4(vPosition, 1.0);
    gl_Position = pass1_vPosition;
    
    
    pass1_vNormal = projectionMatrix *viewMatrix * modelMatrix * vec4(vNormal, 1.0);
   

    pass1_vTexCoords = vTexCoords;
         

}


// Get surface normal in eye coordinates
    //vec3 normalMatrix;
    //vec3 vEyeNormal = normalMatrix * vNormal;

    // Get vertex position in eye coordinates
    //vec4 vPosition4 = viewMatrix * vPosition;
    //vec3 vPosition3 = vPosition4.xyz / vPosition4.w;

    // Get vector to light source
    //vec3 vLightDir = normalize(vLightPosition - vPosition3);

    // Dot product gives us diffuse intensity
    //pass1_vColor = vec4(0.0, 0.0, 0.6, 1.0) * max(0.0, dot(vEyeNormal, vLightDir));
		
#version 150

in vec3 position;
in vec4 color;
out vec4 col;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

void main()
{	
  // students need to implement this
  // compute the transformed and projected vertex position (into gl_Position) 
    gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f);
  // compute the vertex color (into col)
  col = color;
}


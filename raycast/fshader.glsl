#version 150

in vec2 texCoord;

out vec4  fColor;

uniform sampler2D color_texture;

void main() 
{ 
	fColor = texture(color_texture, texCoord);
} 


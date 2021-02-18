#version 330 core

layout (location = 0) in vec2 v_position;

out vec2 v_coord;

void main(){
	v_coord = v_position*0.5+0.5;
	gl_Position = vec4(v_position, 0.0, 1.0);
}

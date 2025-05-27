#version 330 core
out vec3 FragColor;

uniform vec3 pickingColor;

void main() {
    FragColor = pickingColor;
}
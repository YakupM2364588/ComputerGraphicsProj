#version 330 core

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};


in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 Color;

out vec4 FragColor;

uniform vec3 viewPos;
uniform sampler2D diffuse_texture;
uniform float shininess;

#define LIGHTS 1

uniform Light lights[LIGHTS];

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = vec3(0.0);

    vec3 texColor = texture(diffuse_texture, TexCoords).rgb;

    //Check als texColor bestaat
    vec3 finalColor = (texColor == vec3(0.0f)) ? Color : texColor;

    for(int i = 0; i < LIGHTS; i++)
    {
        vec3 lightDir = normalize(lights[i].position - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);

        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

        float distance = length(lights[i].position - FragPos);
        // 1 / ax²+bx+c
        float weakening = 1.0 / (lights[i].constant +
        lights[i].linear * distance +
        lights[i].quadratic * (distance * distance));

        vec3 ambient  = lights[i].ambient * finalColor;
        vec3 diffuse  = lights[i].diffuse * diff * finalColor;
        vec3 specular = lights[i].specular * spec;

        ambient  *= weakening;
        diffuse  *= weakening;
        specular *= weakening;

        result += ambient + diffuse + specular;
    }

    FragColor = vec4(result, 1.0);
}
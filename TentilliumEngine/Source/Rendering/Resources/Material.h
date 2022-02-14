#pragma once
#include <glm.hpp>
#include "Resource.h"

struct Material : Resource<Material>
{
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec3 specular;
    float specular_power;
    Material(glm::vec4 ambient, glm::vec4 diffuse, glm::vec3 specular)
        : ambient(ambient), diffuse(diffuse), specular(specular), specular_power(0)
    { 

    }
};

const Material* Resource<Material>::Load(const char* filepath);
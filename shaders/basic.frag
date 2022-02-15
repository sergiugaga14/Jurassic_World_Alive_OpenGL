#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

out vec4 fColor;
//fog
uniform float fogDensity;
//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 spotLightDir;
uniform vec3 spotLightPos;
uniform int initSpot;
vec3 spotLightColor= vec3(1.0f,0.0f,0.0f);


// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

//components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float constant = 1.0f;
float linear = 0.045f;
float quadratic = 0.0075f;
float shininess = 64.0f;



float computeFog()
{
 
 vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
 float fragmentDistance = length(fPosEye);
 float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

 return clamp(fogFactor, 0.0f, 1.0f);
}

float computeShadow()
{
// perform perspective divide
vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

// Transform to [0,1] range
normalizedCoords = normalizedCoords * 0.5 + 0.5;

// Get closest depth value from light's perspective
float closestDepth = texture(shadowMap, normalizedCoords.xy).r;

// Get depth of current fragment from light's perspective
float currentDepth = normalizedCoords.z;

//Check wheter current frag pos is in shadow
float bias= 0.005f;
float shadow= currentDepth - bias > closestDepth ? 1.0 : 0.0;
if (normalizedCoords.z > 1.0f)
return 0.0f;

return shadow;
}

vec3 computeSpotLight()
{

 vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
vec3 normalEye = normalize(normalMatrix * fNormal);

//compute light direction
vec3 lightDirN = normalize(spotLightPos - fPosition);

//compute view direction
vec3 viewDirN = normalize( - fPosition);

//compute half vector
vec3 halfVector = normalize(lightDirN + viewDirN);

//compute specular light
float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
vec3 specular = specularStrength * specCoeff * spotLightColor;

//compute distance to light
float dist = length(spotLightPos - fPosition);
//compute attenuation
float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));


	
//compute ambient light
vec3 ambient = att * ambientStrength * spotLightColor;
//compute diffuse light
vec3 diffuse = att * max(dot(normalEye, lightDirN), 0.0f) * spotLightColor;
specular = att * specularStrength * specCoeff * spotLightColor;

return (ambient+diffuse+specular);

}


void computeDirLight()
{
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(- fPosEye.xyz);

    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor;
    //return ambient+diffuse+specular;
}

void main() 
{
   computeDirLight();
    vec3 result=computeSpotLight();

    /*//compute final vertex color
    vec3 color = min((ambient + diffuse) * texture(diffuseTexture, fTexCoords).rgb + specular * texture(specularTexture, fTexCoords).rgb, 1.0f);*/

        ambient *= texture(diffuseTexture, fTexCoords).rgb;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb;
	specular *= texture(specularTexture, fTexCoords).rgb;

   //modulate with shadow
	float shadow = computeShadow();
	vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f);  
     

    //fog color 
    float fogFactor = computeFog();
    vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
if(initSpot==1)
    color=color+result;
    vec4 coloraux=vec4(color,1.0f);

    fColor = fogColor * (1 - fogFactor) + coloraux * fogFactor;
    
   // fColor = vec4(color, 1.0f);
}

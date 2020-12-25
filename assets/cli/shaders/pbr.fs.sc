$input v_position, v_color0, v_normal

#define PI 3.1415926538

// ------------------------------------------------------------------------------------------------ material

// https://learnopengl.com/PBR/Theory

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
  
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 F_r(vec3 V, vec3 L, vec3 N, Material material)
{
    vec3 H = normalize(V + L);
    
    // cook-torrance brdf
    float NDF = DistributionGGX(N, H, material.roughness);        
    float G   = GeometrySmith(N, V, L, material.roughness);
    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, material.albedo, material.metallic);
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - material.metallic;	 
    
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    vec3 specular     = numerator / max(denominator, 0.001);  
    
    // lambert bdrf
    vec3 diffuse = kD * material.albedo / PI;
    
    return diffuse + specular;
}


// ------------------------------------------------------------------------------------------------ lights

uniform vec4 u_camera_position;

// https://learnopengl.com/PBR/Theory

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
  
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 F_r(vec3 V, vec3 L, vec3 N)
{
    vec3 H = normalize(V + L);
    
    // cook-torrance brdf
    float NDF = DistributionGGX(N, H, material_roughness());        
    float G   = GeometrySmith(N, V, L, material_roughness());
    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, material_albedo(), material_metallic());
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - material_metallic();	 
    
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    vec3 specular     = numerator / max(denominator, 0.001);  
    
    // lambert bdrf
    vec3 diffuse = kD * material_albedo() / PI;
    
    return diffuse + specular;
}

vec3 light_pos(int i)
{
    int mat_id = (i * 6) / (3 * 3);  
    int pos_id = (i * 6) % (3 * 3);

    return u_lights[mat_id][pos_id];
}

vec3 light_col(int i)
{
    int mat_id = (i * 6 + 3) / (3 * 3);
    int pos_id = (i * 6 + 3) % (3 * 3);
   
    return u_lights[mat_id][pos_id];
}

void main()
{
    vec3 V = vec3(u_camera_position) - v_position;
    vec3 p = v_position;
    vec3 N = v_normal;

    vec3 L_o = vec3(0.0);

    uint lights_num = floatBitsToInt(u_lights_num[0]);
    for (int i = 0; i < u_lights_num; i++)
    {
        vec3 L_pos = light_pos(i);
        vec3 L_col = light_col(i);

        vec3 L            = normalize(L_pos - p);
        float L_dist      = length(L_pos - p);
        float attenuation = 1.0 / (L_dist * L_dist);
        vec3 radiance     = L_col * attenuation;
        
        float NdotL = max(dot(N, L), 0.0);
        L_o += F_r(V, L, N) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03) * material_albedo() * material_ao();
    vec3 color = ambient + L_o;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    gl_FragColor = vec4(color, 1.0);
}

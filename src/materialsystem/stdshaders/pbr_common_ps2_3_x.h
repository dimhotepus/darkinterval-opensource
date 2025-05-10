//==================================================================================================
//
// Physically Based Rendering Header for brushes and models
//
//==================================================================================================

// Universal Constants
static const float PI = 3.141592;
static const float ONE_OVER_PI = 0.318309;
static const float EPSILON = 0.00001;

// Shlick's approximation of the Fresnel factor
float3 fresnelSchlick(float3 F0, float cosTheta)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 fresnelSchlickRoughness(float3 F0, float cosTheta, float roughness)
{
	return F0 + max(0.0, (1.0 - roughness) - F0) * pow(1.0 - cosTheta, 5.0);
}

// GGX/Towbridge-Reitz normal distribution function
// Uses Disney's reparametrization of alpha = roughness^2
float ndfGGX(float cosLh, float roughness)
{
    float alpha   = roughness * roughness;
    float alphaSq = alpha * alpha;

    float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
    return alphaSq / (PI * denom * denom);
}

/*
float3 diffuseOrenNayar(float3 DiffuseColor, float Roughness, float NoV, float NoL, float VoH)
{
	float a = Roughness * Roughness;
	float s = a;// / ( 1.29 + 0.5 * a );
	float s2 = s * s;
	float VoL = 2 * VoH * VoH - 1;		// double angle identity
	float Cosri = VoL - NoV * NoL;
	float C1 = 1 - 0.5 * s2 / (s2 + 0.33);
	float C2 = 0.45 * s2 / (s2 + 0.09) * Cosri * (Cosri >= 0 ? 1 / (max(NoL, NoV)) : 1);
	return DiffuseColor / PI * (C1 + C2) * (1 + Roughness * 0.5);
}
*/

float diffuseOrenNayar(float3 light, float3 view, float3 normal, float roughness )
{
	// Roughness, A and B
	float roughness2 = roughness * roughness;

	float2 oren_nayar_fraction = roughness2 / (roughness2 + float2(0.33, 0.09));
	float2 oren_nayar = float2(1, 0) + float2(-0.5, 0.45) * oren_nayar_fraction;

	// Theta and phi
	float2 cos_theta = saturate(float2(dot(normal, light), dot(normal, view)));
	float2 cos_theta2 = cos_theta * cos_theta;
	float sin_theta = sqrt((1 - cos_theta2.x) * (1 - cos_theta2.y));
	float3 light_plane = normalize(light - cos_theta.x * normal);
	float3 view_plane = normalize(view - cos_theta.y * normal);
	float cos_phi = saturate(dot(light_plane, view_plane));

	// Composition
	float diffuse_oren_nayar = cos_phi * sin_theta / max(cos_theta.x, cos_theta.y);
	float diffuse = cos_theta.x * (oren_nayar.x + oren_nayar.y * diffuse_oren_nayar);

	return diffuse;
}

// Single term for separable Schlick-GGX below
float gaSchlickG1(float cosTheta, float k)
{
    return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method
float gaSchlickGGX(float cosLi, float cosLo, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights
    return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}

// Monte Carlo integration, approximate analytic version based on Dimitar Lazarov's work
// https://www.unrealengine.com/en-US/blog/physically-based-shading-on-mobile
float3 EnvBRDFApprox(float3 SpecularColor, float Roughness, float NoV)
{
    const float4 c0 = { -1, -0.0275, -0.572, 0.022 };
    const float4 c1 = { 1, 0.0425, 1.04, -0.04 };
    float4 r = Roughness * c0 + c1;
    float a004 = min(r.x * r.x, exp2(-9.28 * NoV)) * r.x + r.y;
    float2 AB = float2(-1.04, 1.04) * a004 + r.zw;
    return SpecularColor * AB.x + AB.y;
}

// Compute the matrix used to transform tangent space normals to world space
// This expects DirectX normal maps in Mikk Tangent Space http://www.mikktspace.com
float3x3 compute_tangent_frame(float3 N, float3 P, float2 uv, out float3 T, out float3 B, out float sign_det)
{
    float3 dp1 = ddx(P);
    float3 dp2 = ddy(P);
    float2 duv1 = ddx(uv);
    float2 duv2 = ddy(uv);

    sign_det = dot(dp2, cross(N, dp1)) > 0.0 ? -1 : 1;

    float3x3 M = float3x3(dp1, dp2, cross(dp1, dp2));
    float2x3 inverseM = float2x3(cross(M[1], M[2]), cross(M[2], M[0]));
    T = normalize(mul(float2(duv1.x, duv2.x), inverseM));
    B = normalize(mul(float2(duv1.y, duv2.y), inverseM));
    return float3x3(T, B, N);
}

float GetAttenForLight(float4 lightAtten, int lightNum)
{
#if (NUM_LIGHTS > 1)
    if (lightNum == 1) return lightAtten.y;
#endif

#if (NUM_LIGHTS > 2)
    if (lightNum == 2) return lightAtten.z;
#endif

#if (NUM_LIGHTS > 3)
    if (lightNum == 3) return lightAtten.w;
#endif

    return lightAtten.x;
}

// Calculate direct light for one source
float3 calculateLight(float3 lightIn, float3 lightIntensity, float3 lightOut, float3 normal, float3 fresnelReflectance, float roughness, float metalness, float lightDirectionAngle, float3 albedo)
{
    // Lh
    float3 HalfAngle = normalize(lightIn + lightOut); 
    float cosLightIn = max(0.0, dot(normal, lightIn));
    float cosHalfAngle = max(0.0, dot(normal, HalfAngle));

    // F - Calculate Fresnel term for direct lighting
    //float F = diffuseOrenNayar(lightIn, lightOut, normal, roughness);
    float3 F = fresnelSchlick(fresnelReflectance, max(0.0, dot(HalfAngle, lightOut)));
    float3 F2 = fresnelSchlick(fresnelReflectance, max(0.0, dot(normal, lightOut)));
    float3 F3 = fresnelSchlick(fresnelReflectance, max(0.0, dot(normal, lightIn)));

    // D - Calculate normal distribution for specular BRDF
    float D = ndfGGX(cosHalfAngle, roughness);

    // Calculate geometric attenuation for specular BRDF
    float G = gaSchlickGGX(cosLightIn, lightDirectionAngle, roughness);

    // Cook-Torrance specular microfacet BRDF
	float3 specularBRDF = (F * D * G) / max(EPSILON, 4.0 * cosLightIn * lightDirectionAngle);

#if LIGHTMAPPED && !FLASHLIGHT

    // Ambient light from static lights is already precomputed in the lightmap. Don't add it again
    return specularBRDF * lightIntensity * cosLightIn;

#else

    // Diffuse scattering happens due to light being refracted multiple times by a dielectric medium
    // Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero
    // To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness
    float3 kd = lerp((float3(1, 1, 1) - F2 ) * (float3(1, 1, 1) - F3 ), float3(0, 0, 0), metalness);
 //   float3 diffuseBRDF = kd * albedo;
	float3 diffuseBRDF = diffuseOrenNayar(lightIn, lightOut,normal,roughness) * albedo;
    return (diffuseBRDF + specularBRDF) * lightIntensity * cosLightIn;

#endif // LIGHTMAPPED && !FLASHLIGHT
}

// Get diffuse ambient light
float3 ambientLookupLightmap(float3 normal, float3 textureNormal, float4 lightmapTexCoord1And2, float4 lightmapTexCoord3, sampler LightmapSampler, float4 g_DiffuseModulation)
{
#if (!LIGHTMAPPED_MODEL)
    float2 bumpCoord1;
    float2 bumpCoord2;
    float2 bumpCoord3;

    ComputeBumpedLightmapCoordinates(
            lightmapTexCoord1And2, lightmapTexCoord3.xy,
            bumpCoord1, bumpCoord2, bumpCoord3);

    float3 lightmapColor1 = LightMapSample(LightmapSampler, bumpCoord1);
    float3 lightmapColor2 = LightMapSample(LightmapSampler, bumpCoord2);
    float3 lightmapColor3 = LightMapSample(LightmapSampler, bumpCoord3);

    float3 dp;
    dp.x = saturate(dot(textureNormal, bumpBasis[0]));
    dp.y = saturate(dot(textureNormal, bumpBasis[1]));
    dp.z = saturate(dot(textureNormal, bumpBasis[2]));
    dp *= dp;

    float3 diffuseLighting =    dp.x * lightmapColor1 +
                                dp.y * lightmapColor2 +
                                dp.z * lightmapColor3;

    float sum = dot(dp, float3(1, 1, 1));
    diffuseLighting *= g_DiffuseModulation.xyz / sum;
#else
	float3 diffuseLighting = GammaToLinear(2.0f * tex2D(LightmapSampler, lightmapTexCoord1And2.xy).rgb);
#endif
    return diffuseLighting;
}

float3 ambientLookup(float3 normal, float3 ambientCube[6], float3 textureNormal, float4 lightmapTexCoord1And2, float4 lightmapTexCoord3, sampler LightmapSampler, float4 g_DiffuseModulation)
{
#if LIGHTMAPPED || LIGHTMAPPED_MODEL
    return ambientLookupLightmap(normal, textureNormal, lightmapTexCoord1And2, lightmapTexCoord3, LightmapSampler, g_DiffuseModulation);
#else
    return PixelShaderAmbientLight(normal, ambientCube);
#endif
}

// Create an ambient cube from the envmap
void setupEnvMapAmbientCube(out float3 EnvAmbientCube[6], sampler EnvmapSampler)
{
    float4 directionPosX = { 1, 0, 0, 12 }; float4 directionNegX = {-1, 0, 0, 12 };
    float4 directionPosY = { 0, 1, 0, 12 }; float4 directionNegY = { 0,-1, 0, 12 };
    float4 directionPosZ = { 0, 0, 1, 12 }; float4 directionNegZ = { 0, 0,-1, 12 };
    EnvAmbientCube[0] = ENV_MAP_SCALE * texCUBElod(EnvmapSampler, directionPosX).rgb;
    EnvAmbientCube[1] = ENV_MAP_SCALE * texCUBElod(EnvmapSampler, directionNegX).rgb;
    EnvAmbientCube[2] = ENV_MAP_SCALE * texCUBElod(EnvmapSampler, directionPosY).rgb;
    EnvAmbientCube[3] = ENV_MAP_SCALE * texCUBElod(EnvmapSampler, directionNegY).rgb;
    EnvAmbientCube[4] = ENV_MAP_SCALE * texCUBElod(EnvmapSampler, directionPosZ).rgb;
    EnvAmbientCube[5] = ENV_MAP_SCALE * texCUBElod(EnvmapSampler, directionNegZ).rgb;
}

float3 worldToRelative(float3 worldVector, float3 surfTangent, float3 surfBasis, float3 surfNormal)
{
   return float3(
       dot(worldVector, surfTangent),
       dot(worldVector, surfBasis),
       dot(worldVector, surfNormal)
   );
}

float3 rgb2hsv(float3 c)
{
    float4 P = (c.g < c.b) ? float4(c.bg, -1.0, 2.0 / 3.0) : float4(c.gb, 0.0, -1.0 / 3.0);
    float4 Q = (c.r < P.x) ? float4(P.xyw, c.r) : float4(c.r, P.yzx);
    float C = Q.x - min(Q.w, Q.y);
    return float3(abs(Q.z + (Q.w - Q.y) / (6.0 * C + EPSILON)), C / (Q.x + EPSILON), Q.x);
}

float3 hsv2rgb(float3 c)
{
    float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    float3 p = abs(frac(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * lerp(K.xxx, saturate(p - K.xxx), c.y);
} 

// Calculates UV offset for parallax bump mapping
float2 ParallaxOffset(float h, float t, float height, float3 viewDir)
{
	h = h * height - height / 2.0;
	float3 v = normalize(viewDir);
	v.z += t;
	return h * (v.xy / v.z);
}
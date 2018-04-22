
#define MaxLights 16




struct Light
{
    float3 Strength;
    float FalloffStart; // point/spot light only
    float3 Direction;   // directional/spot light only
    float FalloffEnd;   // point/spot light only
    float3 Position;    // point light only
    float SpotPower;    // spot light only
};

struct Material
{
    float4 DiffuseAlbedo;
    float3 FresnelR0;
    float Shininess;
};

//점광과 점적광에 적용하는 선형 감쇠 계수를 계산.
float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
    // 선형 감쇠
    return saturate((falloffEnd-d) / (falloffEnd - falloffStart));
}

// 슐릭 근사는 프레넬 반사의 근삿값을 제공
// R0 = ( (n-1)/(n+1) )^2, n은 굴절 지수임.
float3 SchlickFresnel(float3 R0, float3 normal, float3 lightVec)
{
    float cosIncidentAngle = saturate(dot(normal, lightVec));

    float f0 = 1.0f - cosIncidentAngle;
    float3 reflectPercent = R0 + (1.0f - R0)*(f0*f0*f0*f0*f0);

    return reflectPercent;
}

float3 BlinnPhong(float3 lightStrength, float3 lightVec, float3 normal, float3 toEye, Material mat)
{
	// 거칠기에서 얻은 광택도로 m을 유도
    const float m = mat.Shininess * 256.0f;
    float3 halfVec = normalize(toEye + lightVec);

    float roughnessFactor = (m + 8.0f)*pow(max(dot(halfVec, normal), 0.0f), m) / 8.0f;
    float3 fresnelFactor = SchlickFresnel(mat.FresnelR0, halfVec, lightVec);

    float3 specAlbedo = fresnelFactor*roughnessFactor;

    // 반사율을 1미만으로 낮춘다 (반영 반사율 공식이 [0,1] 바깥의 값을 낼 수도 있지만)
	// 1이상이면 아주 밝은 하이라이트가 나올 수 있다
    specAlbedo = specAlbedo / (specAlbedo + 1.0f);

    return (mat.DiffuseAlbedo.rgb + specAlbedo) * lightStrength;
}

//---------------------------------------------------------------------------------------
// 지향광의 구현
//---------------------------------------------------------------------------------------
float3 ComputeDirectionalLight(Light L, Material mat, float3 normal, float3 toEye , int toonLevel)
{
    // 빛 벡터는 광선들이 나아가는 반대방향을 가르킴
    float3 lightVec = -L.Direction;

    // 람베르트 코사인 법칙에 따라 빛의 세기를 줄인다(이상한그래프)
    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStrength = L.Strength * ndotl;
	lightStrength = ceil(lightStrength * toonLevel) / (float)toonLevel;

    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

//---------------------------------------------------------------------------------------
// 점광 구현부
//---------------------------------------------------------------------------------------
float3 ComputePointLight(Light L, Material mat, float3 pos, float3 normal, float3 toEye)
{
    // 표면에서 광원으로의 벡터
    float3 lightVec = L.Position - pos;

    // 광원과 표면 사이의 거리
    float d = length(lightVec);

    // 범위 판정
    if(d > L.FalloffEnd)
        return 0.0f;

    // 빛 벡터를 정규화함
    lightVec /= d;

    // 람베르트 코사인 법칙에 따라 빛의 세기를 줄임
    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStrength = L.Strength * ndotl;

    // 거리에 따른 감쇠
    float att = CalcAttenuation(d, L.FalloffStart, L.FalloffEnd);
    lightStrength *= att;

    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

//---------------------------------------------------------------------------------------
// 점적광 구현부
//---------------------------------------------------------------------------------------
float3 ComputeSpotLight(Light L, Material mat, float3 pos, float3 normal, float3 toEye)
{
    // 표면에서 광원으로의 벡터
    float3 lightVec = L.Position - pos;

    // 광원과 표면 사이의 거리
    float d = length(lightVec);

    // 범위 판정
    if(d > L.FalloffEnd)
        return 0.0f;

    // 빛 벡터를 정규화한다
    lightVec /= d;

    // 람베르트 코사인 법칙에 따라 빛의 세기를 줄인다
    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStrength = L.Strength * ndotl;

    // 거리에 따라 빛을 감쇠
    float att = CalcAttenuation(d, L.FalloffStart, L.FalloffEnd);
    lightStrength *= att;

    // 점적광 계수로 비례한다
    float spotFactor = pow(max(dot(-lightVec, L.Direction), 0.0f), L.SpotPower);
    lightStrength *= spotFactor;

    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

float4 ComputeLighting(Light gLights[MaxLights], Material mat,
                       float3 pos, float3 normal, float3 toEye,
                       float3 shadowFactor, int toonLevel)
{
    float3 result = 0.0f;

    int i = 0;

#if (NUM_DIR_LIGHTS > 0)
    for(i = 0; i < NUM_DIR_LIGHTS; ++i)
    {
        result += shadowFactor[i] * ComputeDirectionalLight(gLights[i], mat, normal, toEye, toonLevel);
    }
#endif

#if (NUM_POINT_LIGHTS > 0)
    for(i = NUM_DIR_LIGHTS; i < NUM_DIR_LIGHTS+NUM_POINT_LIGHTS; ++i)
    {
        result += ComputePointLight(gLights[i], mat, pos, normal, toEye);
    }
#endif

#if (NUM_SPOT_LIGHTS > 0)
    for(i = NUM_DIR_LIGHTS + NUM_POINT_LIGHTS; i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS + NUM_SPOT_LIGHTS; ++i)
    {
        result += ComputeSpotLight(gLights[i], mat, pos, normal, toEye);
    }
#endif 

    return float4(result, 0.0f);
}



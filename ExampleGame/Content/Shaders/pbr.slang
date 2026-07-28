import global;
import lighting;
import graphics;

[shader("vertex")]
VSOutput main(VSInput input)
{
    VSOutput output;

    output.location = TransformVertex(input);
    output.fragLocation = mul(transform.value, input.location).xyz;
    
    output.normal = input.normal.xyz;
    output.tangent = input.tangent;
    
    output.uv0 = input.uv0;
    output.color = input.color;

    return output;
}

[shader("fragment")]
float4 main(VSOutput input) : SV_Target
{
    float4 baseColorSample = TrySampleMap(material.baseColorIndex, input.uv0, material.color);
    float4 baseColor = baseColorSample * input.color;

    float4 defaultOrm = float4(material.ao, material.roughness, material.metallic, 1.0);
    float4 orm = TrySampleMap(material.ormIndex, input.uv0, defaultOrm);

    float4 emissiveSample = TrySampleMap(material.emissiveIndex, input.uv0, material.emissiveTint);
    float3 emissive = emissiveSample.rgb;

    float ao = orm.r;
    float roughness = orm.g;
    float metallic = orm.b;

    float4 normalMapSample = TrySampleMap(material.normalIndex, input.uv0, float4(input.normal, 0.0));
    float3 N = CalculateNormal(input, normalMapSample);

    // === LIGHTING SETUP ===
    // Calculate view direction (fragment to camera)
    float3 V = normalize(globals.cameraLocation - input.location.xyz);
    
    // Calculate reflection vector for environment mapping
    float3 R = reflect(-V, N);

    // === PBR MATERIAL SETUP ===
    // Calculate F0 (reflectance at normal incidence)
    // Non-metals: low reflectance (~0.04), Metals: colored reflectance from base color
    float3 F0 = float3(0.04, 0.04, 0.04);  // Dielectric default
    F0 = lerp(F0, baseColor.rgb, metallic); // Lerp to metallic behavior

    // Initialize outgoing radiance accumulator
    float3 Lo = float3(0.0, 0.0, 0.0);

    // === DIRECT LIGHTING LOOP ===
    // Calculate contribution from each light source
    for(int i = 0; i < MAX_LIGHT_COUNT; ++i)
    {
       LightUniform light = lights[i];

       if(light.enabled == 0)
       {
           continue;
       }

       float3 lightColor = light.color.rgb * light.intensity;

       // Calculate light direction and attenuation
       float3 L = LightDirection(i, input.fragLocation);      // Light direction

       float distance = length(L);   // Distance for falloff
       float attenuation = 1.0 / (distance * distance);     // Inverse square falloff
       float3 radiance = lightColor * attenuation;           // Attenuated light color

       // Calculate half vector (between view and light directions)
       float3 H = normalize(V + L);

       // === BRDF EVALUATION ===
       // Calculate all necessary dot products for BRDF terms
       float NdotL = max(dot(N, L), 0.0);  // Lambertian falloff
       float NdotV = max(dot(N, V), 0.0);  // View angle
       float NdotH = max(dot(N, H), 0.0);  // Half vector for specular
       float HdotV = max(dot(H, V), 0.0);  // For Fresnel calculation

       // Evaluate Cook-Torrance BRDF components
       float D = DistributionGGX(NdotH, roughness);    // Normal distribution
       float G = GeometrySmith(NdotV, NdotL, roughness); // Geometry function
       float3 F = FresnelSchlick(HdotV, F0);           // Fresnel reflectance

       // Calculate specular BRDF
       float3 numerator = D * G * F;
       float denominator = 4.0 * NdotV * NdotL + 0.0001; // Prevent division by zero
       float3 specular = numerator / denominator;

       // === ENERGY CONSERVATION ===
       // Fresnel term represents specular reflection ratio
       float3 kS = F;                          // Specular contribution
       float3 kD = float3(1.0, 1.0, 1.0) - kS; // Diffuse contribution (energy conservation)
       kD *= 1.0 - metallic;                   // Metals have no diffuse reflection

       // === RADIANCE ACCUMULATION ===
       // Combine diffuse (Lambertian) and specular (Cook-Torrance) terms
       // Multiply by incident radiance and cosine foreshortening
       Lo += (kD * baseColor.rgb / PI + specular) * radiance * NdotL;
    }
    
    // === AMBIENT AND EMISSIVE ===
    // Add simple ambient lighting (should be replaced with IBL in production)
    float3 ambient = (sceneLighting.ambientColor * sceneLighting.ambientStrength).rgb * baseColor.rgb * ao;

    // Combine all lighting contributions
    float3 color = ambient + Lo + emissive;

    // === HDR TONE MAPPING AND GAMMA CORRECTION ===
    // Apply Reinhard tone mapping to compress HDR values to [0,1] range
    color = color / (color + float3(1.0, 1.0, 1.0));

    // Apply gamma correction for sRGB display (inverse gamma)
    color = pow(color, float3(1.0 / globals.gamma, 1.0 / globals.gamma, 1.0 / globals.gamma));

    // Output final color with original alpha
    return float4(color, baseColor.a);
}
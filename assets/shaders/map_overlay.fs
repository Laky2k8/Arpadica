#version 330

in vec2 fragTexCoord;
in vec3 fragNormal;
in vec3 fragPosition;

out vec4 finalColor;

uniform sampler2D texture0;       // base diffuse (color map)
uniform sampler2D politicalMap;   // overlay from MapEngine
uniform float overlayMix;         // 0..1
uniform vec4 worldMinMax;         // (minX, maxX, minZ, maxZ)

// Lighting uniforms
uniform vec3 lightDir;            // world-space direction TO light (normalized)
uniform vec3 lightColor;          // e.g., (1,1,1)
uniform float ambient;            // e.g., 0.25

void main()
{
    vec4 base = texture(texture0, fragTexCoord);

    float u = (fragPosition.x - worldMinMax.x) / (worldMinMax.y - worldMinMax.x);
    float v = (fragPosition.z - worldMinMax.z) / (worldMinMax.w - worldMinMax.z);
    vec2 overlayUV = vec2(u, 1.0 - v); // if upside-down, change to vec2(u, v)

    vec4 pol = texture(politicalMap, overlayUV);

    // Alpha-driven blend so transparent overlay leaves base intact
    float a = pol.a * overlayMix;
    vec3 albedo = mix(base.rgb, pol.rgb, a);

    // Simple Lambert lighting
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(lightDir);
	
	float diff = max(dot(N, L), 0.0);
	vec3 lighting = ambient + diff * lightColor;
	lighting = max(lighting, vec3(ambient)); // clamp floor
	finalColor = vec4(albedo * lighting, 1.0);
}
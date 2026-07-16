float rand3dTo1d(float3 value) {
    return frac(sin(dot(value, float3(12.9898, 78.233, 37.719))) * 43758.5453123);
}

float3 rand3dTo3d(float3 value) {
    return float3(
        rand3dTo1d(value),
        rand3dTo1d(value + float3(17.23, 41.37, 11.19)),
        rand3dTo1d(value + float3(73.91, 29.57, 53.83))
    );
}

class RandomGenerator {
    float32_t3 seed;
    
    float32_t3 Generate3d() {
        seed = rand3dTo3d(seed);
        return seed;
    }
    
    float32_t Generate1d() {
        float32_t result = rand3dTo1d(seed);
        seed.x = result;
        return result;
    }
};
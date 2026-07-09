struct Well {
    float32_t4x4 skeletonSpaceMatrix;
    float32_t4x4 skeletonSpaceInverseTransposeMatrix;
};
struct Vertex {
    float32_t4 position;
    float32_t2 texcoord;
    float32_t3 normal;
    float32_t4 color;
};
struct VertexInfluence {
    float32_t4 weight;
    int32_t4 index;
};
struct SkinningInformation {
    uint32_t numVertices;
};

StructuredBuffer<Well> gMatrixPalette : register(t0);
StructuredBuffer<Vertex> gInputVertices : register(t1);
StructuredBuffer<VertexInfluence> gInfluence : register(t2);
ConstantBuffer<SkinningInformation> gSkinningInformation : register(b0);
// 計算結果の頂点データ
RWStructuredBuffer<Vertex> gOutputVertices : register(u0);

[numthreads(1024, 1, 1)]
void main(uint32_t3 DTid : SV_DispatchThreadID) {
    uint32_t vertexIndex = DTid.x;
    // 頂点数の分だけ処理
    if (vertexIndex < gSkinningInformation.numVertices) {
        Vertex input = gInputVertices[vertexIndex];
        VertexInfluence influence = gInfluence[vertexIndex];
        
        Vertex skinned;
        skinned.texcoord = input.texcoord;
        skinned.color = input.color;
        
        // position
        skinned.position = mul(input.position, gMatrixPalette[influence.index.x].skeletonSpaceMatrix) * influence.weight.x;
        skinned.position += mul(input.position, gMatrixPalette[influence.index.y].skeletonSpaceMatrix) * influence.weight.y;
        skinned.position += mul(input.position, gMatrixPalette[influence.index.z].skeletonSpaceMatrix) * influence.weight.z;
        skinned.position += mul(input.position, gMatrixPalette[influence.index.w].skeletonSpaceMatrix) * influence.weight.w;
        skinned.position.w = 1.0f;

        // normal
        skinned.normal = mul(input.normal, (float32_t3x3) gMatrixPalette[influence.index.x].skeletonSpaceInverseTransposeMatrix) * influence.weight.x;
        skinned.normal += mul(input.normal, (float32_t3x3) gMatrixPalette[influence.index.y].skeletonSpaceInverseTransposeMatrix) * influence.weight.y;
        skinned.normal += mul(input.normal, (float32_t3x3) gMatrixPalette[influence.index.z].skeletonSpaceInverseTransposeMatrix) * influence.weight.z;
        skinned.normal += mul(input.normal, (float32_t3x3) gMatrixPalette[influence.index.w].skeletonSpaceInverseTransposeMatrix) * influence.weight.w;
        skinned.normal = normalize(skinned.normal);

        
        Vertex debug;

        debug.position = float32_t4(1.0f, 0.0f, 0.0f, 1.0f);
        debug.texcoord = float32_t2(0.0f, 0.0f);
        debug.normal = float32_t3(0.0f, 1.0f, 0.0f);
        debug.color = float32_t4(1, 1, 1, 1);

        gOutputVertices[vertexIndex] = debug;
        
        // 結果
        gOutputVertices[vertexIndex] = skinned;
    }
}
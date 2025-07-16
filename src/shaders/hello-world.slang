// hello-world.slang
StructuredBuffer<float2> buffer0;
RWStructuredBuffer<float2> result;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    uint index = threadId.x;
    result[index] = buffer0[index] + result[index];
}

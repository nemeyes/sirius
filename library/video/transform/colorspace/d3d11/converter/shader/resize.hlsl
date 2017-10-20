cbuffer resize_info
{
	float wratio;
	float hratio;
};
Texture2D input : register(t0);
RWTexture2D<uint4> output : register(u0);
//rgb resize
[numthreads(32, 16, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	//output[int2(0, 0)] = unorm
	int ix = dispatchThreadID.x;
	int iy = dispatchThreadID.y;
	
	int ox = (int)(wratio * ix);
	int oy = (int)(hratio * iy);

	output[int2(ox, oy)] = uint4(input[int2(ix, iy)].r * 255, 
								 input[int2(ix, iy)].g * 255, 
								 input[int2(ix, iy)].b * 255, 
								 input[int2(ix, iy)].a * 255);// (unorm float4)(255, 2, 3, 0);
	return;
}
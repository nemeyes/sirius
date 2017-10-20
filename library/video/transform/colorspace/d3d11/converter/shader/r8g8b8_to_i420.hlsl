struct Pixel
{
	int color;
};
Texture2D<unsigned int4> input : register(t0);
//RWTexture2D<int> output;
RWStructuredBuffer<Pixel> output : register(u0);

int rgb2y(int r, int g, int b)
{
	//return ((4211 * r + 8258 * g + 1606 * b + 8192) >> 14) + 16;
	return 0.299 * r + 0.587 * g + 0.114 * b;
}

int rgb2v(int r, int g, int b)
{
	return 0.500 * r - 0.418688 * g - 0.081312 * b + 128;
	//return -0.147 * r - 0.259 * g + 0.436 * b;
	//return ((7193 * r - 6029 * g - 1163 * b + 8192) >>14 ) + 128;
}

int rgb2u(int r, int g, int b)
{
	return -0.168736 * r - 0.331264 * g + 0.500 * b + 128;
	//return 0.615 * r - 0.515 * g - 0.1 * b;
	//return ((-2425 * r - 4768 * g + 7193 * b + 8192) >> 14) + 128;
}

//b8g8r8a8 to i420
[numthreads(32, 16, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint x = dispatchThreadID.x;
	uint y = dispatchThreadID.y;
	uint red, green, blue;
	if (x % 4 == 0)
	{
		int value = 0;
		int index = (x + y * input.Length.x) / 4;
		for (int i = 0; i < 4; i++)
		{
			//Input is B8G8R8A8 Type
			red		= (int)(input[int2(x + i, y)].r);
			green	= (int)(input[int2(x + i, y)].g);
			blue	= (int)(input[int2(x + i, y)].b);
			value |= rgb2y(red, green, blue) << (8 * i);
		}
		output[index].color = value;
	}
	
	if ((x % 8 == 0) && (y % 2 == 0))
	{
		int y_size = input.Length.x * input.Length.y / 4;
		int uv_size = input.Length.x * input.Length.y / 4 / 4;
		int uv_offset = (x + y / 2 * input.Length.x) / 8;
		int uindex = y_size + uv_offset;
		int vindex = uindex + uv_size;
		output[vindex].color = 0;
		output[uindex].color = 0;
		for (uint i = 0; i < 4; i++)
		{
			uint vvalue = 0;
			uint uvalue = 0;
			for (uint x_off = 0; x_off < 2; x_off++)
			{
				for (uint y_off = 0; y_off < 2; y_off++)
				{
					red	= (int)(input[int2(x + 2 * i + x_off, y + y_off )].r);
					green	= (int)(input[int2(x + 2 * i + x_off, y + y_off )].g);
					blue = (int)(input[int2(x + 2 * i + x_off, y + y_off )].b);
					vvalue += rgb2v(red, green, blue);
					uvalue += rgb2u(red, green, blue);
				}
			}
			output[vindex].color |= (vvalue / 4) << (8 * i);	// |= (vvalue / 4) << (8 * i);
			output[uindex].color |= (uvalue / 4) << (8 * i);
		}
	}
}



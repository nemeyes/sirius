#include <sirius_video_processor.h>
#include <sirius_timestamp_generator.h>

sirius::library::video::transform::codec::processor::processor(void)
	: _frame_count(0)
{

}

sirius::library::video::transform::codec::processor::~processor(void)
{}

void sirius::library::video::transform::codec::processor::begin_elapsed_time(void)
{
	sirius::library::misc::timestamp::generator::instance().begin_elapsed_time();
}

long long sirius::library::video::transform::codec::processor::elapsed_microseconds(void)
{
	return sirius::library::misc::timestamp::generator::instance().elapsed_microseconds();
}

long long sirius::library::video::transform::codec::processor::elapsed_milliseconds(void)
{
	return sirius::library::misc::timestamp::generator::instance().elapsed_milliseconds();
}

int32_t sirius::library::video::transform::codec::processor::initialize_d3d11(ID3D11Device * d3d11_device, int32_t iwidth, int32_t iheight, int32_t ifps, int32_t owidth, int32_t oheight, int32_t ofps, DXGI_FORMAT oformat)
{
	HRESULT hr = S_OK;
	int32_t status = sirius::library::video::transform::codec::processor::err_code_t::fail;

	ATL::CComPtr<ID3D11VideoContext> d3d11_video_context = NULL;

	do
	{
		hr = d3d11_device->QueryInterface(__uuidof(ID3D11VideoDevice), (void**)&_d3d11_video_device);
		if (FAILED(hr))
			break;

		d3d11_device->GetImmediateContext(&_d3d11_device_context);
		hr = _d3d11_device_context->QueryInterface(__uuidof(ID3D11VideoContext), (void**)&d3d11_video_context);
		if (FAILED(hr))
			break;

		D3D11_VIDEO_PROCESSOR_CONTENT_DESC content_desc;
		content_desc.InputFrameFormat = D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE;
		content_desc.InputWidth = (DWORD)iwidth;
		content_desc.InputHeight = (DWORD)iheight;
		content_desc.OutputWidth = (DWORD)owidth;
		content_desc.OutputHeight = (DWORD)oheight;
		content_desc.InputFrameRate.Numerator = ifps;
		content_desc.InputFrameRate.Denominator = 1;
		content_desc.OutputFrameRate.Numerator = ofps;
		content_desc.OutputFrameRate.Denominator = 1;
		content_desc.Usage = D3D11_VIDEO_USAGE_PLAYBACK_NORMAL;

		hr = _d3d11_video_device->CreateVideoProcessorEnumerator(&content_desc, &_d3d11_video_processor_enum);
		if (FAILED(hr))
			break;

		UINT flags;
		DXGI_FORMAT output_format = oformat;
		hr = _d3d11_video_processor_enum->CheckVideoProcessorFormat(output_format, &flags);
		if (FAILED(hr) || (flags & D3D11_VIDEO_PROCESSOR_FORMAT_SUPPORT_OUTPUT) == 0)
			break;

		DWORD index = 0;
		D3D11_VIDEO_PROCESSOR_CAPS caps = {};
		D3D11_VIDEO_PROCESSOR_RATE_CONVERSION_CAPS conv_caps = {};

		hr = _d3d11_video_processor_enum->GetVideoProcessorCaps(&caps);
		if (FAILED(hr))
			break;

		for (DWORD i = 0; i < caps.RateConversionCapsCount; i++)
		{
			hr = _d3d11_video_processor_enum->GetVideoProcessorRateConversionCaps(i, &conv_caps);
			if (FAILED(hr))
				break;

			if ((conv_caps.ProcessorCaps & D3D11_VIDEO_PROCESSOR_PROCESSOR_CAPS_DEINTERLACE_BOB) != 0)
			{
				index = i;
				break;
			}
		}

		if (FAILED(hr))
			break;

		hr = _d3d11_video_device->CreateVideoProcessor(_d3d11_video_processor_enum, index, &_d3d11_video_processor);
		if (FAILED(hr))
			break;

		status = sirius::library::video::transform::codec::processor::err_code_t::success;
	} while (0);

	return status;
}

int32_t sirius::library::video::transform::codec::processor::release_d3d11(void)
{
	_d3d11_video_processor = NULL;
	_d3d11_video_processor_enum = NULL;
	_d3d11_device_context = NULL;
	_d3d11_video_device = NULL;

	return sirius::library::video::transform::codec::processor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::processor::convert_d3d11_texture2d_format(ID3D11Texture2D * input, ID3D11Resource * output, int32_t iwidth, int32_t iheight, int32_t owidth, int32_t oheight)
{
	int32_t status = sirius::library::video::transform::codec::processor::err_code_t::fail;
	HRESULT hr = E_FAIL;

	ATL::CComPtr<ID3D11VideoProcessorInputView> input_view = NULL;
	ATL::CComPtr<ID3D11VideoProcessorOutputView> output_view = NULL;
	ATL::CComPtr<ID3D11VideoContext> video_context = NULL;
	do
	{
		hr = _d3d11_device_context->QueryInterface(__uuidof(ID3D11VideoContext), (void**)&video_context);
		if (FAILED(hr))
			break;

		D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC output_view_desc;
		memset(&output_view_desc, 0x00, sizeof(D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC));
		output_view_desc.ViewDimension = D3D11_VPOV_DIMENSION_TEXTURE2D;
		output_view_desc.Texture2D.MipSlice = 0;
		output_view_desc.Texture2DArray.MipSlice = 0;
		output_view_desc.Texture2DArray.FirstArraySlice = 0;
		hr = _d3d11_video_device->CreateVideoProcessorOutputView(output, _d3d11_video_processor_enum, &output_view_desc, &output_view);
		if (FAILED(hr))
			break;

		D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC input_view_desc;
		memset(&input_view_desc, 0x00, sizeof(D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC));
		input_view_desc.FourCC = 0;
		input_view_desc.ViewDimension = D3D11_VPIV_DIMENSION_TEXTURE2D;
		input_view_desc.Texture2D.MipSlice = 0;
		input_view_desc.Texture2D.ArraySlice = 0;
		hr = _d3d11_video_device->CreateVideoProcessorInputView(input, _d3d11_video_processor_enum, &input_view_desc, &input_view);
		if (FAILED(hr))
			break;

		video_context->VideoProcessorSetStreamFrameFormat(_d3d11_video_processor, 0, D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE);
		video_context->VideoProcessorSetStreamOutputRate(_d3d11_video_processor, 0, D3D11_VIDEO_PROCESSOR_OUTPUT_RATE_NORMAL, TRUE, NULL); // Output rate (repeat frames)

		RECT SRect = { 0, 0, iwidth, iheight };
		RECT DRect = { 0, 0, owidth, oheight };
		video_context->VideoProcessorSetStreamSourceRect(_d3d11_video_processor, 0, TRUE, &SRect); // Source rect
		video_context->VideoProcessorSetStreamDestRect(_d3d11_video_processor, 0, TRUE, &DRect); // Stream dest rect
		video_context->VideoProcessorSetOutputTargetRect(_d3d11_video_processor, TRUE, &DRect);

		D3D11_VIDEO_PROCESSOR_COLOR_SPACE cs = {};
		cs.YCbCr_xvYCC = 1;
		video_context->VideoProcessorSetStreamColorSpace(_d3d11_video_processor, 0, &cs);
		video_context->VideoProcessorSetOutputColorSpace(_d3d11_video_processor, &cs); // Output color space

		D3D11_VIDEO_COLOR bgcolor = {};
		bgcolor.RGBA.A = 1.0F;
		bgcolor.RGBA.R = 1.0F * static_cast<float>(GetRValue(0)) / 255.0F;
		bgcolor.RGBA.G = 1.0F * static_cast<float>(GetGValue(0)) / 255.0F;
		bgcolor.RGBA.B = 1.0F * static_cast<float>(GetBValue(0)) / 255.0F;
		video_context->VideoProcessorSetOutputBackgroundColor(_d3d11_video_processor, TRUE, &bgcolor);

		D3D11_VIDEO_PROCESSOR_STREAM d3d11_stream_data;
		ZeroMemory(&d3d11_stream_data, sizeof(D3D11_VIDEO_PROCESSOR_STREAM));
		d3d11_stream_data.Enable = TRUE;
		d3d11_stream_data.OutputIndex = 0;
		d3d11_stream_data.InputFrameOrField = 0;
		d3d11_stream_data.PastFrames = 0;
		d3d11_stream_data.FutureFrames = 0;
		d3d11_stream_data.ppPastSurfaces = NULL;
		d3d11_stream_data.ppFutureSurfaces = NULL;
		d3d11_stream_data.pInputSurface = input_view;
		d3d11_stream_data.ppPastSurfacesRight = NULL;
		d3d11_stream_data.ppFutureSurfacesRight = NULL;

		hr = video_context->VideoProcessorBlt(_d3d11_video_processor, output_view, 0, 1, &d3d11_stream_data);
		if (FAILED(hr))
		{
			DWORD err = ::GetLastError();
			break;
		}

		status = sirius::library::video::transform::codec::processor::err_code_t::success;

	} while (0);

	return status;
}

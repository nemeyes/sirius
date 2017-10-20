#include <sirius_video_colorspace_converter.h>

/*
sirius::library::video::transform::colorspace::converter::_context_t::_context_t(void)
	: iformat(sirius::library::video::transform::colorspace::converter::video_submedia_type_t::rgb32)
	, oformat(sirius::library::video::transform::colorspace::converter::video_submedia_type_t::nv12)
{

}

sirius::library::video::transform::colorspace::converter::_context_t::_context_t(const sirius::library::video::transform::colorspace::converter::_context_t & clone)
{
	iformat = clone.iformat;
	oformat = clone.oformat;
}

sirius::library::video::transform::colorspace::converter::_context_t & sirius::library::video::transform::colorspace::converter::_context_t::operator=(const sirius::library::video::transform::colorspace::converter::_context_t & clone)
{
	iformat = clone.iformat;
	oformat = clone.oformat;
	return (*this);
}
*/

sirius::library::video::transform::colorspace::converter::converter(void)
{

}

sirius::library::video::transform::colorspace::converter::~converter(void)
{

}

int32_t sirius::library::video::transform::colorspace::converter::initialize(void * context)
{
	return sirius::library::video::transform::colorspace::converter::err_code_t::not_implemented;
}

int32_t sirius::library::video::transform::colorspace::converter::release(void)
{
	return sirius::library::video::transform::colorspace::converter::err_code_t::not_implemented;
}

int32_t sirius::library::video::transform::colorspace::converter::convert(sirius::library::video::transform::colorspace::converter::entity_t * input, sirius::library::video::transform::colorspace::converter::entity_t * output)
{
	return sirius::library::video::transform::colorspace::converter::err_code_t::not_implemented;
}
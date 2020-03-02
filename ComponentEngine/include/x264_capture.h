/*********************************************************
	x264_capture.h
	Movie capture using the x264 encoding library
**********************************************************/

#ifndef GAL_X264_H_INCLUDED
#define GAL_X264_H_INCLUDED

#ifdef _MSC_VER
//#define X264_API_IMPORTS 0
#include "stdint.h"
#endif
extern "C" {
#include "x264.h"
}
#include <string>

namespace gal { namespace system {

class CX264
{
public:
	// Exceptions
	class XCtorFailure{};

public:
	CX264(std::string filename, int width, int height, int fps_num, int fps_den);
	CX264::~CX264();

	void CopyFrame( uint8_t* data, unsigned int pitch );
	bool EncodeAndWriteFrame( bool delayed = false );
	bool CompleteDelayedFrames();

private:
	void CopyFrameSMPTE170M( uint8_t* data, unsigned int pitch );
	void CopyFrameBT709( uint8_t* data, unsigned int pitch );

	FILE*          m_x264OutputHandle;
	x264_param_t   m_x264Param;
	x264_t*        m_x264Encoder;
	x264_picture_t m_x264PicIn;
};


} } // namespaces

#endif // GAL_DX11_RENDER_DEVICE_H_INCLUDED

/*********************************************************
	x264_capture.cpp
	Movie capture using the x264 encoding library
**********************************************************/

#include "x264_capture.h"

namespace gal { namespace system {

const char* X264Preset  = "slow";
const char* X264Tune    = 0;
const char* X264Profile = "high";
const int   X264Level   = 41; //4.1

const int   MOVIE_BITRATE         = 35000;
const int   MOVIE_MAX_BITRATE     = 35000;
const int   MOVIE_DEBLOCK         = -1;
const float MOVIE_PSY_RD          = 1.3f;
const float MOVIE_PSY_TRELLIS     = 0.3;


CX264::CX264( std::string filename, int width, int height, int fps_num, int fps_den )
{
	// Default settings for a given preset
	x264_param_default_preset( &m_x264Param, X264Preset, X264Tune );

	// Movie settings
//	m_x264Param.i_level_idc = X264Level;
	m_x264Param.i_width  = width;
	m_x264Param.i_height = height;
	m_x264Param.i_csp = X264_CSP_I420;
	m_x264Param.i_fps_num = fps_num;
	m_x264Param.i_fps_den = fps_den;
	m_x264Param.vui.b_fullrange = 0;                       // TV range output
	m_x264Param.vui.i_colorprim = (height <= 576) ? 6 : 1; // SMTPE-170M (for SD) or BT709 (for HD)
	m_x264Param.vui.i_transfer  = (height <= 576) ? 6 : 1; // -"-
	m_x264Param.vui.i_colmatrix = (height <= 576) ? 6 : 1; // -"-

	// Encoding settings
	m_x264Param.i_threads = 0;
	m_x264Param.b_vfr_input = 0;
	//--pass 2 --slow-firstpass --stats ".stats"
	m_x264Param.rc.i_rc_method = X264_RC_CQP; // Constant quantizer mode...
	m_x264Param.rc.i_qp_constant = 0;         // ...used for lossless output
//	m_x264Param.rc.i_rc_method   = X264_RC_CRF;
//	m_x264Param.rc.f_rf_constant = 20.0;
//	m_x264Param.rc.i_rc_method       = X264_RC_ABR;          // ABR mode, two pass would be better. Can also use X264_RC_CRF (set f_rf_constant, not bitrate, can use vbv though)
//	m_x264Param.rc.i_bitrate         = MOVIE_BITRATE;        // Bitrate settings
//	m_x264Param.rc.i_vbv_max_bitrate = MOVIE_MAX_BITRATE;    // -"-
//	m_x264Param.rc.i_vbv_buffer_size = MOVIE_MAX_BITRATE;    // -"-
	m_x264Param.i_deblocking_filter_alphac0 = MOVIE_DEBLOCK; // Deblocking
	m_x264Param.i_deblocking_filter_beta    = MOVIE_DEBLOCK; // -"-
	m_x264Param.analyse.f_psy_rd      = MOVIE_PSY_RD;        // Psy-RDO
	m_x264Param.analyse.f_psy_trellis = MOVIE_PSY_TRELLIS;   // -"-

	// Compatibility with some decoders
	m_x264Param.rc.i_qp_min = 4;                  
	if (m_x264Param.i_frame_reference > 4) m_x264Param.i_frame_reference = 4;

	// Profile
	x264_param_apply_profile( &m_x264Param, X264Profile );
		
	m_x264Encoder = x264_encoder_open(&m_x264Param);
	x264_encoder_parameters( m_x264Encoder, &m_x264Param );

	m_x264OutputHandle = fopen( filename.c_str(), "wb" );

	int numNals;
	x264_nal_t* nals;
	x264_encoder_headers( m_x264Encoder, &nals, &numNals );

	unsigned int size = 0;
	for (int i = 0; i < numNals; i++) size += nals[i].i_payload;
	if(fwrite( nals[0].p_payload, size, 1, m_x264OutputHandle ) != 1) throw XCtorFailure();

	x264_picture_alloc( &m_x264PicIn, X264_CSP_I420, m_x264Param.i_width, m_x264Param.i_height );
}

CX264::~CX264()
{
	CompleteDelayedFrames(); 	// Complete any current output
	x264_picture_clean(&m_x264PicIn);
	fclose(m_x264OutputHandle);
	x264_encoder_close(m_x264Encoder); 
}


void CX264::CopyFrame( uint8_t* data, unsigned int pitch )
{
	// HD or SD standards use different conversions from RGB (rendered colourspace) to YUV (movie colourspace)
	m_x264Param.i_height <= 576 ? CopyFrameSMPTE170M( data, pitch ) : CopyFrameBT709( data, pitch );
}

bool CX264::EncodeAndWriteFrame( bool delayed /*= false*/ )
{
	int numNals;
	x264_nal_t* nals;
	x264_picture_t x264PicOut;
    int frame_size = x264_encoder_encode( m_x264Encoder, &nals, &numNals, delayed ? NULL : &m_x264PicIn, &x264PicOut );
	if (frame_size > 0 && fwrite( nals[0].p_payload, frame_size, 1, m_x264OutputHandle ) != 1) return false;

	return true;
}

bool CX264::CompleteDelayedFrames()
{
    while (x264_encoder_delayed_frames(m_x264Encoder))
	{
		if (!EncodeAndWriteFrame(true)) return false;
    }
	return true;
}


void CX264::CopyFrameSMPTE170M( uint8_t* data, unsigned int pitch )
{
	uint8_t* yp1 = m_x264PicIn.img.plane[0];
	uint8_t* yp2 = yp1 + m_x264Param.i_width;
	uint8_t* up = m_x264PicIn.img.plane[1];
	uint8_t* vp = m_x264PicIn.img.plane[2];

	uint8_t* line = data;
	for (int y = 0; y < m_x264Param.i_height; y+=2)
	{
		uint8_t* pixel1 = line;
		uint8_t* pixel2 = line + pitch;
		for (int x = 0; x < m_x264Param.i_width; x+=2)
		{
			float r,g,b, y1,y2, u1,u2,u3,u4, v1,v2,v3,v4;

			r = *pixel1++;
			g = *pixel1++;
			b = *pixel1;
			pixel1 += 2;
			y1 = 0.299f*r + 0.587f*g + 0.114f*b;
			u1 = (0.5f/(1-0.114f))*(b-y1);
			v1 = (0.5f/(1-0.299f))*(r-y1);

			r = *pixel1++;
			g = *pixel1++;
			b = *pixel1;
			pixel1 += 2;
			y2 = 0.299f*r + 0.587f*g + 0.114f*b;
			u2 = (0.5f/(1-0.114f))*(b-y2);
			v2 = (0.5f/(1-0.299f))*(r-y2);
			
			*yp1++ = (uint8_t)(y1*(219.0f/255.0f) + 16.5f); // RGB full range to YUV tv range
			*yp1++ = (uint8_t)(y2*(219.0f/255.0f) + 16.5f);

			r = *pixel2++;
			g = *pixel2++;
			b = *pixel2;
			pixel2 += 2;
			y1 = 0.299f*r + 0.587f*g + 0.114f*b;
			u3 = (0.5f/(1-0.114f))*(b-y1);
			v3 = (0.5f/(1-0.299f))*(r-y1);

			r = *pixel2++;
			g = *pixel2++;
			b = *pixel2;
			pixel2 += 2;
			y2 = 0.299f*r + 0.587f*g + 0.114f*b;
			u4 = (0.5f/(1-0.114f))*(b-y2);
			v4 = (0.5f/(1-0.299f))*(r-y2);

			*yp2++ = (uint8_t)(y1*(219.0f/255.0f) + 16.5f); // RGB full range to YUV tv range
			*yp2++ = (uint8_t)(y2*(219.0f/255.0f) + 16.5f); 

			*up++ = (uint8_t)((u1+u2+u3+u4)*(0.25f*224.0f/255.0f) + 128.5f);
			*vp++ = (uint8_t)((v1+v2+v3+v4)*(0.25f*224.0f/255.0f) + 128.5f);
		}
		yp1 += m_x264Param.i_width;
		yp2 += m_x264Param.i_width;
		line += pitch + pitch;
	}
}

void CX264::CopyFrameBT709( uint8_t* data, unsigned int pitch )
{
	uint8_t* yp1 = m_x264PicIn.img.plane[0];
	uint8_t* yp2 = yp1 + m_x264Param.i_width;
	uint8_t* up = m_x264PicIn.img.plane[1];
	uint8_t* vp = m_x264PicIn.img.plane[2];

	uint8_t* line = data;
	for (int y = 0; y < m_x264Param.i_height; y+=2)
	{
		uint8_t* pixel1 = line;
		uint8_t* pixel2 = line + pitch;
		for (int x = 0; x < m_x264Param.i_width; x+=2)
		{
			float r,g,b, y1,y2, u1,u2,u3,u4, v1,v2,v3,v4;

			r = *pixel1++;
			g = *pixel1++;
			b = *pixel1;
			pixel1 += 2;
			y1 = 0.2126f*r + 0.7152f*g + 0.0722f*b;
			u1 = (0.5f/(1-0.0722f))*(b-y1);
			v1 = (0.5f/(1-0.2126f))*(r-y1);

			r = *pixel1++;
			g = *pixel1++;
			b = *pixel1;
			pixel1 += 2;
			y2 = 0.2126f*r + 0.7152f*g + 0.0722f*b;
			u2 = (0.5f/(1-0.0722f))*(b-y2);
			v2 = (0.5f/(1-0.2126f))*(r-y2);

			*yp1++ = (uint8_t)(y1*(219.0f/255.0f) + 16.5f); // RGB full range to YUV tv range
			*yp1++ = (uint8_t)(y2*(219.0f/255.0f) + 16.5f);

			r = *pixel2++;
			g = *pixel2++;
			b = *pixel2;
			pixel2 += 2;
			y1 = 0.2126f*r + 0.7152f*g + 0.0722f*b;
			u3 = (0.5f/(1-0.0722f))*(b-y1);
			v3 = (0.5f/(1-0.2126f))*(r-y1);

			r = *pixel2++;
			g = *pixel2++;
			b = *pixel2;
			pixel2 += 2;
			y2 = 0.2126f*r + 0.7152f*g + 0.0722f*b;
			u4 = (0.5f/(1-0.0722f))*(b-y2);
			v4 = (0.5f/(1-0.2126f))*(r-y2);

			*yp2++ = (uint8_t)(y1*(219.0f/255.0f) + 16.5f); // RGB full range to YUV tv range
			*yp2++ = (uint8_t)(y2*(219.0f/255.0f) + 16.5f); 

			*up++ = (uint8_t)((u1+u2+u3+u4)*(0.25f*224.0f/255.0f) + 128.5f);
			*vp++ = (uint8_t)((v1+v2+v3+v4)*(0.25f*224.0f/255.0f) + 128.5f);
		}
		yp1 += m_x264Param.i_width;
		yp2 += m_x264Param.i_width;
		line += pitch + pitch;
	}
}


} } // namespaces

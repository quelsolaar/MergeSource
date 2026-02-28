#ifndef __PXC_WRAPPER_H__
#define __PXC_WRAPPER_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    // multi-profile detection
    VIEW_ANGLE_0          = 0x00000001,
    VIEW_ANGLE_45         = 0x00000002,
    VIEW_ANGLE_FRONTAL    = 0x00000004,
    VIEW_ANGLE_135        = 0x00000008,
    VIEW_ANGLE_180        = 0x00000010,

    //// multi-roll detection
    //VIEW_ROLL_30          = 0x00000020,
    //VIEW_ROLL_30N         = 0x00000040,
    //VIEW_ROLL_60          = 0x00000080,
    //VIEW_ROLL_60N         = 0x00000100,

    VIEW_ANGLE_HALF_MULTI = VIEW_ANGLE_FRONTAL | VIEW_ANGLE_45 | VIEW_ANGLE_135,
    VIEW_ANGLE_MULTI      = VIEW_ANGLE_HALF_MULTI | VIEW_ANGLE_0 | VIEW_ANGLE_180, //=0x0000001F
    //VIEW_ANGLE_FRONTALROLL= VIEW_ANGLE_FRONTAL | VIEW_ROLL_30| VIEW_ROLL_30N | VIEW_ROLL_60 | VIEW_ROLL_60N, //=0x000001E4
    VIEW_ANGLE_OMNI       = 0xFFFFFFFF,
} PXCViewAngle;

typedef struct
{
	unsigned int confidence;
	unsigned int rectangle[4];  /* x, y, w, h */
	PXCViewAngle view_angle;

	float left_eye_inner_corner[3];
	float left_eye_outer_corner[3];
	float right_eye_inner_corner[3];
	float right_eye_outer_corner[3];

	float yaw;
	float roll;
	float pitch;
} PXCFaceData;

extern int pxc_color_init(unsigned int width, unsigned int height);
extern int pxc_color_get_frame(unsigned char **buf_color, short **buf_depth, short **buf_accuracy, float **uvmap);
extern int pxc_color_release_frame();

extern int pxc_init(unsigned int width, unsigned int height);
extern int pxc_get_frame(unsigned char **buf_color, short **buf_depth, short **buf_accuracy, float **uvmap, PXCFaceData *face);
extern int pxc_release_frame();
extern int pxc_exit();

#ifdef __cplusplus
} // extern "C"
#endif

#endif

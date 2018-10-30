//================================================================================================================================
//
//  Copyright (c) 2015-2018 VisionStar Information Technology (Shanghai) Co., Ltd. All Rights Reserved.
//  EasyAR is the registered trademark or trademark of VisionStar Information Technology (Shanghai) Co., Ltd in China
//  and other countries for the augmented reality technology developed by VisionStar Information Technology (Shanghai) Co., Ltd.
//
//================================================================================================================================

#ifndef __EASYAR_SAMPLE_BOXRENDERER_H__
#define __EASYAR_SAMPLE_BOXRENDERER_H__

#include <objParser/objLoader.h>
#include "easyar/vector.hxx"
#include "easyar/matrix.hxx"

namespace easyar{
namespace samples{

class BoxRenderer
{
public:
    void init();
    void render(const Matrix44F& projectionMatrix, const Matrix44F& cameraview, Vec2F size);
    ~BoxRenderer();
private:
    unsigned int program_box;
    int pos_coord_box;
    int pos_coord_normal_box;
    int pos_color_box;
    int pos_scale_box;
    int pos_rotation_box;
    int pos_trans_box;
    int pos_proj_box;
    int light_source_box;
    unsigned int vbo_coord_box;
    unsigned int vbo_coord_normal_box;
    unsigned int vbo_color_box;
    unsigned int vbo_color_box_2;
    unsigned int vbo_faces_box;
    GLuint m_vao;

    objLoader modelLoader;
};

}
}

#endif

//================================================================================================================================
//
//  Copyright (c) 2015-2018 VisionStar Information Technology (Shanghai) Co., Ltd. All Rights Reserved.
//  EasyAR is the registered trademark or trademark of VisionStar Information Technology (Shanghai) Co., Ltd in China
//  and other countries for the augmented reality technology developed by VisionStar Information Technology (Shanghai) Co., Ltd.
//
//================================================================================================================================

#include "boxrenderer.hpp"
#include <android/log.h>

#define APPNAME "MyApp"
#if defined __APPLE__
#include <OpenGLES/ES2/gl.h>
#else
#include <GLES2/gl2.h>
#endif

const char* box_vert=
        "uniform mat4 trans;\n"
        "uniform mat4 proj;\n"
        "uniform vec4 rotation;\n"
        "attribute vec4 coord;\n"
        "attribute vec4 coord_normal;\n"
        //"attribute vec4 color;\n"
        "attribute vec4 scale;\n"
		"varying vec4 pos;\n"
        "varying vec4 N;\n"
        "varying vec4 vcolor;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "    pos = coord;\n"
        "    N = coord_normal;\n"
        "    float angle = radians(rotation[0]);\n"
        "    float cosAngle = cos(angle);\n"
        "    float sinAngle = sin(angle);\n"
        "// Specify the axis to rotate about:\n"
        "float x = rotation[1];\n"
        "float y = rotation[2];\n"
        "float z = rotation[3];\n"
        "vec3 q;\n"
        "vec3 p = pos.xyz;\n"
        "q.x = p.x * (x*x * (1.0 - cosAngle) + cosAngle)\n"
        "    + p.y * (x*y * (1.0 - cosAngle) + z * sinAngle)\n"
        "    + p.z * (x*z * (1.0 - cosAngle) - y * sinAngle);\n"
        "q.y = p.x * (y*x * (1.0 - cosAngle) - z * sinAngle)\n"
        "    + p.y * (y*y * (1.0 - cosAngle) + cosAngle)\n"
        "    + p.z * (y*z * (1.0 - cosAngle) + x * sinAngle);\n"
        "q.z = p.x * (z*x * (1.0 - cosAngle) + y * sinAngle)\n"
        "    + p.y * (z*y * (1.0 - cosAngle) - x * sinAngle)\n"
        "    + p.z * (z*z * (1.0 - cosAngle) + cosAngle);\n"
        "pos = vec4(q, 1.0);\n"
        //"    pos *= rot;\n"
        "vcolor = vec4(1.0, 1.0, 1.0, 0.5);\n"
        "gl_Position = proj*trans*pos;\n"
        "}\n"
        "\n"
;

const char* box_frag="#ifdef GL_ES\n"
        "precision highp float;\n"
        "#endif\n"
        "varying vec4 vcolor;\n"
        "varying vec4 pos;\n"
        "varying vec4 N;\n"
        "uniform vec3 lightSource;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "    float intensity =0.5;\n"
        //"    intensity = max(dot(normalize(lightSource - pos.xyz), N.xyz),0.0);\n"
        "    intensity = max(dot(normalize(lightSource - pos.xyz), N.xyz),0.0);\n"
        "    gl_FragColor = vec4(vcolor.xyz*intensity, 1.0);\n"
        //"    gl_FragColor = vec4(lightSource, 0.9);\n"
        "}\n"
        "\n"
;

namespace easyar{
namespace samples{

    BoxRenderer::~BoxRenderer()
    {
        if (modelLoader.m_orderedVertexes != NULL) {
            delete[] modelLoader.m_orderedVertexes;
            modelLoader.m_orderedVertexes = NULL;
        }
        if (modelLoader.m_orderedNormals != NULL) {
            delete[] modelLoader.m_orderedNormals;
            modelLoader.m_orderedNormals = NULL;
        }
    }


void BoxRenderer::init()
{
    // test debug //
    modelLoader.load("/storage/emulated/0/Android/skull.obj", "", false, 0.05f);

    // creates faces
    for (int i = 0; i < modelLoader.faceCount; i++)
    {
        obj_face *o = modelLoader.faceList[i];

        for (int j = 0; j < o->vertex_count; j++)
        {
            modelLoader.m_indexes.push_back(o->vertex_index[j]);
        }
    }

    modelLoader.m_orderedVertexes = new GLfloat[modelLoader.vertexCount * 3];
    for (int i = 0; i < modelLoader.vertexCount; i++)
    {
        modelLoader.m_orderedVertexes[i*3] = modelLoader.vertexList[i]->e[0];
        modelLoader.m_orderedVertexes[(i*3)+1] = modelLoader.vertexList[i]->e[1];
        modelLoader.m_orderedVertexes[(i*3)+2] = modelLoader.vertexList[i]->e[2];
    }

    modelLoader.m_orderedNormals = new GLfloat[modelLoader.normalCount * 3];
    for (int i = 0; i < modelLoader.vertexCount; i++)
    {
        modelLoader.m_orderedNormals[i*3] = modelLoader.normalList[i]->e[0];
        modelLoader.m_orderedNormals[(i*3)+1] = modelLoader.normalList[i]->e[1];
        modelLoader.m_orderedNormals[(i*3)+2] = modelLoader.normalList[i]->e[2];
    }




    // end of test //

    program_box = glCreateProgram();

    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &box_vert, 0);
    glCompileShader(vertShader);

    GLint logSize = 0;
    glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &logSize);
    std::vector<GLchar> errorLog(logSize);
    glGetShaderInfoLog(vertShader, logSize, &logSize, &errorLog[0]);
    if (errorLog.size() > 10)
    {
        printf("error %s", errorLog[0]);
    }

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &box_frag, 0);
    glCompileShader(fragShader);

    glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logSize);
    glGetShaderInfoLog(fragShader, logSize, &logSize, &errorLog[0]);
    if (errorLog.size() > 10)
    {
        printf("error %s", errorLog[0]);
    }

    glAttachShader(program_box, vertShader);
    glAttachShader(program_box, fragShader);
    glLinkProgram(program_box);
    glUseProgram(program_box);
    pos_coord_box = glGetAttribLocation(program_box, "coord");
    pos_coord_normal_box = glGetAttribLocation(program_box, "coord_normal");
    //pos_color_box = glGetAttribLocation(program_box, "color");
    pos_trans_box = glGetUniformLocation(program_box, "trans");
    pos_proj_box = glGetUniformLocation(program_box, "proj");
    pos_scale_box = glGetAttribLocation(program_box, "scale");
    pos_rotation_box = glGetUniformLocation(program_box, "rotation");
    light_source_box = glGetUniformLocation(program_box, "lightSource");

    // generates the VAO
    //glGenVertexArrays(1, &m_vao);
    // binds the VAO for this model
    //glBindVertexArray(m_vao);

    glGenBuffers(1, &vbo_coord_normal_box);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_coord_normal_box);

    glGenBuffers(1, &vbo_coord_box);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_coord_box);
    const GLfloat cube_vertices[8][3] = {
        /* +z */{1.0f / 2, 1.0f / 2, 0.01f / 2}, {1.0f / 2, -1.0f / 2, 0.01f / 2}, {-1.0f / 2, -1.0f / 2, 0.01f / 2}, {-1.0f / 2, 1.0f / 2, 0.01f / 2},
        /* -z */{1.0f / 2, 1.0f / 2, -0.01f / 2}, {1.0f / 2, -1.0f / 2, -0.01f / 2}, {-1.0f / 2, -1.0f / 2, -0.01f / 2}, {-1.0f / 2, 1.0f / 2, -0.01f / 2}};
    //glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_DYNAMIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * modelLoader.vertexCount * 3, modelLoader.m_orderedVertexes, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(pos_coord_box, 3, GL_FLOAT, GL_FALSE, 0, 0);

    /*glGenBuffers(1, &vbo_color_box);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color_box);
    const GLubyte cube_vertex_colors[8][4] = {
        {255, 0, 0, 128}, {0, 255, 0, 128}, {0, 0, 255, 128}, {0, 0, 0, 128},
        {0, 255, 255, 128}, {255, 0, 255, 128}, {255, 255, 0, 128}, {255, 255, 255, 128}};
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertex_colors), cube_vertex_colors, GL_STATIC_DRAW);

    glGenBuffers(1, &vbo_color_box_2);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color_box_2);
    const GLubyte cube_vertex_colors_2[8][4] = {
        {255, 0, 0, 255}, {255, 255, 0, 255}, {0, 255, 0, 255}, {255, 0, 255, 255},
        {255, 0, 255, 255}, {255, 255, 255, 255}, {0, 255, 255, 255}, {255, 0, 255, 255}};
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertex_colors_2), cube_vertex_colors_2, GL_STATIC_DRAW);*/

    glGenBuffers(1, &vbo_faces_box);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_faces_box);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * modelLoader.m_indexes.size(), &modelLoader.m_indexes[0], GL_STATIC_DRAW);

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_faces_box);
    //const GLushort cube_faces[6][4] = {
    //    /* +z */{3, 2, 1, 0}, /* -y */{2, 3, 7, 6}, /* +y */{0, 1, 5, 4},
    //    /* -x */{3, 0, 4, 7}, /* +x */{1, 2, 6, 5}, /* -z */{4, 5, 6, 7}};
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_faces), cube_faces, GL_STATIC_DRAW);
}

void BoxRenderer::render(const Matrix44F& projectionMatrix, const Matrix44F& cameraview, Vec2F size)
{
    float size0 = size.data[0];
    float size1 = size.data[1];

    /*glBindBuffer(GL_ARRAY_BUFFER, vbo_coord_box);
    float height = size0 / 1000;
    const GLfloat cube_vertices[8][3] = {
        {size0 / 2, size1 / 2, height / 2}, {size0 / 2, -size1 / 2, height / 2}, {-size0 / 2, -size1 / 2, height / 2}, {-size0 / 2, size1 / 2, height / 2}, //+z
        {size0 / 2, size1 / 2, 0}, {size0 / 2, -size1 / 2, 0}, {-size0 / 2, -size1 / 2, 0}, {-size0 / 2, size1 / 2, 0}}; // -z
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_DYNAMIC_DRAW);*/

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glUseProgram(program_box);
    /*glEnableVertexAttribArray(pos_scale_box);
    glVertexAttribPointer(pos_scale_box, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_coord_box);
    glEnableVertexAttribArray(pos_coord_box);
    glVertexAttribPointer(pos_coord_box, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color_box);
    glEnableVertexAttribArray(pos_color_box);
    glVertexAttribPointer(pos_color_box, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);*/
    glUniform3f(light_source_box,
                cameraview.data[8],
                cameraview.data[9],
                cameraview.data[10]);
                //5.0f, 0.0f, 0.0f);

    static int noFlood = 0;
    if (noFlood++ > 10) {
        __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "<1> X %f Y %f Z %f", cameraview.data[0],
                            cameraview.data[1], cameraview.data[2]);
        __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "<2> X %f Y %f Z %f", cameraview.data[4],
                            cameraview.data[5], cameraview.data[6]);
        __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "<3> X %f Y %f Z %f", cameraview.data[8],
                            cameraview.data[9], cameraview.data[10]);
        __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "----------------------");
        noFlood = 0;
    }

    glUniformMatrix4fv(pos_trans_box, 1, 0, cameraview.data);
    glUniformMatrix4fv(pos_proj_box, 1, 0, projectionMatrix.data);
    static float rotateAngle = 1.0f;
    //rotateAngle += 1.0f;
    glUniform4f(pos_rotation_box, rotateAngle, 0.f, 1.f, 0.f);
    //glUniform4f(pos_rotation_box, 15., 1.f, 0.f, 0.f);
    glUniform4f(pos_scale_box, size0, size1, 1.f, 1.f);
    //glUniform3f(light_source_box, 0.0f, -5.0f, 0.0f);

    /*glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_faces_box);
    for(int i = 0; i < 6; i++) {
        glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT, (void*)(i * 4 * sizeof(GLushort)));
    }*/

    glBindBuffer(GL_ARRAY_BUFFER, vbo_coord_box);
    //const GLfloat cube_vertices_2[8][3] = {
    //    {size0 / 4, size1 / 4, size0 / 4},{size0 / 4, -size1 / 4, size0 / 4},{-size0 / 4, -size1 / 4, size0 / 4},{-size0 / 4, size1 / 4, size0 / 4}, // +z
    //    {size0 / 4, size1 / 4, 0},{size0 / 4, -size1 / 4, 0},{-size0 / 4, -size1 / 4, 0},{-size0 / 4, size1 / 4, 0}}; // -z

    //if (((void*)cube_vertices_2) != ((void*)modelLoader.vertexList))
    //glBufferData(GL_ARRAY_BUFFER, sizeof(obj_vector) * modelLoader.vertexCount, modelLoader.vertexList, GL_DYNAMIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * modelLoader.vertexCount * 3, modelLoader.m_orderedVertexes, GL_DYNAMIC_DRAW);

    //glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8 * 3, cube_vertices_2, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(pos_coord_box);
    glVertexAttribPointer(pos_coord_box, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_coord_normal_box);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * modelLoader.normalCount * 3, modelLoader.m_orderedNormals, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(pos_coord_normal_box);
    glVertexAttribPointer(pos_coord_normal_box, 3, GL_FLOAT, GL_FALSE, 0, 0);
    //glBindBuffer(GL_ARRAY_BUFFER, vbo_color_box_2);
    //glEnableVertexAttribArray(pos_color_box);
    //glVertexAttribPointer(pos_color_box, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
    if (modelLoader.faceList != NULL) {
        glDrawElements(GL_TRIANGLES, modelLoader.m_indexes.size(), GL_UNSIGNED_SHORT, (void*)(0));
    }

    //for(int i = 0; i < 6; i++) {
    //    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT, (void*)(i * 4 * sizeof(GLushort)));
    //}

}

}
}

#include "objLoader.h"
#include "obj_parser.h"


int objLoader::load(char *filename, char* material, bool isStreamOfBytes, float resizeFactor)
{
	int no_error = 1;
	if (isStreamOfBytes)
	{
		no_error = parse_obj_scene_from_stream(&data, filename, material);
	}
	else
	{
		no_error = parse_obj_scene(&data, filename, resizeFactor);
	}
	if(no_error)
	{
		this->vertexCount = data.vertex_count;
		this->normalCount = data.vertex_normal_count;
		this->textureCount = data.vertex_texture_count;

		this->faceCount = data.face_count;
		this->sphereCount = data.sphere_count;
		this->planeCount = data.plane_count;

		this->lightPointCount = data.light_point_count;
		this->lightDiscCount = data.light_disc_count;
		this->lightQuadCount = data.light_quad_count;

		this->materialCount = data.material_count;

		this->vertexList = data.vertex_list;
		this->normalList = data.vertex_normal_list;
		this->textureList = data.vertex_texture_list;

		this->faceList = data.face_list;
		this->sphereList = data.sphere_list;
		this->planeList = data.plane_list;

		this->lightPointList = data.light_point_list;
		this->lightDiscList = data.light_disc_list;
		this->lightQuadList = data.light_quad_list;

		this->materialList = data.material_list;

		this->camera = data.camera;
	}

	return no_error;
}
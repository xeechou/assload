#include <cstddef>
#include <iostream>
#include <exception>
#include <string>
#include <filesystem>

#include <assimp/version.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <assimp/pbrmaterial.h>
#include <assimp/color4.h>
#include <assimp/types.h>
#include <assimp/vector2.h>
#include <assimp/vector3.h>


namespace fs = std::filesystem;

enum DIM {
	ZERO, //return false
	ONE,
	THREE,
	FOUR,
};

static std::string get_shading_model(aiMaterial *material)
{
	std::string model;
	int type;
	material->Get(AI_MATKEY_SHADING_MODEL, type);
	switch (type) {
	case aiShadingMode_Flat:
		model = "Flat";
		break;
	case aiShadingMode_Gouraud:
		model = "Gouraud";
		break;
	case aiShadingMode_Phong:
		model = "Phong";
		break;
	case aiShadingMode_Blinn:
		model = "Blinn-Phong";
		break;
	case aiShadingMode_Toon:
		model = "Toon-Shading";
		break;
	case aiShadingMode_OrenNayar:
		model = "Oren-Nayer";
		break;
	case aiShadingMode_Minnaert:
		model = "Minnaert";
		break;
	case aiShadingMode_CookTorrance:
		model = "Cook-Torrance";
		break;
	case aiShadingMode_Fresnel:
		model = "Fresnel";
		break;
	case aiShadingMode_NoShading:
		model = "No shading";
		break;
	default:
		model = "Uknown";
		break;
	}
	return model;
}

static bool assimp_get_texture_path(aiMaterial*    material,
                                    aiTextureType texture_type, int idx,
                                    std::string&  path)
{
    aiString ai_path;
    aiReturn result = material->GetTexture(texture_type, idx, &ai_path);

    if (result == aiReturn_FAILURE)
    {
        return false;
    }
    else
    {
        path = std::string(ai_path.C_Str());
        return !path.empty();
    }
}

static bool assimp_get_value(aiMaterial *material,
                             const char *key, int i0, int i1, DIM dim,
                             aiColor4D& ret)
{
	aiReturn success = aiReturn_SUCCESS;
	float     value = 0;
	aiColor3D value3;

	switch (dim) {
	case ZERO:
		success = aiReturn_FAILURE;
		return false;
	case ONE:
		success = material->Get(key, i0, i1, value);
		ret.r = value;
		break;
	case THREE:
		success = material->Get(key, i0, i1, value3);
		ret.r = value3.r; ret.g = value3.g; ret.b = value3.b; ret.a = 1.0f;
		break;
	case FOUR:
		success = material->Get(key, i0, i1, ret);
		break;
	}
	return success == aiReturn_SUCCESS;
}

//refer to https://assimp-docs.readthedocs.io/en/latest/usage/use_the_lib.html?highlight=material#material-system for value types

static void print_material(aiMaterial *material)
{
	std::cout << "\t" << "material: " << material->GetName().C_Str() << " ";
	std::cout << "Shading Model: " << get_shading_model(material) << std::endl;

	struct { aiTextureType type; int idx;           //texture
		const char *key; int i0, i1; DIM dim;   // value
		const char *name;
	} types [] = {
		{AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE,
		 AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR, DIM::THREE,
		 "glft-base-color"},
		{AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE,
		 AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR, DIM::ONE,
		 "gltf-roughness"},
		{AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE,
		 AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, DIM::ONE,
		 "gltf-roughness"},
		//legacy textures
		{aiTextureType_DIFFUSE, 0,
		 AI_MATKEY_COLOR_DIFFUSE, DIM::THREE,
		 "diffuse"},
		{aiTextureType_SPECULAR, 0,
		 AI_MATKEY_COLOR_SPECULAR, DIM::THREE,
		 "specular"},
		{aiTextureType_AMBIENT, 0,
		 AI_MATKEY_COLOR_AMBIENT, DIM::THREE,
		 "ambient"},
		{aiTextureType_EMISSIVE, 0,
		 AI_MATKEY_COLOR_EMISSIVE, DIM::THREE,
		 "emissive"},
		{aiTextureType_HEIGHT,             0,
		 nullptr, 0, 0, DIM::ZERO,
		 "height"},
		{aiTextureType_NORMALS, 0,
		 nullptr, 0, 0, DIM::ZERO,
		 "normal"},
		{aiTextureType_SHININESS, 0,
		 AI_MATKEY_SHININESS, DIM::ONE, //false
		 "shininess"},
		{aiTextureType_OPACITY,            0,
		 AI_MATKEY_OPACITY, DIM::ONE,
		 "opacity"},
		{aiTextureType_DISPLACEMENT,       0,
		 nullptr, 0, 0, DIM::ZERO,
		 "displacement"} ,
		{aiTextureType_LIGHTMAP,           0,
		 nullptr, 0, 0, DIM::ZERO,
		"light-map"},
		{aiTextureType_REFLECTION,         0,
		 AI_MATKEY_COLOR_REFLECTIVE, DIM::THREE,
		 "reflection"},
		//pbr workflows
		{aiTextureType_BASE_COLOR,         0,
		 nullptr, 0, 0, DIM::ZERO,
		 "pbr-base-color"},
		{aiTextureType_NORMAL_CAMERA,      0,
		 nullptr, 0, 0, DIM::ZERO,
		 "pbr-normal"},
		{aiTextureType_EMISSION_COLOR,     0,
		 nullptr, 0, 0, DIM::ZERO,
		 "pbr-emissive"},
		{aiTextureType_METALNESS,          0,
		 nullptr, 0, 0, DIM::ZERO,
		 "pbr-metallic"},
		{aiTextureType_DIFFUSE_ROUGHNESS,  0,
		 nullptr, 0, 0, DIM::ZERO,
		 "pbr-roughness"},
		{aiTextureType_AMBIENT_OCCLUSION,  0,
		 nullptr, 0, 0, DIM::ZERO,
		 "pbr-ambient-occlusion"},
	};

	for (auto type : types) {
		aiColor4D value;
		std::string texture_path = "";

		bool success = assimp_get_texture_path(material,
		                                      type.type, type.idx,
		                                      texture_path) ||
			assimp_get_value(material, type.key, type.i0, type.i1, type.dim,
			                 value);
		std::cout << "\t\t" << type.name << ": ";
		if (success && (!texture_path.empty())) {
			std::cout << texture_path;
		} else if (success) {
			std::cout << "(" << value.r << ", " << value.g << ", ";
			std::cout << value.b << ", " << value.a  << ")";
		}
		std::cout << std::endl;
	}

	std::cout << std::endl << std::endl;
}

int main(int argc, char *argv[])
{
	std::cout << "compiled with assimp Version: ";
	std::cout << aiGetVersionMajor() << "." << aiGetVersionMinor() << ".";
	std::cout << aiGetVersionRevision() << std::endl;

	//we just try out lucks.
	fs::path path = argv[1];

	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(path.string(),
	                                         aiProcess_Triangulate |
	                                         aiProcess_GenSmoothNormals |
	                                         aiProcess_FlipUVs |
	                                         aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		throw std::runtime_error(std::string("ERROR::Assimp:: ")  +
			                         importer.GetErrorString());
	}
	std::cout << "print materials for " << path.string() << std::endl;
	//process material
	for (unsigned i = 0; i < scene->mNumMaterials; i++) {
		aiMaterial *material = scene->mMaterials[i];
		print_material(material);

	}
	return 0;
}

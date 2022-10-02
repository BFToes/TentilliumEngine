#include "LoadSystem.h"
#include "../Components/Mesh.h"
#include "../Components/Hierarchy.h"
#include "../Components/Transform.h"
#include "../Components/Material.h"
#include "../Rendering/Resources/Texture.h"
#include <queue>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <filesystem>

// hierarchy		*DONE*
// transform		*DONE*
// mesh				*DONE*
// material			*DONE* ish
// animation
// lightEmitter
// shadowCastor
// Bones
// Skinning

//#define MESH_FILENAME(filepath, node, mesh) filepath + "/" + node->mName.C_Str() + "/" + mesh->mName.C_Str()
//#define MAT_FILENAME(filepath, node, mat) filepath + "/" + node->mName.C_Str() + "/" + mat->GetName().C_Str()

// TODO:
// add bounding box to Transform vao combo for screen space culling optimizations
// boneID and transform indices
// animation

// Framebuffer Needs:
//	position		vec3
//	depth			float
//	normal			vec3
//	texcoord		vec2
//	material Index	unsigned int

entt::entity LoadSystem::load(std::string filepath)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(filepath,
		aiProcess_Triangulate |
		aiProcess_FlipUVs |
		aiProcess_GlobalScale |
		aiProcess_EmbedTextures |
		aiProcess_OptimizeGraph | 
		aiProcess_OptimizeMeshes |
		aiProcess_CalcTangentSpace | 
		aiProcess_GenUVCoords | 
		aiProcess_JoinIdenticalVertices);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cerr << "[Loading Error] : " << importer.GetErrorString() << std::endl;
		throw std::exception();
	}
	
	// array of textures
	std::vector<Texture> textures;
	if (scene->HasTextures()) {
		auto texPtr = scene->mTextures;
		for (unsigned int i = 0; i < scene->mNumTextures; i++) {
			textures.push_back(Texture::get_or_default(texPtr[i]->mFilename.C_Str(),
				texPtr[i]->mWidth, texPtr[i]->mHeight, Texture::Format::RGBA, texPtr[i]->pcData));
		}
	}
	
	entt::entity e = create();

	if (scene->HasMeshes())
	{
		// get size needed for buffers
		unsigned int vertexCount = 0, faceCount = 0;
		for (unsigned int mesh_i = 0; mesh_i < scene->mNumMeshes; mesh_i++) {
			vertexCount += scene->mMeshes[mesh_i]->mNumVertices;
			faceCount += scene->mMeshes[mesh_i]->mNumFaces;
		}

		// iteterate over each mesh
		unsigned int vertexOffset = 0, faceOffset = 0, boneOffset = 0;
		for (unsigned int mesh_i = 0; mesh_i < scene->mNumMeshes; mesh_i++)
		{
			auto aiMeshPtr = scene->mMeshes[mesh_i];

			if (aiMeshPtr->HasFaces())
			{
				std::vector<unsigned int> indices;
				for (unsigned int face_i = 0; face_i < aiMeshPtr->mNumFaces; face_i++)
				{
					auto& face = aiMeshPtr->mFaces[face_i];
					for (unsigned int index_i = 0; index_i < face.mNumIndices; index_i++)
						indices.push_back(face.mIndices[index_i] + vertexOffset);
				}
				get_or_emplace<Mesh::VBO<Mesh::V_Index>>(e, nullptr, sizeof(int) * faceCount * 3)
					.setData(&indices[0], sizeof(int) * 3 * aiMeshPtr->mNumFaces, sizeof(int) * 3 * faceOffset);
			}

			if (aiMeshPtr->HasPositions()) {
				get_or_emplace<Mesh::VBO<Mesh::V_Position>>(e, nullptr, sizeof(float) * 3 * vertexCount)
					.setData(aiMeshPtr->mVertices, sizeof(float) * 3 * aiMeshPtr->mNumVertices, sizeof(float) * 3 * vertexOffset);
			}

			if (aiMeshPtr->HasNormals()) {
				get_or_emplace<Mesh::VBO<Mesh::V_Normal>>(e, nullptr, sizeof(float) * 3 * vertexCount)
					.setData(aiMeshPtr->mNormals, sizeof(float) * 3 * aiMeshPtr->mNumVertices, sizeof(float) * 3 * vertexOffset);

				get_or_emplace<Mesh::VBO<Mesh::V_Tangent>>(e, nullptr, sizeof(float) * 3 * vertexCount)
					.setData(aiMeshPtr->mTangents, sizeof(float) * 3 * aiMeshPtr->mNumVertices, sizeof(float) * 3 * vertexOffset);
			}

			if (aiMeshPtr->HasTextureCoords(0)) {
				std::vector<float> texCoords;
				for (unsigned int uv_i = 0; uv_i < aiMeshPtr->mNumVertices; uv_i++) {
					texCoords.push_back(aiMeshPtr->mTextureCoords[0][uv_i].x);
					texCoords.push_back(aiMeshPtr->mTextureCoords[0][uv_i].y);
				}
				get_or_emplace<Mesh::VBO<Mesh::V_TexCoord>>(e, nullptr, sizeof(float) * vertexCount * 2)
					.setData(&texCoords[0], sizeof(float) * aiMeshPtr->mNumVertices * 2, sizeof(float) * vertexOffset * 2);
			}

			/*if (scene->HasMaterials())*/       {
				std::vector<unsigned int> materialIndex(aiMeshPtr->mNumVertices, mesh_i);
				get_or_emplace<Mesh::VBO<Mesh::V_MaterialIndex>>(e, nullptr, sizeof(unsigned int) * vertexCount)
					.setData(&materialIndex[0], sizeof(unsigned int) * aiMeshPtr->mNumVertices, sizeof(unsigned int) * vertexOffset);
			}

			vertexOffset += aiMeshPtr->mNumVertices;
			faceOffset += aiMeshPtr->mNumFaces;
		}
	
		//for (unsigned int material_i = 0; material_i < scene->mNumMeshes; material_i++)
	}

	if (scene->HasMaterials())
	{
		aiMaterial* material = scene->mMaterials[0];

		Material::InitData data;
		// diffuse map
		{
			aiColor3D colour;
			aiString texture;
			if (material->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), texture) == AI_SUCCESS)
				data.diffuse = Texture::get(texture.C_Str());
			else if (material->Get(AI_MATKEY_COLOR_DIFFUSE, colour) == AI_SUCCESS)
				data.diffuse = glm::vec3(colour.r, colour.g, colour.b);
		}
		// opacity map
		{
			float value;
			aiString texture;
			if (material->Get(AI_MATKEY_TEXTURE_OPACITY(0), texture) == AI_SUCCESS)
				data.opacity = Texture::get(texture.C_Str());
			else if (material->Get(AI_MATKEY_OPACITY, value) == AI_SUCCESS)
				data.opacity = value;
		}
		// specular map
		{
			aiColor3D colour;
			aiString texture;
			if (material->Get(AI_MATKEY_TEXTURE_SPECULAR(0), texture) == AI_SUCCESS)
				data.specular = Texture::get(texture.C_Str());
			else if (material->Get(AI_MATKEY_COLOR_SPECULAR, colour) == AI_SUCCESS)
				data.specular = glm::vec3(colour.r, colour.g, colour.b);

		}
		// shininess map
		{
			float value;
			aiString texture;
			if (material->Get(AI_MATKEY_TEXTURE_SHININESS(0), texture) == AI_SUCCESS)
				data.shininess = Texture::get(texture.C_Str());
			else if (material->Get(AI_MATKEY_SHININESS, value) == AI_SUCCESS)
				data.shininess = value;
		}
		// emissive map
		{
			aiColor3D colour;
			aiString texture;
			if (material->Get(AI_MATKEY_TEXTURE_EMISSIVE(0), texture) == AI_SUCCESS)
				data.shininess = Texture::get(texture.C_Str());
			else if (material->Get(AI_MATKEY_COLOR_EMISSIVE, colour) == AI_SUCCESS)
				data.emissive = glm::vec3(colour.r, colour.g, colour.b);
		}
		// ambient occlusion map
		{
			aiString texture;
			if (material->Get(AI_MATKEY_TEXTURE_LIGHTMAP(0), texture) == AI_SUCCESS)
				data.ambientOcclusion = Texture::get(texture.C_Str());
		}
		// height map
		{
			aiString texture;
			if (material->Get(AI_MATKEY_TEXTURE_HEIGHT(0), texture) == AI_SUCCESS)
				data.ambientOcclusion = Texture::get(texture.C_Str());
		}
		// normal map
		{
			aiString texture;
			if (material->Get(AI_MATKEY_TEXTURE_NORMALS(0), texture) == AI_SUCCESS)
				data.normal = Texture::get(texture.C_Str());
		}

		auto& mat = emplace<Material>(e, data);
	}
	
	return e;
}
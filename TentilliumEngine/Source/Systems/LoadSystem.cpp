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
// optimize scene graph
//		-> not impressed by assimp graph optimization. 
//		-> automatically sort the useless nodes and attach them to parents to reduce the node spam
//		-> its kinda ridiculous
//		-> requires collection of parts and delayed assigning of components???
//		-> alternatively I could just assign the meshes to the parent entity

// add bounding box to Transform vao combo for screen space culling optimizations
//		-> Make Transform a requirement of Specular system???
//		

// how to solve multiple meshes per entity
//		combine meshes in to single vaos and vbos 
//			-> could adapt vao and material to distinguish meshes with indexes of the mesh
//			-> probably best to keep them seperate tho
//			-> requires manipulation of the raw data
//			-> requires changes to material to identify the mesh groups
//			changes to material:
//				1:n relationship for entity to material
//					-> complicated and not necessarily a performance improvement
//					-> better syntax
//				a vector in material to identify the seperations 
//					-> simple
//					-> nasty to set material properties
// 
//		make mesh a child entity with vbos, vao and components
//			-> simple
//			-> bad syntax
//			-> not very data oriented
//			-> doesnt have transform component so will get annoying 
// 
//		
//		
//			

// blender's vertex count is wrong -> it does not identify unique vertices properly
// if a 2 vertices have the same position but different normals it is incorrectly
// marked as unique.

// restructure to prioritise relevant nodes:
// root node + Assimilated meshes + transform

entt::entity LoadSystem::load(std::string filepath)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(filepath,
		aiProcess_Triangulate |
		aiProcess_FlipUVs |
		aiProcess_GlobalScale |
		aiProcess_EmbedTextures |
		aiProcess_OptimizeGraph | // doesnt do enough
		aiProcess_OptimizeMeshes |
		aiProcess_CalcTangentSpace | 
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
				texPtr[i]->mWidth, texPtr[i]->mHeight, Texture::Format::RGBA, false, texPtr[i]->pcData));
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

			vertexOffset += aiMeshPtr->mNumVertices;
			faceOffset += aiMeshPtr->mNumFaces;
		}

		// has bones
	}

	if (scene->HasMaterials()) {
		
		for (unsigned int material_i = 0; material_i < scene->mNumMaterials; material_i++)
		{
			auto material = scene->mMaterials[material_i];
			std::cout << "Material " << material->GetName().C_Str() << std::endl;

			for (unsigned int property_i = 0; property_i < material->mNumProperties; property_i++) {
				auto prop = material->mProperties[property_i];
				
				std::cout << "name: " << prop->mKey.C_Str() << " type: ";
				if (prop->mType == aiPropertyTypeInfo::aiPTI_Buffer)std::cout << "buffer";
				if (prop->mType == aiPropertyTypeInfo::aiPTI_Double)std::cout << "double";
				if (prop->mType == aiPropertyTypeInfo::aiPTI_Float)std::cout << "float";
				if (prop->mType == aiPropertyTypeInfo::aiPTI_Integer)std::cout << "int";
				if (prop->mType == aiPropertyTypeInfo::aiPTI_String)std::cout << "string";
				if (prop->mType == aiPropertyTypeInfo::_aiPTI_Force32Bit) std::cout << "force 32bits";
				if (prop->mDataLength != 4) std::cout << " x " << prop->mDataLength / 4;
				std::cout << std::endl;
			}

		}

		{
			auto material = scene->mMaterials[0];

			std::variant<Texture, glm::vec4> diffuse;
			std::variant<Texture, float> specular;
			std::variant<Texture, float> gloss;
			std::optional<Texture> normal;
			{
				aiColor3D colour;
				aiString texture;
				if (material->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), texture) == AI_SUCCESS)
					diffuse = Texture::get(texture.C_Str());
				else if (material->Get(AI_MATKEY_COLOR_DIFFUSE, colour) == AI_SUCCESS)
					diffuse = glm::vec4(colour.r, colour.g, colour.b, 1);
			}

			{
				aiColor3D colour;
				aiString texture;
				if (material->Get(AI_MATKEY_TEXTURE_SPECULAR(0), texture) == AI_SUCCESS)
					specular = Texture::get(texture.C_Str());
				else if (material->Get(AI_MATKEY_COLOR_SPECULAR, colour) == AI_SUCCESS)
					specular = colour.r;

			}

			{
				float value;
				aiString texture;
				if (material->Get(AI_MATKEY_TEXTURE_SHININESS(0), texture) == AI_SUCCESS)
					gloss = Texture::get(texture.C_Str());
				else if (material->Get(AI_MATKEY_SHININESS, value) == AI_SUCCESS)
					gloss = value;
			}

			{
				aiColor3D colour;
				aiString texture;
				if (material->Get(AI_MATKEY_TEXTURE_NORMALS(0), texture) == AI_SUCCESS)
					normal = Texture::get(texture.C_Str());
			}

			auto& mat = emplace<Material>(e, diffuse, specular, gloss, normal);
		
		}
	}
	return e; // return scene root 
}
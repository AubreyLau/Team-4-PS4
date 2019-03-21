#include "PS4Mesh.h"
#include "../../Common/Vector2.h"
#include "../../Common/Vector3.h"
#include "../../Common/Vector4.h"
#include "../../Common/Assets.h"

#include <fstream>
#include <string>
#include <.\graphics\api_gnm\toolkit\allocators.h>
#include <.\graphics\api_gnm\toolkit\stack_allocator.h>


using namespace NCL::Maths;
using namespace NCL::PS4;
PS4Mesh::PS4Mesh()	{
	indexBuffer		= 0;
	vertexBuffer	= 0;
	attributeCount	= 0;
}


PS4Mesh::PS4Mesh(const std::string&filename):MeshGeometry(filename) {
	
}

PS4Mesh* PS4Mesh::GenerateMesh(const std::string & filename)
{
	PS4Mesh* mesh = new PS4Mesh(filename);
	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriList;
	mesh->UploadToGPU();
	return mesh;
}

PS4Mesh::~PS4Mesh()	{
	delete[] attributeBuffers;
}

PS4Mesh* PS4Mesh::GenerateQuad() {
	PS4Mesh* mesh = new PS4Mesh();

	mesh->indexType		= sce::Gnm::IndexSize::kIndexSize32;
	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;

	mesh->SetVertexPositions({ Vector3(-1.0f, -1.0f, 0.0f), Vector3(-1.0f,  1.0f, 0.0f),Vector3(1.0f, -1.0f, 0.0f), Vector3(1.0f,  1.0f, 0.0f) });

	std::vector<Vector3> normals;
	std::vector<Vector3> tangents;
	std::vector<unsigned int> indices;

	for (int i = 0; i < 4; ++i) {
		normals.emplace_back(Vector3(0, 0, 1));
		tangents.emplace_back(Vector3(1, 0, 0));

		indices.emplace_back(i);
	}

	mesh->SetVertexNormals(normals);
	mesh->SetVertexTangents(tangents);
	mesh->SetVertexIndices(indices);

	mesh->UploadToGPU();
	return mesh;
}

PS4Mesh* PS4Mesh::GenerateSphere()
{
	PS4Mesh* mesh = new PS4Mesh("/app0/sphere2.msh");
	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriList;
	mesh->UploadToGPU();
	return mesh;
}

PS4Mesh* PS4Mesh::GenerateSinglePoint() {
	PS4Mesh* mesh = new PS4Mesh();

	mesh->indexType		= sce::Gnm::IndexSize::kIndexSize32;
	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypePointList;

	mesh->SetVertexPositions({ Vector3(0.0f, 0.0f, 0.0f) });
	mesh->SetVertexNormals({ Vector3(0, 0, 1) });
	mesh->SetVertexTangents({ Vector3(1, 0, 0) });
	mesh->SetVertexIndices({ 0 });

	mesh->UploadToGPU();
	return mesh;
}

PS4Mesh* PS4Mesh::GenerateTriangle() {
	
	PS4Mesh* mesh = new PS4Mesh();

	mesh->indexType		= sce::Gnm::IndexSize::kIndexSize32;
	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriList;

	mesh->SetVertexPositions({ Vector3(0.0f, 0.5f, 0.0f), Vector3(0.5f, -0.5f, 0.0f), Vector3(-0.5f, -0.5f, 0.0f) });
	mesh->SetVertexTextureCoords({ Vector2(0.5f, 0.0f) , Vector2(1.0f, 1.0f), Vector2(0.0f, 1.0f) });
	mesh->SetVertexNormals({ Vector3(0, 0, 1),Vector3(0, 0, 1), Vector3(0, 0, 1) });
	mesh->SetVertexTangents({ Vector3(1, 0, 0), Vector3(1, 0, 0), Vector3(1, 0, 0) });
	mesh->SetVertexIndices({ 0, 1, 2 });

	mesh->UploadToGPU();
	return mesh;
}

void	PS4Mesh::UploadToGPU() {
	vertexDataSize = GetVertexCount() * sizeof(MeshVertex);
	indexDataSize  = GetIndexCount() * sizeof(int);

	indexBuffer = static_cast<int*>			(garlicAllocator->allocate(indexDataSize, Gnm::kAlignmentOfBufferInBytes));
	vertexBuffer = static_cast<MeshVertex*>	(garlicAllocator->allocate(vertexDataSize, Gnm::kAlignmentOfBufferInBytes));

	Gnm::registerResource(nullptr, ownerHandle, indexBuffer , indexDataSize , "IndexData" , Gnm::kResourceTypeIndexBufferBaseAddress, 0);
	Gnm::registerResource(nullptr, ownerHandle, vertexBuffer, vertexDataSize, "VertexData", Gnm::kResourceTypeIndexBufferBaseAddress, 0);

	for (int i = 0; i < GetVertexCount(); ++i) {
		memcpy(&vertexBuffer[i].position,	  &positions[i], sizeof(float) * 3);
		memcpy(&vertexBuffer[i].textureCoord, &texCoords[i], sizeof(float) * 2);
		memcpy(&vertexBuffer[i].normal,		  &normals[i],   sizeof(float) * 3);
		memcpy(&vertexBuffer[i].tangent,	  &tangents[i],  sizeof(float) * 3);
	}

	for (int i = 0; i < GetIndexCount(); ++i) { //Our index buffer might not have the same data size as the source indices?
		indexBuffer[i] = indices[i];
	}	

	attributeCount		= 4;
	attributeBuffers	= new sce::Gnm::Buffer[4];

	InitAttributeBuffer(attributeBuffers[0], Gnm::kDataFormatR32G32B32Float, &(vertexBuffer[0].position));
	InitAttributeBuffer(attributeBuffers[1], Gnm::kDataFormatR32G32Float	, &(vertexBuffer[0].textureCoord));
	InitAttributeBuffer(attributeBuffers[2], Gnm::kDataFormatR32G32B32Float, &(vertexBuffer[0].normal));
	InitAttributeBuffer(attributeBuffers[3], Gnm::kDataFormatR32G32B32Float, &(vertexBuffer[0].tangent));
}

void	PS4Mesh::InitAttributeBuffer(sce::Gnm::Buffer &buffer, Gnm::DataFormat format, void*offset) {
	buffer.initAsVertexBuffer(offset, format, sizeof(MeshVertex), GetVertexCount());
	buffer.setResourceMemoryType(Gnm::kResourceMemoryTypeRO);
}

void PS4Mesh::SubmitDraw(Gnmx::GnmxGfxContext& cmdList, Gnm::ShaderStage stage) {
	cmdList.setVertexBuffers(stage, 0, attributeCount, attributeBuffers);
	cmdList.setPrimitiveType(primitiveType);
	cmdList.setIndexSize(indexType);
	cmdList.drawIndex(GetIndexCount(), indexBuffer);
} 


//
///*Mo test*/
//
///* x=0 √*/
//PS4Mesh * PS4Mesh::GenerateQuadLeft()
//{
//	PS4Mesh* mesh = new PS4Mesh();
//
//	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
//	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;
//
//	mesh->SetVertexPositions({ Vector3(0.0f, 0.0f, 1.0f),  Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 1.0f),Vector3(0.0f, 1.0f, 0.0f) });
//
//	//	mesh->SetVertexTextureCoords({ Vector2(0.0f,  1.0f),Vector2(0.0f, 0.0f) ,Vector2(1.0f, 1.0f), Vector2(1.0f,  0.0f) });//倒着的
//	mesh->SetVertexTextureCoords({ Vector2(1.0f, 0.0f),Vector2(1.0f, 1.0f) ,Vector2(0.0f, 0.0f), Vector2(0.0f, 1.0f) });
//	std::vector<Vector3> normals;
//	std::vector<Vector3> tangents;
//	std::vector<unsigned int> indices;
//
//	for (int i = 0; i < 4; ++i) {
//		normals.emplace_back(Vector3(1, 0, 0));
//		tangents.emplace_back(Vector3(0, 0, 1));
//
//		indices.emplace_back(i);
//	}
//
//	mesh->SetVertexNormals(normals);
//	mesh->SetVertexTangents(tangents);
//	mesh->SetVertexIndices(indices);
//
//	mesh->UploadToGPU();
//	return mesh;
//}
//
///* x = 1 √*/
//PS4Mesh * PS4Mesh::GenerateQuadRight()
//{
//	PS4Mesh* mesh = new PS4Mesh();
//
//	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
//	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;
//
//	mesh->SetVertexPositions({ Vector3(1.0f, 0.0f, 1.0f),  Vector3(1.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f),Vector3(1.0f, 1.0f, 0.0f) });
//
//	//	mesh->SetVertexTextureCoords({ Vector2(0.0f,  1.0f),Vector2(0.0f, 0.0f) ,Vector2(1.0f, 1.0f), Vector2(1.0f,  0.0f) });//倒着的
//	mesh->SetVertexTextureCoords({ Vector2(1.0f, 0.0f),Vector2(1.0f, 1.0f) ,Vector2(0.0f, 0.0f), Vector2(0.0f, 1.0f) });
//	std::vector<Vector3> normals;
//	std::vector<Vector3> tangents;
//	std::vector<unsigned int> indices;
//
//	for (int i = 0; i < 4; ++i) {
//		normals.emplace_back(Vector3(-1, 0, 0));
//		tangents.emplace_back(Vector3(0, 0, 1));
//
//		indices.emplace_back(i);
//	}
//
//	mesh->SetVertexNormals(normals);
//	mesh->SetVertexTangents(tangents);
//	mesh->SetVertexIndices(indices);
//
//	mesh->UploadToGPU();
//	return mesh;
//}
//
///*z=1 √*/
//PS4Mesh * PS4Mesh::GenerateQuadFront()
//{
//	PS4Mesh* mesh = new PS4Mesh();
//
//	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
//	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;
//
//	mesh->SetVertexPositions({ Vector3(0.0f, 1.0f, 1.0f),  Vector3(0.0f, 0.0f, 1.0f), Vector3(1.0f, 1.0f, 1.0f),Vector3(1.0f, 0.0f, 1.0f) });
//	mesh->SetVertexTextureCoords({ Vector2(1.0f,  0.0f),Vector2(1.0f, 1.0f) ,Vector2(0.0f, 0.0f), Vector2(0.0f,  1.0f) });
//	std::vector<Vector3> normals;
//	std::vector<Vector3> tangents;
//	std::vector<unsigned int> indices;
//
//	for (int i = 0; i < 4; ++i) {
//		normals.emplace_back(Vector3(0, 0, -1));
//		tangents.emplace_back(Vector3(1, 0, 0));
//
//		indices.emplace_back(i);
//	}
//
//	mesh->SetVertexNormals(normals);
//	mesh->SetVertexTangents(tangents);
//	mesh->SetVertexIndices(indices);
//
//	mesh->UploadToGPU();
//	return mesh;
//}
//
///* z=0 √ */
//PS4Mesh * PS4Mesh::GenerateQuadBack()
//{
//	PS4Mesh* mesh = new PS4Mesh();
//
//	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
//	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;
//
//	mesh->SetVertexPositions({ Vector3(0.0f, 1.0f, 0.0f),  Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 0.0f),Vector3(1.0f, 0.0f, 0.0f) });
//
//	mesh->SetVertexTextureCoords({ Vector2(1.0f,  0.0f),Vector2(1.0f, 1.0f) ,Vector2(0.0f, 0.0f), Vector2(0.0f,  1.0f) });
//	std::vector<Vector3> normals;
//	std::vector<Vector3> tangents;
//	std::vector<unsigned int> indices;
//
//	for (int i = 0; i < 4; ++i) {
//		normals.emplace_back(Vector3(0, 0, 1));
//		tangents.emplace_back(Vector3(1, 0, 0));
//
//		indices.emplace_back(i);
//	}
//
//	mesh->SetVertexNormals(normals);
//	mesh->SetVertexTangents(tangents);
//	mesh->SetVertexIndices(indices);
//
//	mesh->UploadToGPU();
//	return mesh;
//}
//
///* y=0 */
//PS4Mesh * PS4Mesh::GenerateQuadDown()
//{
//	PS4Mesh* mesh = new PS4Mesh();
//
//	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
//	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;
//
//	mesh->SetVertexPositions({ Vector3(0.0f, 0.0f, 1.0f),  Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 0.0f, 1.0f),Vector3(1.0f, 0.0f, 0.0f) });
//
//	mesh->SetVertexTextureCoords({ Vector2(1.0f,  0.0f),Vector2(1.0f, 1.0f) ,Vector2(0.0f, 0.0f), Vector2(0.0f,  1.0f) });
//	std::vector<Vector3> normals;
//	std::vector<Vector3> tangents;
//	std::vector<unsigned int> indices;
//
//	for (int i = 0; i < 4; ++i) {
//		normals.emplace_back(Vector3(0, 1, 0));
//		tangents.emplace_back(Vector3(1, 0, 0));
//
//		indices.emplace_back(i);
//	}
//
//	mesh->SetVertexNormals(normals);
//	mesh->SetVertexTangents(tangents);
//	mesh->SetVertexIndices(indices);
//
//	mesh->UploadToGPU();
//	return mesh;
//}
//
///*y=1*/
//PS4Mesh * PS4Mesh::GenerateQuadUp()
//{
//	PS4Mesh* mesh = new PS4Mesh();
//
//	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
//	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;
//
//	mesh->SetVertexPositions({ Vector3(0.0f, 1.0f, 1.0f),  Vector3(0.0f, 1.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f),Vector3(1.0f, 1.0f, 0.0f) });
//
//	mesh->SetVertexTextureCoords({ Vector2(1.0f,  0.0f),Vector2(1.0f, 1.0f) ,Vector2(0.0f, 0.0f), Vector2(0.0f,  1.0f) });
//	std::vector<Vector3> normals;
//	std::vector<Vector3> tangents;
//	std::vector<unsigned int> indices;
//
//	for (int i = 0; i < 4; ++i) {
//		normals.emplace_back(Vector3(0, -1, 0));
//		tangents.emplace_back(Vector3(1, 0, 0));
//
//		indices.emplace_back(i);
//	}
//
//	mesh->SetVertexNormals(normals);
//	mesh->SetVertexTangents(tangents);
//	mesh->SetVertexIndices(indices);
//
//	mesh->UploadToGPU();
//	return mesh;
//}
///*MO TEST*/

Vector3 min = Vector3(-0.5f, -0.5f, -0.5f);

///*Mo test*/
//
///* x=0 √*/
//PS4Mesh * PS4Mesh::GenerateQuadLeft(Vector3 cameraPos)
//{
//	PS4Mesh* mesh = new PS4Mesh();
//
//	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
//	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;
//	mesh->SetVertexPositions({ Vector3(0.0f, 0.0f, 1.0f)+min+ cameraPos,  Vector3(0.0f, 0.0f, 0.0f) + min + cameraPos, Vector3(0.0f, 1.0f, 1.0f) + min + cameraPos,Vector3(0.0f, 1.0f, 0.0f) + min + cameraPos });
////	mesh->SetVertexPositions({ Vector3(-0.5f, -0.5f, 0.5f),  Vector3(-0.5f, -0.5f, -0.5f), Vector3(-0.5f, 0.5f, 0.5f),Vector3(-0.5f, 0.5f, -0.5f) });
//
//	//	mesh->SetVertexTextureCoords({ Vector2(0.0f,  1.0f),Vector2(0.0f, 0.0f) ,Vector2(1.0f, 1.0f), Vector2(1.0f,  0.0f) });//倒着的
//	mesh->SetVertexTextureCoords({ Vector2(1.0f, 0.0f),Vector2(1.0f, 1.0f) ,Vector2(0.0f, 0.0f), Vector2(0.0f, 1.0f) });
//	std::vector<Vector3> normals;
//	std::vector<Vector3> tangents;
//	std::vector<unsigned int> indices;
//
//	for (int i = 0; i < 4; ++i) {
//		normals.emplace_back(Vector3(1, 0, 0));
//		tangents.emplace_back(Vector3(0, 0, 1));
//
//		indices.emplace_back(i);
//	}
//
//	mesh->SetVertexNormals(normals);
//	mesh->SetVertexTangents(tangents);
//	mesh->SetVertexIndices(indices);
//
//	mesh->UploadToGPU();
//	return mesh;
//}
//
///* x = 1 √*/
//PS4Mesh * PS4Mesh::GenerateQuadRight(Vector3 cameraPos)
//{
//	PS4Mesh* mesh = new PS4Mesh();
//
//	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
//	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;
//
//	mesh->SetVertexPositions({ Vector3(1.0f, 0.0f, 1.0f)+min + cameraPos,  Vector3(1.0f, 0.0f, 0.0f) + min + cameraPos, Vector3(1.0f, 1.0f, 1.0f) + min + cameraPos,Vector3(1.0f, 1.0f, 0.0f) + min + cameraPos });
//
//	//	mesh->SetVertexTextureCoords({ Vector2(0.0f,  1.0f),Vector2(0.0f, 0.0f) ,Vector2(1.0f, 1.0f), Vector2(1.0f,  0.0f) });//倒着的
//	mesh->SetVertexTextureCoords({ Vector2(1.0f, 0.0f),Vector2(1.0f, 1.0f) ,Vector2(0.0f, 0.0f), Vector2(0.0f, 1.0f) });
//	std::vector<Vector3> normals;
//	std::vector<Vector3> tangents;
//	std::vector<unsigned int> indices;
//
//	for (int i = 0; i < 4; ++i) {
//		normals.emplace_back(Vector3(-1, 0, 0));
//		tangents.emplace_back(Vector3(0, 0, 1));
//
//		indices.emplace_back(i);
//	}
//
//	mesh->SetVertexNormals(normals);
//	mesh->SetVertexTangents(tangents);
//	mesh->SetVertexIndices(indices);
//
//	mesh->UploadToGPU();
//	return mesh;
//}
//
///*z=1 √*/
//PS4Mesh * PS4Mesh::GenerateQuadFront(Vector3 cameraPos)
//{
//	PS4Mesh* mesh = new PS4Mesh();
//
//	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
//	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;
//
//	mesh->SetVertexPositions({ Vector3(0.0f, 1.0f, 1.0f)+ min + cameraPos,  Vector3(0.0f, 0.0f, 1.0f)+ min + cameraPos, Vector3(1.0f, 1.0f, 1.0f) + min + cameraPos,Vector3(1.0f, 0.0f, 1.0f) + min + cameraPos });
//	mesh->SetVertexTextureCoords({ Vector2(1.0f,  0.0f),Vector2(1.0f, 1.0f) ,Vector2(0.0f, 0.0f), Vector2(0.0f,  1.0f) });
//	std::vector<Vector3> normals;
//	std::vector<Vector3> tangents;
//	std::vector<unsigned int> indices;
//
//	for (int i = 0; i < 4; ++i) {
//		normals.emplace_back(Vector3(0, 0, -1));
//		tangents.emplace_back(Vector3(1, 0, 0));
//
//		indices.emplace_back(i);
//	}
//
//	mesh->SetVertexNormals(normals);
//	mesh->SetVertexTangents(tangents);
//	mesh->SetVertexIndices(indices);
//
//	mesh->UploadToGPU();
//	return mesh;
//}
//
///* z=0 √ */
//PS4Mesh * PS4Mesh::GenerateQuadBack(Vector3 cameraPos)
//{
//	PS4Mesh* mesh = new PS4Mesh();
//
//	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
//	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;
//
//	mesh->SetVertexPositions({ Vector3(0.0f, 1.0f, 0.0f) + min + cameraPos,  Vector3(0.0f, 0.0f, 0.0f) + min + cameraPos, Vector3(1.0f, 1.0f, 0.0f) + min + cameraPos,Vector3(1.0f, 0.0f, 0.0f) + min + cameraPos });
//
//	mesh->SetVertexTextureCoords({ Vector2(1.0f,  0.0f),Vector2(1.0f, 1.0f) ,Vector2(0.0f, 0.0f), Vector2(0.0f,  1.0f) });
//	std::vector<Vector3> normals;
//	std::vector<Vector3> tangents;
//	std::vector<unsigned int> indices;
//
//	for (int i = 0; i < 4; ++i) {
//		normals.emplace_back(Vector3(0, 0, 1));
//		tangents.emplace_back(Vector3(1, 0, 0));
//
//		indices.emplace_back(i);
//	}
//
//	mesh->SetVertexNormals(normals);
//	mesh->SetVertexTangents(tangents);
//	mesh->SetVertexIndices(indices);
//
//	mesh->UploadToGPU();
//	return mesh;
//}
//
///* y=0 */
//PS4Mesh * PS4Mesh::GenerateQuadDown(Vector3 cameraPos)
//{
//	PS4Mesh* mesh = new PS4Mesh();
//
//	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
//	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;
//
//	mesh->SetVertexPositions({ Vector3(0.0f, 0.0f, 1.0f) + min + cameraPos,  Vector3(0.0f, 0.0f, 0.0f) + min + cameraPos , Vector3(1.0f, 0.0f, 1.0f) + min + cameraPos ,Vector3(1.0f, 0.0f, 0.0f) + min + cameraPos });
//	std::cout << "Mesh camera Position down" << cameraPos << std::endl;
//	mesh->SetVertexTextureCoords({ Vector2(1.0f,  0.0f),Vector2(1.0f, 1.0f) ,Vector2(0.0f, 0.0f), Vector2(0.0f,  1.0f) });
//	std::vector<Vector3> normals;
//	std::vector<Vector3> tangents;
//	std::vector<unsigned int> indices;
//
//	for (int i = 0; i < 4; ++i) {
//		normals.emplace_back(Vector3(0, 1, 0));
//		tangents.emplace_back(Vector3(1, 0, 0));
//
//		indices.emplace_back(i);
//	}
//
//	mesh->SetVertexNormals(normals);
//	mesh->SetVertexTangents(tangents);
//	mesh->SetVertexIndices(indices);
//
//	mesh->UploadToGPU();
//	return mesh;
//}
//
///*y=1*/
//int w = 0;
//PS4Mesh * PS4Mesh::GenerateQuadUp(Vector3 cameraPos)
//{
//	w++;
//	std::cout << "******************************" << std::endl;
//	std::cout << "w" << w << std::endl;
//	std::cout << "******************************" << std::endl;
//	PS4Mesh* mesh = new PS4Mesh();
//
//	std::cout << "Mesh cameraPos up" << cameraPos << std::endl;
//	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
//	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;
//
//	mesh->SetVertexPositions({ Vector3(0.0f, 1.0f, 1.0f) + min + cameraPos ,  Vector3(0.0f, 1.0f, 0.0f) + min + cameraPos, Vector3(1.0f, 1.0f, 1.0f) + min + cameraPos,Vector3(1.0f, 1.0f, 0.0f) + min + cameraPos });
//
//	mesh->SetVertexTextureCoords({ Vector2(1.0f,  0.0f),Vector2(1.0f, 1.0f) ,Vector2(0.0f, 0.0f), Vector2(0.0f,  1.0f) });
//	std::vector<Vector3> normals;
//	std::vector<Vector3> tangents;
//	std::vector<unsigned int> indices;
//
//	for (int i = 0; i < 4; ++i) {
//		normals.emplace_back(Vector3(0, -1, 0));
//		tangents.emplace_back(Vector3(1, 0, 0));
//
//		indices.emplace_back(i);
//	}
//
//	mesh->SetVertexNormals(normals);
//	mesh->SetVertexTangents(tangents);
//	mesh->SetVertexIndices(indices);
//
//	mesh->UploadToGPU();
//	return mesh;
//}
/////*MO TEST*/
//
///*Mo test*/
//
///* x=0 √*/
//PS4Mesh * PS4Mesh::GenerateQuadLeft(Vector3 cameraPos)
//{
//	PS4Mesh* mesh = new PS4Mesh();
//
//	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
//	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;
//	mesh->SetVertexPositions({ Vector3(0.0f, 0.0f, 1.0f) + min ,  Vector3(0.0f, 0.0f, 0.0f) + min , Vector3(0.0f, 1.0f, 1.0f) + min ,Vector3(0.0f, 1.0f, 0.0f) + min});
//	//	mesh->SetVertexPositions({ Vector3(-0.5f, -0.5f, 0.5f),  Vector3(-0.5f, -0.5f, -0.5f), Vector3(-0.5f, 0.5f, 0.5f),Vector3(-0.5f, 0.5f, -0.5f) });
//
//		//	mesh->SetVertexTextureCoords({ Vector2(0.0f,  1.0f),Vector2(0.0f, 0.0f) ,Vector2(1.0f, 1.0f), Vector2(1.0f,  0.0f) });//倒着的
//	mesh->SetVertexTextureCoords({ Vector2(1.0f, 0.0f),Vector2(1.0f, 1.0f) ,Vector2(0.0f, 0.0f), Vector2(0.0f, 1.0f) });
//	std::vector<Vector3> normals;
//	std::vector<Vector3> tangents;
//	std::vector<unsigned int> indices;
//
//	for (int i = 0; i < 4; ++i) {
//		normals.emplace_back(Vector3(1, 0, 0));
//		tangents.emplace_back(Vector3(0, 0, 1));
//
//		indices.emplace_back(i);
//	}
//
//	mesh->SetVertexNormals(normals);
//	mesh->SetVertexTangents(tangents);
//	mesh->SetVertexIndices(indices);
//
//	mesh->UploadToGPU();
//	return mesh;
//}
//
///* x = 1 √*/
//PS4Mesh * PS4Mesh::GenerateQuadRight(Vector3 cameraPos)
//{
//	PS4Mesh* mesh = new PS4Mesh();
//
//	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
//	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;
//
//	mesh->SetVertexPositions({ Vector3(1.0f, 0.0f, 1.0f) + min ,  Vector3(1.0f, 0.0f, 0.0f) + min , Vector3(1.0f, 1.0f, 1.0f) + min ,Vector3(1.0f, 1.0f, 0.0f) + min  });
//
//	//	mesh->SetVertexTextureCoords({ Vector2(0.0f,  1.0f),Vector2(0.0f, 0.0f) ,Vector2(1.0f, 1.0f), Vector2(1.0f,  0.0f) });//倒着的
//	mesh->SetVertexTextureCoords({ Vector2(1.0f, 0.0f),Vector2(1.0f, 1.0f) ,Vector2(0.0f, 0.0f), Vector2(0.0f, 1.0f) });
//	std::vector<Vector3> normals;
//	std::vector<Vector3> tangents;
//	std::vector<unsigned int> indices;
//
//	for (int i = 0; i < 4; ++i) {
//		normals.emplace_back(Vector3(-1, 0, 0));
//		tangents.emplace_back(Vector3(0, 0, 1));
//
//		indices.emplace_back(i);
//	}
//
//	mesh->SetVertexNormals(normals);
//	mesh->SetVertexTangents(tangents);
//	mesh->SetVertexIndices(indices);
//
//	mesh->UploadToGPU();
//	return mesh;
//}
//
///*z=1 √*/
//PS4Mesh * PS4Mesh::GenerateQuadFront(Vector3 cameraPos)
//{
//	PS4Mesh* mesh = new PS4Mesh();
//
//	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
//	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;
//
//	mesh->SetVertexPositions({ Vector3(0.0f, 1.0f, 1.0f) + min ,  Vector3(0.0f, 0.0f, 1.0f) + min , Vector3(1.0f, 1.0f, 1.0f) + min ,Vector3(1.0f, 0.0f, 1.0f) + min  });
//	mesh->SetVertexTextureCoords({ Vector2(1.0f,  0.0f),Vector2(1.0f, 1.0f) ,Vector2(0.0f, 0.0f), Vector2(0.0f,  1.0f) });
//	std::vector<Vector3> normals;
//	std::vector<Vector3> tangents;
//	std::vector<unsigned int> indices;
//
//	for (int i = 0; i < 4; ++i) {
//		normals.emplace_back(Vector3(0, 0, -1));
//		tangents.emplace_back(Vector3(1, 0, 0));
//
//		indices.emplace_back(i);
//	}
//
//	mesh->SetVertexNormals(normals);
//	mesh->SetVertexTangents(tangents);
//	mesh->SetVertexIndices(indices);
//
//	mesh->UploadToGPU();
//	return mesh;
//}
//
///* z=0 √ */
//PS4Mesh * PS4Mesh::GenerateQuadBack(Vector3 cameraPos)
//{
//	PS4Mesh* mesh = new PS4Mesh();
//
//	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
//	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;
//
//	mesh->SetVertexPositions({ Vector3(0.0f, 1.0f, 0.0f) + min ,  Vector3(0.0f, 0.0f, 0.0f) + min , Vector3(1.0f, 1.0f, 0.0f) + min ,Vector3(1.0f, 0.0f, 0.0f) + min  });
//
//	mesh->SetVertexTextureCoords({ Vector2(1.0f,  0.0f),Vector2(1.0f, 1.0f) ,Vector2(0.0f, 0.0f), Vector2(0.0f,  1.0f) });
//	std::vector<Vector3> normals;
//	std::vector<Vector3> tangents;
//	std::vector<unsigned int> indices;
//
//	for (int i = 0; i < 4; ++i) {
//		normals.emplace_back(Vector3(0, 0, 1));
//		tangents.emplace_back(Vector3(1, 0, 0));
//
//		indices.emplace_back(i);
//	}
//
//	mesh->SetVertexNormals(normals);
//	mesh->SetVertexTangents(tangents);
//	mesh->SetVertexIndices(indices);
//
//	mesh->UploadToGPU();
//	return mesh;
//}
//
///* y=0 */
//PS4Mesh * PS4Mesh::GenerateQuadDown(Vector3 cameraPos)
//{
//	PS4Mesh* mesh = new PS4Mesh();
//
//	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
//	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;
//
//	mesh->SetVertexPositions({ Vector3(0.0f, 0.0f, 1.0f) + min,  Vector3(0.0f, 0.0f, 0.0f) + min  , Vector3(1.0f, 0.0f, 1.0f) + min  ,Vector3(1.0f, 0.0f, 0.0f) + min  });
//
//	mesh->SetVertexTextureCoords({ Vector2(1.0f,  0.0f),Vector2(1.0f, 1.0f) ,Vector2(0.0f, 0.0f), Vector2(0.0f,  1.0f) });
//	std::vector<Vector3> normals;
//	std::vector<Vector3> tangents;
//	std::vector<unsigned int> indices;
//
//	for (int i = 0; i < 4; ++i) {
//		normals.emplace_back(Vector3(0, 1, 0));
//		tangents.emplace_back(Vector3(1, 0, 0));
//
//		indices.emplace_back(i);
//	}
//
//	mesh->SetVertexNormals(normals);
//	mesh->SetVertexTangents(tangents);
//	mesh->SetVertexIndices(indices);
//
//	mesh->UploadToGPU();
//	return mesh;
//}
//
///*y=1*/
//int w = 0;
//PS4Mesh * PS4Mesh::GenerateQuadUp(Vector3 cameraPos)
//{
//	w++;
//	std::cout << "******************************" << std::endl;
//	std::cout << "w" << w << std::endl;
//	std::cout << "******************************" << std::endl;
//	PS4Mesh* mesh = new PS4Mesh();
//
//	std::cout << "Mesh cameraPos up" << cameraPos << std::endl;
//	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
//	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;
//
//	mesh->SetVertexPositions({ Vector3(0.0f, 1.0f, 1.0f) + min  ,  Vector3(0.0f, 1.0f, 0.0f) + min , Vector3(1.0f, 1.0f, 1.0f) + min ,Vector3(1.0f, 1.0f, 0.0f) + min  });
//
//	mesh->SetVertexTextureCoords({ Vector2(1.0f,  0.0f),Vector2(1.0f, 1.0f) ,Vector2(0.0f, 0.0f), Vector2(0.0f,  1.0f) });
//	std::vector<Vector3> normals;
//	std::vector<Vector3> tangents;
//	std::vector<unsigned int> indices;
//
//	for (int i = 0; i < 4; ++i) {
//		normals.emplace_back(Vector3(0, -1, 0));
//		tangents.emplace_back(Vector3(1, 0, 0));
//
//		indices.emplace_back(i);
//	}
//
//	mesh->SetVertexNormals(normals);
//	mesh->SetVertexTangents(tangents);
//	mesh->SetVertexIndices(indices);
//
//	mesh->UploadToGPU();
//	return mesh;
//}
///*MO TEST*/




///*MO fplane TEST*/

/*Mo test*/
Vector3 t = Vector3(-0.01, -0.01 ,- 0.01);

const unsigned int MAP_WIDTH = 1024;
const unsigned int CELL_WIDTH = 16;
const unsigned int MAP = MAP_WIDTH * CELL_WIDTH / 2;

float width = MAP * 1 / 8;
float height = MAP * 1 / 8;
float length = MAP * 1 / 8;
/* x=0 √*/
PS4Mesh * PS4Mesh::GenerateQuadLeft()
{
	PS4Mesh* mesh = new PS4Mesh();

	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;
	mesh->SetVertexPositions({ (Vector3(0.0f, 0.0f, 1.0f) + min) * 2000 ,  (Vector3(0.0f, 0.0f, 0.0f) + min) * 2000 , (Vector3(0.0f, 1.0f, 1.0f) + min) * 2000,(Vector3(0.0f, 1.0f, 0.0f) + min) * 2000 });
	//	mesh->SetVertexPositions({ Vector3(-0.5f, -0.5f, 0.5f),  Vector3(-0.5f, -0.5f, -0.5f), Vector3(-0.5f, 0.5f, 0.5f),Vector3(-0.5f, 0.5f, -0.5f) });

		//	mesh->SetVertexTextureCoords({ Vector2(0.0f,  1.0f),Vector2(0.0f, 0.0f) ,Vector2(1.0f, 1.0f), Vector2(1.0f,  0.0f) });//倒着的
	mesh->SetVertexTextureCoords({Vector2(0.0f, 1.0f) ,Vector2(1.0f, 1.0f) ,Vector2(0.0f, 0.0f), Vector2(1.0f, 0.0f) });
	std::vector<Vector3> normals;
	std::vector<Vector3> tangents;
	std::vector<unsigned int> indices;

	for (int i = 0; i < 4; ++i) {
		normals.emplace_back(Vector3(1, 0, 0));
		tangents.emplace_back(Vector3(0, 0, 1));

		indices.emplace_back(i);
	}

	mesh->SetVertexNormals(normals);
	mesh->SetVertexTangents(tangents);
	mesh->SetVertexIndices(indices);

	mesh->UploadToGPU();
	return mesh;
}

/* x = 1 √*/
PS4Mesh * PS4Mesh::GenerateQuadRight()
{
	PS4Mesh* mesh = new PS4Mesh();

	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;

	mesh->SetVertexPositions({ (Vector3(1.0f, 0.0f, 1.0f) + min) * 2000 , (Vector3(1.0f, 0.0f, 0.0f) + min) * 2000 , (Vector3(1.0f, 1.0f, 1.0f) + min) * 2000 ,(Vector3(1.0f, 1.0f, 0.0f) + min) * 2000 });

	mesh->SetVertexTextureCoords({ Vector2(1.0f, 1.0f),Vector2(0.0f,  1.0f),Vector2(1.0f,  0.0f), Vector2(0.0f, 0.0f) });
	std::vector<Vector3> normals;
	std::vector<Vector3> tangents;
	std::vector<unsigned int> indices;

	for (int i = 0; i < 4; ++i) {
		normals.emplace_back(Vector3(-1, 0, 0));
		tangents.emplace_back(Vector3(0, 0, 1));

		indices.emplace_back(i);
	}

	mesh->SetVertexNormals(normals);
	mesh->SetVertexTangents(tangents);
	mesh->SetVertexIndices(indices);

	mesh->UploadToGPU();
	return mesh;
}

/*z=1 √*/
PS4Mesh * PS4Mesh::GenerateQuadFront()
{
	PS4Mesh* mesh = new PS4Mesh();

	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;

	mesh->SetVertexPositions({ (Vector3(0.0f, 1.0f, 1.0f) + min) * 2000 ,  (Vector3(0.0f, 0.0f, 1.0f) + min) * 2000 , (Vector3(1.0f, 1.0f, 1.0f) + min) * 2000 ,(Vector3(1.0f, 0.0f, 1.0f) + min) * 2000 });
	mesh->SetVertexTextureCoords({ Vector2(1.0f,  0.0f),Vector2(1.0f, 1.0f) ,Vector2(0.0f, 0.0f), Vector2(0.0f,  1.0f) });
	std::vector<Vector3> normals;
	std::vector<Vector3> tangents;
	std::vector<unsigned int> indices;

	for (int i = 0; i < 4; ++i) {
		normals.emplace_back(Vector3(0, 0, -1));
		tangents.emplace_back(Vector3(1, 0, 0));

		indices.emplace_back(i);
	}

	mesh->SetVertexNormals(normals);
	mesh->SetVertexTangents(tangents);
	mesh->SetVertexIndices(indices);

	mesh->UploadToGPU();
	return mesh;
}

/* z=0 √ */
PS4Mesh * PS4Mesh::GenerateQuadBack()
{
	PS4Mesh* mesh = new PS4Mesh();

	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;

	mesh->SetVertexPositions({ (Vector3(0.0f, 1.0f, 0.0f) + min) * 2000 ,  (Vector3(0.0f, 0.0f, 0.0f) + min) * 2000 ,(Vector3(1.0f, 1.0f, 0.0f) + min) * 2000 ,(Vector3(1.0f, 0.0f, 0.0f) + min) * 2000 });

	mesh->SetVertexTextureCoords({ Vector2(0.0f, 0.0f),Vector2(0.0f,  1.0f), Vector2(1.0f,  0.0f) ,Vector2(1.0f, 1.0f) });
	std::vector<Vector3> normals;
	std::vector<Vector3> tangents;
	std::vector<unsigned int> indices;

	for (int i = 0; i < 4; ++i) {
		normals.emplace_back(Vector3(0, 0, 1));
		tangents.emplace_back(Vector3(1, 0, 0));

		indices.emplace_back(i);
	}

	mesh->SetVertexNormals(normals);
	mesh->SetVertexTangents(tangents);
	mesh->SetVertexIndices(indices);

	mesh->UploadToGPU();
	return mesh;
}

/* y=0 */
PS4Mesh * PS4Mesh::GenerateQuadDown()
{
	PS4Mesh* mesh = new PS4Mesh();

	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;

	mesh->SetVertexPositions({ (Vector3(0.0f, 0.0f, 1.0f) + min) * 2000, (Vector3(0.0f, 0.0f, 0.0f) + min) * 2000 , (Vector3(1.0f, 0.0f, 1.0f) + min) * 2000  ,(Vector3(1.0f, 0.0f, 0.0f) + min) * 2000 });

	mesh->SetVertexTextureCoords({ Vector2(1.0f, 1.0f),Vector2(0.0f,  1.0f),Vector2(1.0f,  0.0f), Vector2(0.0f, 0.0f) });
	std::vector<Vector3> normals;
	std::vector<Vector3> tangents;
	std::vector<unsigned int> indices;

	for (int i = 0; i < 4; ++i) {
		normals.emplace_back(Vector3(0, 1, 0));
		tangents.emplace_back(Vector3(1, 0, 0));

		indices.emplace_back(i);
	}

	mesh->SetVertexNormals(normals);
	mesh->SetVertexTangents(tangents);
	mesh->SetVertexIndices(indices);

	mesh->UploadToGPU();
	return mesh;
}

/*y=1*/
int w = 0;
PS4Mesh * PS4Mesh::GenerateQuadUp()
{
	w++;
	std::cout << "******************************" << std::endl;
	std::cout << "w" << w << std::endl;
	std::cout << "******************************" << std::endl;
	PS4Mesh* mesh = new PS4Mesh();

	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;

	mesh->SetVertexPositions({ (Vector3(0.0f, 1.0f, 1.0f) + min) * 2000  ,  (Vector3(0.0f, 1.0f, 0.0f) + min) * 2000,(Vector3(1.0f, 1.0f, 1.0f) + min) * 2000 ,(Vector3(1.0f, 1.0f, 0.0f) + min) * 2000 });

	mesh->SetVertexTextureCoords({ Vector2(1.0f,  0.0f),Vector2(1.0f, 1.0f) ,Vector2(0.0f, 0.0f), Vector2(0.0f,  1.0f) });
	std::vector<Vector3> normals;
	std::vector<Vector3> tangents;
	std::vector<unsigned int> indices;

	for (int i = 0; i < 4; ++i) {
		normals.emplace_back(Vector3(0, -1, 0));
		tangents.emplace_back(Vector3(1, 0, 0));

		indices.emplace_back(i);
	}

	mesh->SetVertexNormals(normals);
	mesh->SetVertexTangents(tangents);
	mesh->SetVertexIndices(indices);

	mesh->UploadToGPU();
	return mesh;
}
PS4Mesh * PS4Mesh::GenerateFloor()
{
		PS4Mesh* mesh = new PS4Mesh();

	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;

	mesh->SetVertexPositions({ (Vector3(0.0f, 0.0f, 1.0f) + min)*200, (Vector3(0.0f, 0.0f, 0.0f) + min)*200 , (Vector3(1.0f, 0.0f, 1.0f) + min)*200  ,(Vector3(1.0f, 0.0f, 0.0f) + min)*200 });

	mesh->SetVertexTextureCoords({ Vector2(1.0f,  0.0f),Vector2(1.0f, 1.0f) ,Vector2(0.0f, 0.0f), Vector2(0.0f,  1.0f) });
	std::vector<Vector3> normals;
	std::vector<Vector3> tangents;
	std::vector<unsigned int> indices;

	for (int i = 0; i < 4; ++i) {
		normals.emplace_back(Vector3(0, 1, 0));
		tangents.emplace_back(Vector3(1, 0, 0));

		indices.emplace_back(i);
	}

	mesh->SetVertexNormals(normals);
	mesh->SetVertexTangents(tangents);
	mesh->SetVertexIndices(indices);

	mesh->UploadToGPU();
	return mesh;
}
/*MO TEST*/




///* x=0 √*/
//PS4Mesh * PS4Mesh::GenerateQuadLeft(Vector3 cameraPos)
//{
//	PS4Mesh* mesh = new PS4Mesh();
//
//	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
//	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;
//	mesh->SetVertexPositions({ (Vector3(0.0f, 0.0f, 1.0f) + min)*2000 ,  (Vector3(0.0f, 0.0f, 0.0f) + min)*2000 , (Vector3(0.0f, 1.0f, 1.0f) + min)*2000,(Vector3(0.0f, 1.0f, 0.0f) + min)*2000});
//	//	mesh->SetVertexPositions({ Vector3(-0.5f, -0.5f, 0.5f),  Vector3(-0.5f, -0.5f, -0.5f), Vector3(-0.5f, 0.5f, 0.5f),Vector3(-0.5f, 0.5f, -0.5f) });
//
//		//	mesh->SetVertexTextureCoords({ Vector2(0.0f,  1.0f),Vector2(0.0f, 0.0f) ,Vector2(1.0f, 1.0f), Vector2(1.0f,  0.0f) });//倒着的
//	mesh->SetVertexTextureCoords({ Vector2(1.0f, 0.0f),Vector2(1.0f, 1.0f) ,Vector2(0.0f, 0.0f), Vector2(0.0f, 1.0f) });
//	std::vector<Vector3> normals;
//	std::vector<Vector3> tangents;
//	std::vector<unsigned int> indices;
//
//	for (int i = 0; i < 4; ++i) {
//		normals.emplace_back(Vector3(1, 0, 0));
//		tangents.emplace_back(Vector3(0, 0, 1));
//
//		indices.emplace_back(i);
//	}
//
//	mesh->SetVertexNormals(normals);
//	mesh->SetVertexTangents(tangents);
//	mesh->SetVertexIndices(indices);
//
//	mesh->UploadToGPU();
//	return mesh;
//}
//
///* x = 1 √*/
//PS4Mesh * PS4Mesh::GenerateQuadRight(Vector3 cameraPos)
//{
//	PS4Mesh* mesh = new PS4Mesh();
//
//	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
//	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;
//
//	mesh->SetVertexPositions({ (Vector3(1.0f, 0.0f, 1.0f) + min)*2000 , (Vector3(1.0f, 0.0f, 0.0f) + min)*2000 , (Vector3(1.0f, 1.0f, 1.0f) + min)*2000 ,(Vector3(1.0f, 1.0f, 0.0f) + min)*2000 });
//
//	//	mesh->SetVertexTextureCoords({ Vector2(0.0f,  1.0f),Vector2(0.0f, 0.0f) ,Vector2(1.0f, 1.0f), Vector2(1.0f,  0.0f) });//倒着的
//	mesh->SetVertexTextureCoords({ Vector2(1.0f, 0.0f),Vector2(1.0f, 1.0f) ,Vector2(0.0f, 0.0f), Vector2(0.0f, 1.0f) });
//	std::vector<Vector3> normals;
//	std::vector<Vector3> tangents;
//	std::vector<unsigned int> indices;
//
//	for (int i = 0; i < 4; ++i) {
//		normals.emplace_back(Vector3(-1, 0, 0));
//		tangents.emplace_back(Vector3(0, 0, 1));
//
//		indices.emplace_back(i);
//	}
//
//	mesh->SetVertexNormals(normals);
//	mesh->SetVertexTangents(tangents);
//	mesh->SetVertexIndices(indices);
//
//	mesh->UploadToGPU();
//	return mesh;
//}
//
///*z=1 √*/
//PS4Mesh * PS4Mesh::GenerateQuadFront(Vector3 cameraPos)
//{
//	PS4Mesh* mesh = new PS4Mesh();
//
//	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
//	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;
//
//	mesh->SetVertexPositions({ (Vector3(0.0f, 1.0f, 1.0f) + min)*2000 ,  (Vector3(0.0f, 0.0f, 1.0f) + min)*2000 , (Vector3(1.0f, 1.0f, 1.0f) + min)*2000 ,(Vector3(1.0f, 0.0f, 1.0f) + min)*2000 });
//	mesh->SetVertexTextureCoords({ Vector2(1.0f,  0.0f),Vector2(1.0f, 1.0f) ,Vector2(0.0f, 0.0f), Vector2(0.0f,  1.0f) });
//	std::vector<Vector3> normals;
//	std::vector<Vector3> tangents;
//	std::vector<unsigned int> indices;
//
//	for (int i = 0; i < 4; ++i) {
//		normals.emplace_back(Vector3(0, 0, -1));
//		tangents.emplace_back(Vector3(1, 0, 0));
//
//		indices.emplace_back(i);
//	}
//
//	mesh->SetVertexNormals(normals);
//	mesh->SetVertexTangents(tangents);
//	mesh->SetVertexIndices(indices);
//
//	mesh->UploadToGPU();
//	return mesh;
//}
//
///* z=0 √ */
//PS4Mesh * PS4Mesh::GenerateQuadBack(Vector3 cameraPos)
//{
//	PS4Mesh* mesh = new PS4Mesh();
//
//	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
//	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;
//
//	mesh->SetVertexPositions({(Vector3(0.0f, 1.0f, 0.0f) + min)*2000 ,  (Vector3(0.0f, 0.0f, 0.0f) + min)*2000 ,(Vector3(1.0f, 1.0f, 0.0f) + min)*2000 ,(Vector3(1.0f, 0.0f, 0.0f) + min)*2000 });
//
//	mesh->SetVertexTextureCoords({ Vector2(1.0f,  0.0f),Vector2(1.0f, 1.0f) ,Vector2(0.0f, 0.0f), Vector2(0.0f,  1.0f) });
//	std::vector<Vector3> normals;
//	std::vector<Vector3> tangents;
//	std::vector<unsigned int> indices;
//
//	for (int i = 0; i < 4; ++i) {
//		normals.emplace_back(Vector3(0, 0, 1));
//		tangents.emplace_back(Vector3(1, 0, 0));
//
//		indices.emplace_back(i);
//	}
//
//	mesh->SetVertexNormals(normals);
//	mesh->SetVertexTangents(tangents);
//	mesh->SetVertexIndices(indices);
//
//	mesh->UploadToGPU();
//	return mesh;
//}
//
///* y=0 */
//PS4Mesh * PS4Mesh::GenerateQuadDown(Vector3 cameraPos)
//{
//	PS4Mesh* mesh = new PS4Mesh();
//
//	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
//	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;
//
//	mesh->SetVertexPositions({ (Vector3(0.0f, 0.0f, 1.0f) + min)*2000, (Vector3(0.0f, 0.0f, 0.0f) + min)*2000 , (Vector3(1.0f, 0.0f, 1.0f) + min)*2000  ,(Vector3(1.0f, 0.0f, 0.0f) + min)*2000 });
//
//	mesh->SetVertexTextureCoords({ Vector2(1.0f,  0.0f),Vector2(1.0f, 1.0f) ,Vector2(0.0f, 0.0f), Vector2(0.0f,  1.0f) });
//	std::vector<Vector3> normals;
//	std::vector<Vector3> tangents;
//	std::vector<unsigned int> indices;
//
//	for (int i = 0; i < 4; ++i) {
//		normals.emplace_back(Vector3(0, 1, 0));
//		tangents.emplace_back(Vector3(1, 0, 0));
//
//		indices.emplace_back(i);
//	}
//
//	mesh->SetVertexNormals(normals);
//	mesh->SetVertexTangents(tangents);
//	mesh->SetVertexIndices(indices);
//
//	mesh->UploadToGPU();
//	return mesh;
//}
//
///*y=1*/
//int w = 0;
//PS4Mesh * PS4Mesh::GenerateQuadUp(Vector3 cameraPos)
//{
//	w++;
//	std::cout << "******************************" << std::endl;
//	std::cout << "w" << w << std::endl;
//	std::cout << "******************************" << std::endl;
//	PS4Mesh* mesh = new PS4Mesh();
//
//	std::cout << "Mesh cameraPos up" << cameraPos << std::endl;
//	mesh->indexType = sce::Gnm::IndexSize::kIndexSize32;
//	mesh->primitiveType = sce::Gnm::PrimitiveType::kPrimitiveTypeTriStrip;
//
//	mesh->SetVertexPositions({ (Vector3(0.0f, 1.0f, 1.0f) + min)*2000  ,  (Vector3(0.0f, 1.0f, 0.0f) + min)*2000,(Vector3(1.0f, 1.0f, 1.0f) + min)*2000 ,(Vector3(1.0f, 1.0f, 0.0f) + min)*2000});
//
//	mesh->SetVertexTextureCoords({ Vector2(1.0f,  0.0f),Vector2(1.0f, 1.0f) ,Vector2(0.0f, 0.0f), Vector2(0.0f,  1.0f) });
//	std::vector<Vector3> normals;
//	std::vector<Vector3> tangents;
//	std::vector<unsigned int> indices;
//
//	for (int i = 0; i < 4; ++i) {
//		normals.emplace_back(Vector3(0, -1, 0));
//		tangents.emplace_back(Vector3(1, 0, 0));
//
//		indices.emplace_back(i);
//	}
//
//	mesh->SetVertexNormals(normals);
//	mesh->SetVertexTangents(tangents);
//	mesh->SetVertexIndices(indices);
//
//	mesh->UploadToGPU();
//	return mesh;
//}
///*MO TEST*/
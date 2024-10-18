#include "TriangleMesh.h"
#include <unordered_map>

// Hash function for VertexPTN to use in unordered_map.
struct VertexPTNHash
{
	std::size_t operator()(const VertexPTN &v) const
	{
		// Combine the hashes of position, normal, and texcoord.
		return ((std::hash<float>()(v.position.x) ^ (std::hash<float>()(v.position.y) << 1)) >> 1) ^ (std::hash<float>()(v.position.z)) ^ (std::hash<float>()(v.normal.x) << 1) ^ (std::hash<float>()(v.normal.y)) ^ (std::hash<float>()(v.normal.z) << 1) ^ (std::hash<float>()(v.texcoord.x) << 1) ^ (std::hash<float>()(v.texcoord.y));
	}
};

// Equality operator for VertexPTN to use in unordered_map.
bool operator==(const VertexPTN &lhs, const VertexPTN &rhs)
{
	return lhs.position == rhs.position && lhs.normal == rhs.normal && lhs.texcoord == rhs.texcoord;
}

// Desc: Constructor of a triangle mesh.
TriangleMesh::TriangleMesh()
{
	numVertices = 0;
	numTriangles = 0;
	objCenter = glm::vec3(0.0f, 0.0f, 0.0f);
	vboId = 0;
	iboId = 0;
}

// Desc: Destructor of a triangle mesh.
TriangleMesh::~TriangleMesh()
{
	vertices.clear();
	vertexIndices.clear();
	glDeleteBuffers(1, &vboId);
	glDeleteBuffers(1, &iboId);
}

// Desc: Load the geometry data of the model from file and normalize it.
bool TriangleMesh::LoadFromFile(const std::string &filePath, const bool normalized)
{
	// Add your code here.
	// ...

	// Open the file.
	std::ifstream file(filePath);
	if (!file.is_open())
	{
		std::cerr << "Cannot open file: " << filePath << std::endl;
		return false;
	}

	// Read the file line by line.
	std::string line;
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texcoords;
	std::unordered_map<VertexPTN, unsigned int, VertexPTNHash> uniqueVertices;
	while (std::getline(file, line))
	{
		std::istringstream iss(line);
		std::string type;
		iss >> type;

		if (type == "v")
		{
			// Read vertex position.
			glm::vec3 pos;
			iss >> pos.x >> pos.y >> pos.z;
			positions.push_back(pos);
		}
		else if (type == "vt")
		{
			// Read vertex texture coordinate.
			glm::vec2 texcoord;
			iss >> texcoord.x >> texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (type == "vn")
		{
			// Read vertex normal.
			glm::vec3 normal;
			iss >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);
		}
		else if (type == "f")
		{
			// Read face data.
			std::vector<std::string> faceData;
			std::string vertexStr;

			// Read all vertices for this face.
			while (iss >> vertexStr)
			{
				faceData.push_back(vertexStr);
			}

			// Check if we have enough vertices to form triangles.
			if (faceData.size() >= 3)
			{
				// Create a vertex for the first vertex.
				std::istringstream firstVertexStream(faceData[0]);
				int firstIndex[3] = {0, 0, 0}; // Position, texcoord, normal
				VertexPTN firstVertex;

				// Parse the first vertex data.
				std::string indexStr;
				for (int j = 0; j < 3; ++j)
				{
					std::getline(firstVertexStream, indexStr, '/');
					if (!indexStr.empty())
					{
						firstIndex[j] = std::stoi(indexStr) - 1; // OBJ is 1-indexed
					}
				}

				// Assign position, texcoord, and normal for the first vertex.
				firstVertex.position = positions[firstIndex[0]];
				if (firstIndex[1] >= 0)
					firstVertex.texcoord = texcoords[firstIndex[1]];
				if (firstIndex[2] >= 0)
					firstVertex.normal = normals[firstIndex[2]];

				// Store the first vertex index.
				unsigned int firstVertexIndex;
				if (uniqueVertices.find(firstVertex) == uniqueVertices.end())
				{
					firstVertexIndex = numVertices;
					uniqueVertices[firstVertex] = firstVertexIndex;
					vertices.push_back(firstVertex);
					numVertices++;
				}
				else
				{
					firstVertexIndex = uniqueVertices[firstVertex];
				}

				// Process the second vertex.
				std::istringstream secondVertexStream(faceData[1]);
				int secondIndex[3] = {0, 0, 0};
				VertexPTN secondVertex;

				// Parse the second vertex data.
				for (int j = 0; j < 3; ++j)
				{
					std::getline(secondVertexStream, indexStr, '/');
					if (!indexStr.empty())
					{
						secondIndex[j] = std::stoi(indexStr) - 1;
					}
				}

				// Assign position, texcoord, and normal for the second vertex.
				secondVertex.position = positions[secondIndex[0]];
				if (secondIndex[1] >= 0)
					secondVertex.texcoord = texcoords[secondIndex[1]];
				if (secondIndex[2] >= 0)
					secondVertex.normal = normals[secondIndex[2]];

				// Store the second vertex index.
				unsigned int secondVertexIndex;
				if (uniqueVertices.find(secondVertex) == uniqueVertices.end())
				{
					secondVertexIndex = numVertices;
					uniqueVertices[secondVertex] = secondVertexIndex;
					vertices.push_back(secondVertex);
					numVertices++;
				}
				else
				{
					secondVertexIndex = uniqueVertices[secondVertex];
				}

				// Iterate over the remaining vertices to form triangles.
				for (size_t i = 2; i < faceData.size(); ++i)
				{
					VertexPTN vertex;

					// Parse the current vertex data.
					std::istringstream currentVertexStream(faceData[i]);
					int indices[3] = {0, 0, 0};
					for (int j = 0; j < 3; ++j)
					{
						std::getline(currentVertexStream, indexStr, '/');
						if (!indexStr.empty())
						{
							indices[j] = std::stoi(indexStr) - 1;
						}
					}

					// Assign position, texcoord, and normal based on the current vertex data.
					vertex.position = positions[indices[0]];
					if (indices[1] >= 0)
						vertex.texcoord = texcoords[indices[1]];
					if (indices[2] >= 0)
						vertex.normal = normals[indices[2]];

					// Check if the vertex has already been added.
					unsigned int currentVertexIndex;
					if (uniqueVertices.find(vertex) == uniqueVertices.end())
					{
						currentVertexIndex = numVertices;
						uniqueVertices[vertex] = currentVertexIndex;
						vertices.push_back(vertex);
						numVertices++;
					}
					else
					{
						currentVertexIndex = uniqueVertices[vertex];
					}

					// Form triangles using the first, second, and current vertices.
					vertexIndices.push_back(firstVertexIndex);	 // First vertex.
					vertexIndices.push_back(secondVertexIndex);	 // Second vertex for the first triangle.
					vertexIndices.push_back(currentVertexIndex); // Current vertex.

					// Update secondVertexIndex for the next iteration.
					secondVertexIndex = currentVertexIndex;
				}

				// Increase the triangle count.
				numTriangles += faceData.size() - 2; // Number of triangles formed.
			}
		}
	}

	if (normalized)
	{
		// Normalize the geometry data.
		// Add your code here.
		// ...

		// 步驟 1：計算包圍盒 (Bounding Box)
		glm::vec3 minPos = vertices[0].position;
		glm::vec3 maxPos = vertices[0].position;

		for (const auto &vertex : vertices)
		{
			minPos = glm::min(minPos, vertex.position);
			maxPos = glm::max(maxPos, vertex.position);
		}

		// 步驟 2：計算最大邊長 (maxDimension)
		glm::vec3 size = maxPos - minPos;
		float maxDimension = std::max(size.x, std::max(size.y, size.z)); // 找到最大邊長

		// 步驟 3：計算包圍盒中心點 (objCenter)
		glm::vec3 objCenter = (minPos + maxPos) / 2.0f;

		// 步驟 4：平移並縮放頂點
		for (auto &vertex : vertices)
		{
			vertex.position = (vertex.position - objCenter) / maxDimension; // 平移到中心並縮放
		}
	}

	PrintMeshInfo();
	return true;
}

// Desc: Create vertex buffer and index buffer.
void TriangleMesh::CreateBuffers()
{
	// Add your code here.
	// ...

	// Create vertex buffer.
	glGenBuffers(1, &vboId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexPTN), vertices.data(), GL_STATIC_DRAW);

	// Create index buffer.
	glGenBuffers(1, &iboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexIndices.size() * sizeof(unsigned int), vertexIndices.data(), GL_STATIC_DRAW);
}

// Desc: Apply transformation to all vertices (DON'T NEED TO TOUCH)
void TriangleMesh::ApplyTransformCPU(const glm::mat4x4 &mvpMatrix)
{
	for (int i = 0; i < numVertices; ++i)
	{
		glm::vec4 p = mvpMatrix * glm::vec4(vertices[i].position, 1.0f);
		if (p.w != 0.0f)
		{
			float inv = 1.0f / p.w;
			vertices[i].position.x = p.x * inv;
			vertices[i].position.y = p.y * inv;
			vertices[i].position.z = p.z * inv;
		}
	}
}

// Desc: Print mesh information.
void TriangleMesh::PrintMeshInfo() const
{
	std::cout << "[*] Mesh Information: " << std::endl;
	std::cout << "# Vertices: " << numVertices << std::endl;
	std::cout << "# Triangles: " << numTriangles << std::endl;
	std::cout << "Center: (" << objCenter.x << " , " << objCenter.y << " , " << objCenter.z << ")" << std::endl;
}

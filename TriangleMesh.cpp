#include "TriangleMesh.h"
#include <unordered_map>

// Hash 是用來將 VertexPTN 轉換成一個 hash 值，這樣就可以用來當作 unordered_map 的 key，這樣就可以快速查找重複的頂點
struct VertexPTNHash
{
	std::size_t operator()(const VertexPTN &v) const
	{
		// Combine the hashes of position, normal, and texcoord.
		return ((std::hash<float>()(v.position.x) ^ (std::hash<float>()(v.position.y) << 1)) >> 1) ^ (std::hash<float>()(v.position.z)) ^ (std::hash<float>()(v.normal.x) << 1) ^ (std::hash<float>()(v.normal.y)) ^ (std::hash<float>()(v.normal.z) << 1) ^ (std::hash<float>()(v.texcoord.x) << 1) ^ (std::hash<float>()(v.texcoord.y));
	}
};

// == 的定義，用來比較兩個 VertexPTN 是否相等
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

	// 讀取 obj 檔案中的每一行
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
			glm::vec3 pos;
			iss >> pos.x >> pos.y >> pos.z;
			positions.push_back(pos);
		}
		else if (type == "vt")
		{
			glm::vec2 texcoord;
			iss >> texcoord.x >> texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (type == "vn")
		{
			glm::vec3 normal;
			iss >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);
		}
		else if (type == "f")
		{
			std::vector<std::string> faceData;
			std::string vertexStr;

			while (iss >> vertexStr)
			{
				faceData.push_back(vertexStr);
			}

			// 避免小於 3 個頂點的情況
			if (faceData.size() >= 3)
			{
				// 創第一個頂點
				std::istringstream firstVertexStream(faceData[0]);
				int firstIndex[3] = {0, 0, 0}; // Position, texcoord, normal
				VertexPTN firstVertex;

				std::string indexStr;
				for (int j = 0; j < 3; ++j)
				{
					std::getline(firstVertexStream, indexStr, '/');
					if (!indexStr.empty())
					{
						firstIndex[j] = std::stoi(indexStr) - 1; // OBJ 檔案的 index 從 1 開始，所以要減 1
					}
				}

				// 把 position, texcoord, normal 賦值給 firstVertex
				firstVertex.position = positions[firstIndex[0]];
				if (firstIndex[1] >= 0)
					firstVertex.texcoord = texcoords[firstIndex[1]];
				if (firstIndex[2] >= 0)
					firstVertex.normal = normals[firstIndex[2]];

				// Store 起來
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

				// 創第二個頂點
				std::istringstream secondVertexStream(faceData[1]);
				int secondIndex[3] = {0, 0, 0};
				VertexPTN secondVertex;

				for (int j = 0; j < 3; ++j)
				{
					std::getline(secondVertexStream, indexStr, '/');
					if (!indexStr.empty())
					{
						secondIndex[j] = std::stoi(indexStr) - 1;
					}
				}

				secondVertex.position = positions[secondIndex[0]];
				if (secondIndex[1] >= 0)
					secondVertex.texcoord = texcoords[secondIndex[1]];
				if (secondIndex[2] >= 0)
					secondVertex.normal = normals[secondIndex[2]];

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

				// 依序處理第三個頂點以後的頂點
				for (size_t i = 2; i < faceData.size(); ++i)
				{
					VertexPTN vertex;

					// 處理第 i 個頂點
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

					// 把 position, texcoord, normal 賦值給 vertex
					vertex.position = positions[indices[0]];
					if (indices[1] >= 0)
						vertex.texcoord = texcoords[indices[1]];
					if (indices[2] >= 0)
						vertex.normal = normals[indices[2]];

					// 檢查是否已經存在
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

					// 讓三角型為：第1, i-1, i個頂點組成
					vertexIndices.push_back(firstVertexIndex);	 // 固定的第一個頂點
					vertexIndices.push_back(secondVertexIndex);	 // 第二個頂點（上一個 currentVertexIndex）
					vertexIndices.push_back(currentVertexIndex); // Current vertex.

					// 更新 secondVertexIndex
					secondVertexIndex = currentVertexIndex;
				}

				// 更新三角形數量
				numTriangles += faceData.size() - 2;
			}
		}
	}

	if (normalized)
	{
		// Normalize the geometry data.
		// Add your code here.
		// ...

		// 計算bBounding Box, 找到最小和最大的位置
		glm::vec3 minPos = vertices[0].position;
		glm::vec3 maxPos = vertices[0].position;

		for (const auto &vertex : vertices)
		{
			minPos = glm::min(minPos, vertex.position);
			maxPos = glm::max(maxPos, vertex.position);
		}

		// 計算最大邊長 (maxDimension)
		glm::vec3 size = maxPos - minPos;
		float maxDimension = std::max(size.x, std::max(size.y, size.z)); // 找到最大邊長

		// 計算包圍盒中心點 (objCenter)
		glm::vec3 objCenter = (minPos + maxPos) / 2.0f;

		// 平移並縮放頂點
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

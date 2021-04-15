#include "Shape.h"
#include <algorithm>
#include <iostream>

#include "GLSL.h"
#include "Program.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>



#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

using namespace std;

Shape::Shape() :
	posBufID(0),
	norBufID(0),
	texBufID(0) {
}

Shape::~Shape() {
}

void Shape::loadMesh(const string& meshName) {
	// Load geometry
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	string errStr;
	bool rc = tinyobj::LoadObj(&attrib, &shapes, &materials, &errStr, meshName.c_str());
	if (!rc) {
		cerr << errStr << endl;
	} else {
		// Some OBJ files have different indices for vertex positions, normals,
		// and texture coordinates. For example, a cube corner vertex may have
		// three different normals. Here, we are going to duplicate all such
		// vertices.
		// Loop over shapes
		for (size_t s = 0; s < shapes.size(); s++) {
			// Loop over faces (polygons)
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
				size_t fv = shapes[s].mesh.num_face_vertices[f];
				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					posBuf.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
					posBuf.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
					posBuf.push_back(attrib.vertices[3 * idx.vertex_index + 2]);
					if (!attrib.normals.empty()) {
						norBuf.push_back(attrib.normals[3 * idx.normal_index + 0]);
						norBuf.push_back(attrib.normals[3 * idx.normal_index + 1]);
						norBuf.push_back(attrib.normals[3 * idx.normal_index + 2]);
					}
					if (!attrib.texcoords.empty()) {
						texBuf.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
						texBuf.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
					}
				}
				index_offset += fv;
				// per-face material (IGNORE)
				shapes[s].mesh.material_ids[f];
			}
		}
	}
}

void Shape::loadPlane() {
	vector<float> iposBuf; // temporary indexed version of posBuf
	vector<float> inorBuf;
	vector<float> itexBuf;
	vector<unsigned int> indBuf;
	int nRows = 10, nCols = 10;
	glm::vec3 gridMax = glm::vec3(0.5f, 0.5f, 0.0f);
	glm::vec3 gridMin = glm::vec3(-0.5f, -0.5f, 0.0f);
	glm::vec3 gridSize = gridMax - gridMin;
	gridSize.x /= (nRows - 1);
	gridSize.y /= (nCols - 1);
	glm::vec2 texSize = { 1.0f, 1.0f };
	texSize.x /= (nRows - 1);
	texSize.y /= (nCols - 1);
	for (int r = 0; r < nRows; r++) {
		for (int c = 0; c < nCols; c++) {
			// position
			iposBuf.push_back(r * gridSize.x + gridMin.x);
			iposBuf.push_back(c * gridSize.y + gridMin.y);
			iposBuf.push_back(0.0f);
			// normal
			inorBuf.push_back(0.0f);
			inorBuf.push_back(0.0f);
			inorBuf.push_back(1.0f);
			// texture
			itexBuf.push_back(r * texSize.x);
			itexBuf.push_back(c * texSize.y);
		}
	}
	for (int r = 0; r < nRows - 1; r++) {
		for (int c = 0; c < nCols - 1; c++) {
			// for each square { a, b, c, d}
			// push (a), push (d), push (b)
			// push (a), push (c), push (d)
			glm::vec4 square = { c + nCols * r, (c + 1) + nCols * r, c + nCols * (r + 1), (c + 1) + nCols * (r + 1) };
			// triangle 1
			indBuf.push_back((int)square.x);
			indBuf.push_back((int)square.w);
			indBuf.push_back((int)square.y);
			// triangle 2
			indBuf.push_back((int)square.x);
			indBuf.push_back((int)square.z);
			indBuf.push_back((int)square.w);
		}
	}
	// fill array version of buffers
	for (int v = 0; v < indBuf.size(); v++) {
		posBuf.push_back(iposBuf.at(indBuf.at(v) * 3));
		posBuf.push_back(iposBuf.at(indBuf.at(v) * 3 + 1));
		posBuf.push_back(iposBuf.at(indBuf.at(v) * 3 + 2));
		norBuf.push_back(inorBuf.at(indBuf.at(v) * 3));
		norBuf.push_back(inorBuf.at(indBuf.at(v) * 3 + 1));
		norBuf.push_back(inorBuf.at(indBuf.at(v) * 3 + 2));
		texBuf.push_back(itexBuf.at(indBuf.at(v) * 2));
		texBuf.push_back(itexBuf.at(indBuf.at(v) * 2 + 1));
	}
}

void Shape::loadSphere() {
	// procedurally create sphere instead of obj
	// ideally we would use indexed drawing in draw, but for this, I am just going to convert to array drawing after creating sphere
	vector<float> iposBuf; // temporary indexed version of posBuf
	vector<float> inorBuf;
	vector<float> itexBuf;
	vector<unsigned int> indBuf;
	int nRows = 50, nCols = 50;
	float pi = 3.14159265358979323846f;
	glm::vec3 sphBounds = { 1.0f, pi, pi * 2 }; // {r, theta, phi}
	sphBounds.y /= (nRows - 1);
	sphBounds.z /= (nCols - 1);
	glm::vec2 texSize = { 1.0f, 1.0f };
	texSize.x /= (nRows - 1);
	texSize.y /= (nCols - 1);
	// for theta = 0 -> pi, nRows
	for (int r = 0; r < nRows; r++) {
		// for phi = 0 -> 2 pi, nCols
		for (int c = 0; c < nCols; c++) {
			// position
			float radius = sphBounds.x;
			float theta = r * sphBounds.y, phi = c * sphBounds.z;
			iposBuf.push_back(radius * (float)(sin(theta) * sin(phi)));
			iposBuf.push_back(radius * (float)(cos(theta)));
			iposBuf.push_back(radius * (float)(sin(theta) * cos(phi)));
			// normal
			inorBuf.push_back((float)(sin(theta) * sin(phi)));
			inorBuf.push_back((float)(cos(theta)));
			inorBuf.push_back((float)(sin(theta) * cos(phi)));
			// texture
			itexBuf.push_back(c * texSize.y);
			itexBuf.push_back(1.0f - r * texSize.x); // flip y coord for texture
		}
	}
	// create triangles
	for (int r = 0; r < nRows - 1; r++) {
		for (int c = 0; c < nCols - 1; c++) {
			// for each square { a, b, c, d}
			// push (a), push (d), push (b)
			// push (a), push (c), push (d)
			glm::vec4 square = { c + nCols * r, (c + 1) + nCols * r, c + nCols * (r + 1), (c + 1) + nCols * (r + 1) };
			// triangle 1
			indBuf.push_back((int)square.x);
			indBuf.push_back((int)square.w);
			indBuf.push_back((int)square.y);
			// triangle 2
			indBuf.push_back((int)square.x);
			indBuf.push_back((int)square.z);
			indBuf.push_back((int)square.w);
		}
	}
	// fill array version of buffers
	for (int v = 0; v < indBuf.size(); v++) {
		posBuf.push_back(iposBuf.at(indBuf.at(v) * 3));
		posBuf.push_back(iposBuf.at(indBuf.at(v) * 3 + 1));
		posBuf.push_back(iposBuf.at(indBuf.at(v) * 3 + 2));
		norBuf.push_back(inorBuf.at(indBuf.at(v) * 3));
		norBuf.push_back(inorBuf.at(indBuf.at(v) * 3 + 1));
		norBuf.push_back(inorBuf.at(indBuf.at(v) * 3 + 2));
		texBuf.push_back(itexBuf.at(indBuf.at(v) * 2));
		texBuf.push_back(itexBuf.at(indBuf.at(v) * 2 + 1));
	}
}

void Shape::loadRev() {
	// procedurally create surface of revolution, to be computed on gpu
	// ideally we would use indexed drawing in draw, but for this, I am just going to convert to array drawing after creating sphere
	vector<float> iposBuf; // temporary indexed version of posBuf
	vector<float> itexBuf;
	vector<unsigned int> indBuf;
	int nRows = 50, nCols = 50;
	float pi = 3.14159265358979323846f;
	glm::vec2 revBounds = { 10.0f, pi * 2 }; // {x, theta}

	revBounds.x /= (nRows - 1);
	revBounds.y /= (nCols - 1);
	glm::vec2 texSize = { 10.0f, 10.0f };
	texSize.x /= (nRows - 1);
	texSize.y /= (nCols - 1);
	// for x = 0 -> 10, nRows
	for (int r = 0; r < nRows; r++) {
		// for theta = 0 -> 2 pi, nCols
		for (int c = 0; c < nCols; c++) {
			// position
			float x = r * revBounds.x;
			float theta = c * revBounds.y;
			iposBuf.push_back(x);
			iposBuf.push_back(theta);
			iposBuf.push_back(0.0f);
			// texture
			itexBuf.push_back(c * texSize.y);
			itexBuf.push_back(r * texSize.x); // flip y coord for texture
		}
	}
	for (int r = 0; r < nRows - 1; r++) {
		for (int c = 0; c < nCols - 1; c++) {
			// for each square { a, b, c, d}
			// push (a), push (d), push (b)
			// push (a), push (c), push (d)
			glm::vec4 square = { c + nCols * r, (c + 1) + nCols * r, c + nCols * (r + 1), (c + 1) + nCols * (r + 1) };
			// triangle 1
			indBuf.push_back((int)square.x);
			indBuf.push_back((int)square.y);
			indBuf.push_back((int)square.w);
			// triangle 2
			indBuf.push_back((int)square.x);
			indBuf.push_back((int)square.w);
			indBuf.push_back((int)square.z);
		}
	}
	// fill array version of buffers
	for (int v = 0; v < indBuf.size(); v++) {
		posBuf.push_back(iposBuf.at(indBuf.at(v) * 3));
		posBuf.push_back(iposBuf.at(indBuf.at(v) * 3 + 1));
		posBuf.push_back(iposBuf.at(indBuf.at(v) * 3 + 2));
		texBuf.push_back(itexBuf.at(indBuf.at(v) * 2));
		texBuf.push_back(itexBuf.at(indBuf.at(v) * 2 + 1));
	}
}


void Shape::fitToUnitBox() {
	// Scale the vertex positions so that they fit within [-1, +1] in all three dimensions.
	glm::vec3 vmin(posBuf[0], posBuf[1], posBuf[2]);
	glm::vec3 vmax(posBuf[0], posBuf[1], posBuf[2]);
	for (int i = 0; i < (int)posBuf.size(); i += 3) {
		glm::vec3 v(posBuf[i], posBuf[i + 1], posBuf[i + 2]);
		vmin.x = min(vmin.x, v.x);
		vmin.y = min(vmin.y, v.y);
		vmin.z = min(vmin.z, v.z);
		vmax.x = max(vmax.x, v.x);
		vmax.y = max(vmax.y, v.y);
		vmax.z = max(vmax.z, v.z);
	}
	glm::vec3 center = 0.5f * (vmin + vmax);
	glm::vec3 diff = vmax - vmin;
	float diffmax = diff.x;
	diffmax = max(diffmax, diff.y);
	diffmax = max(diffmax, diff.z);
	float scale = 1.0f / diffmax;
	for (int i = 0; i < (int)posBuf.size(); i += 3) {
		posBuf[i] = (posBuf[i] - center.x) * scale;
		posBuf[i + 1] = (posBuf[i + 1] - center.y) * scale;
		posBuf[i + 2] = (posBuf[i + 2] - center.z) * scale;
	}
}

void Shape::init() {
	// Send the position array to the GPU
	glGenBuffers(1, &posBufID);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size() * sizeof(float), &posBuf[0], GL_STATIC_DRAW);

	// Send the normal array to the GPU
	if (!norBuf.empty()) {
		glGenBuffers(1, &norBufID);
		glBindBuffer(GL_ARRAY_BUFFER, norBufID);
		glBufferData(GL_ARRAY_BUFFER, norBuf.size() * sizeof(float), &norBuf[0], GL_STATIC_DRAW);
	}

	// Send the texture array to the GPU
	if (!texBuf.empty()) {
		glGenBuffers(1, &texBufID);
		glBindBuffer(GL_ARRAY_BUFFER, texBufID);
		glBufferData(GL_ARRAY_BUFFER, texBuf.size() * sizeof(float), &texBuf[0], GL_STATIC_DRAW);
	}

	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLSL::checkError(GET_FILE_LINE);
}

void Shape::draw(const shared_ptr<Program> prog) const {
	// Bind position buffer
	int h_pos = prog->getAttribute("aPos");
	glEnableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);

	// Bind normal buffer
	int h_nor = prog->getAttribute("aNor");
	if (h_nor != -1 && norBufID != 0) {
		glEnableVertexAttribArray(h_nor);
		glBindBuffer(GL_ARRAY_BUFFER, norBufID);
		glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);
	}

	// Bind texcoords buffer
	int h_tex = prog->getAttribute("aTex");
	if (h_tex != -1 && texBufID != 0) {
		glEnableVertexAttribArray(h_tex);
		glBindBuffer(GL_ARRAY_BUFFER, texBufID);
		glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void*)0);
	}

	// Draw
	int count = posBuf.size() / 3; // number of indices to be rendered
	glDrawArrays(GL_TRIANGLES, 0, count);

	// Disable and unbind
	if (h_tex != -1) {
		glDisableVertexAttribArray(h_tex);
	}
	if (h_nor != -1) {
		glDisableVertexAttribArray(h_nor);
	}
	glDisableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLSL::checkError(GET_FILE_LINE);
}

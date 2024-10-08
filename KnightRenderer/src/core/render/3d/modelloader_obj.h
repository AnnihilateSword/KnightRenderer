////////////////////////////////////////////////////////////////////////////////
// Filename: modelloader_obj.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _MODELLOADER_OBJ_H_
#define _MODELLOADER_OBJ_H_


//////////////
// INCLUDES //
//////////////
#include <iostream>
#include <fstream>


class ObjModelLoadder
{
	//////////////
	// TYPEDEFS //
	//////////////
	typedef struct
	{
		float x, y, z;
	}VertexType;

	typedef struct
	{
		int vIndex1, vIndex2, vIndex3;
		int tIndex1, tIndex2, tIndex3;
		int nIndex1, nIndex2, nIndex3;
	}FaceType;


public:
	//////////////////
	// LoadObjModel //
	//////////////////
	bool LoadObjModel(const char* filename)
	{
		bool result;
		int vertexCount, textureCount, normalCount, faceCount;
		std::ifstream fin;


		// Read in the name of the model file.
		// Attempt to open the file.
		fin.open(filename);
		if (!fin.good())
		{
			std::cout << "File " << filename << " could not be opened." << std::endl << std::endl;
			return false;
		}

		// Read in the number of vertices, tex coords, normals, and faces so that the data structures can be initialized with the exact sizes needed.
		result = ReadFileCounts(filename, vertexCount, textureCount, normalCount, faceCount);
		if (!result)
		{
			return false;
		}

		// Display the counts to the screen for information purposes.
		std::cout << std::endl;
		std::cout << "Vertices: " << vertexCount << std::endl;
		std::cout << "UVs:      " << textureCount << std::endl;
		std::cout << "Normals:  " << normalCount << std::endl;
		std::cout << "Faces:    " << faceCount << std::endl;

		// Now read the data from the file into the data structures and then output it in our model format.
		result = LoadDataStructures(filename, vertexCount, textureCount, normalCount, faceCount);
		if (!result)
		{
			return false;
		}

		return true;
	}

	bool ReadFileCounts(const char* filename, int& vertexCount, int& textureCount, int& normalCount, int& faceCount)
	{
		std::ifstream fin;
		char input;


		// Initialize the counts.
		vertexCount = 0;
		textureCount = 0;
		normalCount = 0;
		faceCount = 0;

		// Open the file.
		fin.open(filename);

		// Check if it was successful in opening the file.
		if (fin.fail() == true)
		{
			return false;
		}

		// Read from the file and continue to read until the end of the file is reached.
		fin.get(input);
		while (!fin.eof())
		{
			// If the line starts with 'v' then count either the vertex, the texture coordinates, or the normal vector.
			if (input == 'v')
			{
				fin.get(input);
				if (input == ' ') { vertexCount++; }
				if (input == 't') { textureCount++; }
				if (input == 'n') { normalCount++; }
			}

			// If the line starts with 'f' then increment the face count.
			if (input == 'f')
			{
				fin.get(input);
				if (input == ' ') { faceCount++; }
			}

			// Otherwise read in the remainder of the line.
			while (input != '\n')
			{
				fin.get(input);
			}

			// Start reading the beginning of the next line.
			fin.get(input);
		}

		// Close the file.
		fin.close();

		return true;
	}


	bool LoadDataStructures(const char* filename, int vertexCount, int textureCount, int normalCount, int faceCount)
	{
		VertexType* vertices, * texcoords, * normals;
		FaceType* faces;
		std::ifstream fin;
		int vertexIndex, texcoordIndex, normalIndex, faceIndex, vIndex, tIndex, nIndex;
		char input, input2;
		std::ofstream fout;


		// Initialize the four data structures.
		vertices = new VertexType[vertexCount];

		texcoords = new VertexType[textureCount];

		normals = new VertexType[normalCount];

		faces = new FaceType[faceCount];

		// Initialize the indexes.
		vertexIndex = 0;
		texcoordIndex = 0;
		normalIndex = 0;
		faceIndex = 0;

		// Open the file.
		fin.open(filename);

		// Check if it was successful in opening the file.
		if (fin.fail() == true)
		{
			return false;
		}

		// Read in the vertices, texture coordinates, and normals into the data structures.
		// Important: Also convert to left hand coordinate system since Maya uses right hand coordinate system.
		fin.get(input);
		while (!fin.eof())
		{
			if (input == 'v')
			{
				fin.get(input);

				// Read in the vertices.
				if (input == ' ')
				{
					fin >> vertices[vertexIndex].x >> vertices[vertexIndex].y >> vertices[vertexIndex].z;

					// Invert the Z vertex to change to left hand system.
					vertices[vertexIndex].z = vertices[vertexIndex].z * -1.0f;
					vertexIndex++;
				}

				// Read in the texture uv coordinates.
				if (input == 't')
				{
					fin >> texcoords[texcoordIndex].x >> texcoords[texcoordIndex].y;

					// Invert the V texture coordinates to left hand system.
					texcoords[texcoordIndex].y = 1.0f - texcoords[texcoordIndex].y;
					texcoordIndex++;
				}

				// Read in the normals.
				if (input == 'n')
				{
					fin >> normals[normalIndex].x >> normals[normalIndex].y >> normals[normalIndex].z;

					// Invert the Z normal to change to left hand system.
					normals[normalIndex].z = normals[normalIndex].z * -1.0f;
					normalIndex++;
				}
			}

			// Read in the faces.
			if (input == 'f')
			{
				fin.get(input);
				if (input == ' ')
				{
					// Read the face data in backwards to convert it to a left hand system from right hand system.
					fin >> faces[faceIndex].vIndex3 >> input2 >> faces[faceIndex].tIndex3 >> input2 >> faces[faceIndex].nIndex3
						>> faces[faceIndex].vIndex2 >> input2 >> faces[faceIndex].tIndex2 >> input2 >> faces[faceIndex].nIndex2
						>> faces[faceIndex].vIndex1 >> input2 >> faces[faceIndex].tIndex1 >> input2 >> faces[faceIndex].nIndex1;
					faceIndex++;
				}
			}

			// Read in the remainder of the line.
			while (input != '\n')
			{
				fin.get(input);
			}

			// Start reading the beginning of the next line.
			fin.get(input);
		}

		// Close the file.
		fin.close();

		// Open the output file.
		char path[128];
		strcpy_s(path, filename);
		strcat_s(path, sizeof(path), ".txt");
		fout.open(path);

		// Write out the file header that our model format uses.
		fout << "Vertex Count: " << (faceCount * 3) << std::endl;
		fout << std::endl;
		fout << "Data:" << std::endl;
		fout << std::endl;

		// Now loop through all the faces and output the three vertices for each face.
		for (int i = 0; i < faceIndex; i++)
		{
			vIndex = faces[i].vIndex1 - 1;
			tIndex = faces[i].tIndex1 - 1;
			nIndex = faces[i].nIndex1 - 1;

			fout << vertices[vIndex].x << ' ' << vertices[vIndex].y << ' ' << vertices[vIndex].z << ' '
				<< texcoords[tIndex].x << ' ' << texcoords[tIndex].y << ' '
				<< normals[nIndex].x << ' ' << normals[nIndex].y << ' ' << normals[nIndex].z << std::endl;

			vIndex = faces[i].vIndex2 - 1;
			tIndex = faces[i].tIndex2 - 1;
			nIndex = faces[i].nIndex2 - 1;

			fout << vertices[vIndex].x << ' ' << vertices[vIndex].y << ' ' << vertices[vIndex].z << ' '
				<< texcoords[tIndex].x << ' ' << texcoords[tIndex].y << ' '
				<< normals[nIndex].x << ' ' << normals[nIndex].y << ' ' << normals[nIndex].z << std::endl;

			vIndex = faces[i].vIndex3 - 1;
			tIndex = faces[i].tIndex3 - 1;
			nIndex = faces[i].nIndex3 - 1;

			fout << vertices[vIndex].x << ' ' << vertices[vIndex].y << ' ' << vertices[vIndex].z << ' '
				<< texcoords[tIndex].x << ' ' << texcoords[tIndex].y << ' '
				<< normals[nIndex].x << ' ' << normals[nIndex].y << ' ' << normals[nIndex].z << std::endl;
		}

		// Close the output file.
		fout.close();

		// Release the four data structures.
		if (vertices)
		{
			delete[] vertices;
			vertices = 0;
		}
		if (texcoords)
		{
			delete[] texcoords;
			texcoords = 0;
		}
		if (normals)
		{
			delete[] normals;
			normals = 0;
		}
		if (faces)
		{
			delete[] faces;
			faces = 0;
		}

		return true;
	}
};


#endif
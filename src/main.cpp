#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using json = nlohmann::json;

struct DecomposedPath {
	std::string parentPath = "";
	std::string stem = "";
	std::string extension = "";
};

struct ImageDataResource {
	std::vector<uint8_t> pixels;
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t channels = 0;
};

void decomposePath(DecomposedPath& decomposedPath, const std::string& path)
{
	std::filesystem::path filesystemPath(path);

	decomposedPath.parentPath = filesystemPath.parent_path().generic_string();
	decomposedPath.stem = filesystemPath.stem().generic_string();
	decomposedPath.extension = filesystemPath.extension().generic_string();
}

bool loadImage(ImageDataResource& imageDataResource, const std::string& filename)
{
	DecomposedPath decomposedPath;
	decomposePath(decomposedPath, filename);

	int x = 0;
	int y = 0;
	int comp = 0;
	int req_comp = 0;

	uint8_t* tempData = static_cast<uint8_t*>(stbi_load(filename.c_str(), &x, &y, &comp, req_comp));
	if (!tempData)
	{
		return false;
	}

	imageDataResource.width = static_cast<uint32_t>(x);
	imageDataResource.height = static_cast<uint32_t>(y);
	imageDataResource.channels = static_cast<uint32_t>(comp);
	imageDataResource.pixels.resize(imageDataResource.width * imageDataResource.height * imageDataResource.channels);
	memcpy(imageDataResource.pixels.data(), tempData, imageDataResource.width * imageDataResource.height * imageDataResource.channels);

	free(tempData);

	return true;
}

bool loadFile(std::string& output, const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open())
	{
		return false;
	}

	size_t fileSize = static_cast<size_t>(file.tellg());
	file.seekg(0);

	output.resize(fileSize);

	file.read(output.data(), fileSize);
	file.close();

	return true;
}

bool saveFile(const std::string& output, const std::string& filename)
{
	std::ofstream file(filename, std::ios::binary);
	if (!file.is_open())
	{
		return false;
	}

	file.write(output.data(), output.size());
	file.close();

	return true;
}

int main(int argc, char *argv[])
{
	const float voxelDimension = 2.0f;

    uint32_t pixelSize = 25;
    uint8_t redBackground = 245;
    uint8_t greenBackground = 245;
    uint8_t blueBackground = 245;

	if (argc <= 1)
	{
		printf("Usage: pixel2gltf2 image [-p 25 -r 245 -g 245 -b 245]\n");

		return 0;
	}

	//

	for (int i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], "-p") == 0 && (i + 1 < argc))
		{
			pixelSize = (uint32_t)std::stoi(argv[i + 1]);
		}
		else if (strcmp(argv[i], "-r") == 0 && (i + 1 < argc))
		{
			redBackground = (uint32_t)std::stoi(argv[i + 1]);
		}
		else if (strcmp(argv[i], "-g") == 0 && (i + 1 < argc))
		{
			greenBackground = (uint32_t)std::stoi(argv[i + 1]);
		}
		else if (strcmp(argv[i], "-b") == 0 && (i + 1 < argc))
		{
			blueBackground = (uint32_t)std::stoi(argv[i + 1]);
		}
	}

	//

	std::string filename = argv[1];

	DecomposedPath decomposedFilename;
	decomposePath(decomposedFilename, filename);

	ImageDataResource imageData;

	if (!loadImage(imageData, filename))
	{
		printf("Error: Could not load image file '%s'\n", filename.c_str());

		return -1;
	}

	//

	std::string loadname = "template.gltf";

    std::string templateContent;
	if (!loadFile(templateContent, loadname))
	{
		printf("Error: Could not load template glTF file '%s'\n", loadname.c_str());

		return -1;
	}

	//

    json glTF = json::parse(templateContent);

    //

    float xOffset = -(float)(imageData.width/pixelSize) * voxelDimension * 0.5f;
    float yOffset = (float)(imageData.height/pixelSize) * voxelDimension - voxelDimension * 0.5f;
    float zOffset = 0.0f;

    bool first = true;

    for (uint32_t y = 0; y < imageData.height; y+=pixelSize)
    {
        for (uint32_t x = 0; x < imageData.width; x+=pixelSize)
        {
        	uint8_t* pixel = &imageData.pixels[imageData.height * imageData.channels * y + imageData.channels * x];

        	if (pixel[0] != redBackground && pixel[1] != greenBackground && pixel[2] != blueBackground)
        	{
				//
				// Material
				//

				size_t materialIndex = glTF["materials"].size();
				if (first)
				{
					materialIndex = 0;
				}

				//

				json material = glTF["materials"][0];

				//

				json& baseColorFactor = material["pbrMetallicRoughness"]["baseColorFactor"];
				baseColorFactor[0] = powf((float)pixel[0] / 255.0f, 2.2f);
				baseColorFactor[1] = powf((float)pixel[1] / 255.0f, 2.2f);
				baseColorFactor[2] = powf((float)pixel[2] / 255.0f, 2.2f);

				//

				if (first)
				{
					glTF["materials"][0] = material;
				}
				else
				{
					glTF["materials"].push_back(material);
				}

				//
				// Mesh
				//

				size_t meshIndex = glTF["meshes"].size();
				if (first)
				{
					meshIndex = 0;
				}

				//

				json mesh = glTF["meshes"][0];

				//

				mesh["primitives"][0]["material"] = materialIndex;

				//

				if (first)
				{
					glTF["meshes"][0] = mesh;
				}
				else
				{
					glTF["meshes"].push_back(mesh);
				}

				//
				// Node
				//

				size_t nodeIndex = glTF["nodes"].size();

				//

				json node = json::object();

				//

				node["mesh"] = meshIndex;

				json translation = json::array();

				float xPos = xOffset + (float)(x/pixelSize) * voxelDimension;
				float yPos = yOffset - (float)(y/pixelSize) * voxelDimension;
				float zPos = zOffset;

				translation.push_back(xPos);
				translation.push_back(yPos);
				translation.push_back(zPos);

				node["translation"] = translation;

				//

				glTF["nodes"].push_back(node);

				//
				//

				glTF["nodes"][0]["children"].push_back(nodeIndex);

				//

				first = false;
        	}
        }
    }

    //

    std::string savename = decomposedFilename.parentPath + "/" + decomposedFilename.stem + ".gltf";

	if (!saveFile(glTF.dump(3), savename))
	{
		printf("Error: Could not save generated glTF file '%s'\n", savename.c_str());

		return -1;
	}

	printf("Success: Generated to '%s'\n", savename.c_str());

	return 0;
}

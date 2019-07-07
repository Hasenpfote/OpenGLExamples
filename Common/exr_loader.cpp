#include <cassert>
#define TINYEXR_IMPLEMENTATION
#include <tinyexr.h>
#include "logger.h"
#include "exr_loader.h"

ExrLoader::ExrLoader()
    : width(0), height(0), color_format(ColorFormat::Unknown), pixel_type(PixelType::Unknown), data()
{
}

bool ExrLoader::Load(const std::filesystem::path& filepath)
{
    Release();

    // 1. Read EXR version.
    EXRVersion exr_version;

    int ret = ParseEXRVersionFromFile(&exr_version, filepath.string().c_str());
    if(ret != 0)
    {
        LOG_E("Invalid EXR file: " << filepath.string().c_str());
        return false;
    }

    if(exr_version.multipart){
        // must be multipart flag is false.
        return false;
    }

    // 2. Read EXR header
    EXRHeader exr_header;
    InitEXRHeader(&exr_header);

    const char* err = nullptr;
    ret = ParseEXRHeaderFromFile(&exr_header, &exr_version, filepath.string().c_str(), &err);
    if(ret != 0)
    {
        LOG_E("Parse EXR err: " << err);
        FreeEXRErrorMessage(err); // free's buffer for an error message 
        return false;
    }
#if 0
    // Read HALF channel as FLOAT.
    for (int i = 0; i < exr_header.num_channels; i++) {
        if (exr_header.pixel_types[i] == TINYEXR_PIXELTYPE_HALF) {
            exr_header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
        }
    }
#endif
    EXRImage exr_image;
    InitEXRImage(&exr_image);

    ret = LoadEXRImageFromFile(&exr_image, &exr_header, filepath.string().c_str(), &err);
    if(ret != 0)
    {
        LOG_E("Load EXR err: " << err);
        FreeEXRHeader(&exr_header);
        FreeEXRErrorMessage(err); // free's buffer for an error message 
        return false;
    }

    // RGBA
    int idxR = -1;
    int idxG = -1;
    int idxB = -1;
    int idxA = -1;
    for(int c = 0; c < exr_header.num_channels; c++)
    {
        if(strcmp(exr_header.channels[c].name, "R") == 0)
        {
            idxR = c;
        }
        else
        if(strcmp(exr_header.channels[c].name, "G") == 0)
        {
            idxG = c;
        }
        else
        if(strcmp(exr_header.channels[c].name, "B") == 0)
        {
            idxB = c;
        }
        else
        if(strcmp(exr_header.channels[c].name, "A") == 0)
        {
            idxA = c;
        }
    }

    // 3. Access image data
    // `exr_image.images` will be filled when EXR is scanline format.
    // `exr_image.tiled` will be filled when EXR is tiled format.

    std::size_t num_of_channels = 0;
    if(exr_header.num_channels == 3)
    {
        color_format = ColorFormat::RGB;
        num_of_channels = 3;
    }
    else
    if(exr_header.num_channels == 4)
    {
        color_format = ColorFormat::RGBA;
        num_of_channels = 4;
    }
    else
    {
        color_format = ColorFormat::Unknown;
        assert(false);
    }

    std::size_t num_of_bytes_per_channel = 0;
    if(exr_header.pixel_types[0] == TINYEXR_PIXELTYPE_HALF)
    {
        pixel_type = PixelType::Half;
        num_of_bytes_per_channel = 2;
    }
    else
    if(exr_header.pixel_types[0] == TINYEXR_PIXELTYPE_FLOAT)
    {
        pixel_type = PixelType::Float;
        num_of_bytes_per_channel = 4;
    }
    else
    {
        pixel_type = PixelType::Unknown;
        assert(false);
    }

    width = static_cast<std::size_t>(exr_image.width);
    height = static_cast<std::size_t>(exr_image.height);

    auto size = width * height;

    std::unique_ptr<std::uint8_t[]> ptr = std::make_unique<std::uint8_t[]>(num_of_channels * num_of_bytes_per_channel * size);

    for(decltype(size) i = 0; i < size; i++)
    {
        auto src = &exr_image.images[idxR][num_of_bytes_per_channel * i];
        auto dst = &ptr[num_of_bytes_per_channel * (num_of_channels * i + 0)];
        memcpy(dst, src, num_of_bytes_per_channel);

        src = &exr_image.images[idxG][num_of_bytes_per_channel * i];
        dst = &ptr[num_of_bytes_per_channel * (num_of_channels * i + 1)];
        memcpy(dst, src, num_of_bytes_per_channel);

        src = &exr_image.images[idxB][num_of_bytes_per_channel * i];
        dst = &ptr[num_of_bytes_per_channel * (num_of_channels * i + 2)];
        memcpy(dst, src, num_of_bytes_per_channel);

        if(num_of_channels == 4)
        {
            src = &exr_image.images[idxA][num_of_bytes_per_channel * i];
            dst = &ptr[num_of_bytes_per_channel * (num_of_channels * i + 3)];
            memcpy(dst, src, num_of_bytes_per_channel);
        }
    }
    data = std::move(ptr);

    // 4. Free image data
    FreeEXRImage(&exr_image);
    FreeEXRHeader(&exr_header);

    return true;
}

void ExrLoader::Release()
{
    data.reset();
    color_format = ColorFormat::Unknown;
    pixel_type = PixelType::Unknown;
    width = height = 0;
}
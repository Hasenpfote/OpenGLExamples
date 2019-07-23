#include <hasenpfote/assert.h>
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define TINYEXR_IMPLEMENTATION
#include <tinyexr.h>
#include "logger.h"
#include "image.h"

Image::Image()
    : width(0), height(0), color_format(Image::ColorFormat::Unknown), pixel_type(Image::PixelType::Unknown), data()
{
}

bool Image::LoadFromFile(const std::filesystem::path& filepath)
{
    Release();

    if(filepath.extension() == ".exr")
        return LoadFromExrFile(filepath);

    const int num_of_requested_components = STBI_default;

    int width, height, num_of_components;
    std::unique_ptr<stbi_uc[], decltype(&stbi_image_free)> stbi_data(
        stbi_load(filepath.string().c_str(), &width, &height, &num_of_components, num_of_requested_components),
        &stbi_image_free
    );

    if(!stbi_data)
    {
        LOG_E("Failed to load image from file `" << filepath.string() << "`.");
        return false;
    }

    const auto num_of_actual_components = (num_of_requested_components > 0) ? num_of_requested_components : num_of_components;
    const auto data_size = width * height * num_of_actual_components;

    this->data = std::make_unique<std::uint8_t[]>(data_size);
    std::memcpy(this->data.get(), stbi_data.get(), data_size);
    this->width = static_cast<decltype(this->width)>(width);
    this->height = static_cast<decltype(this->height)>(height);

    if(num_of_actual_components == STBI_grey)
    {
        this->color_format = ColorFormat::R;
    }
    else if(num_of_actual_components == STBI_grey_alpha)
    {
        this->color_format = ColorFormat::RG;
    }
    else if(num_of_actual_components == STBI_rgb)
    {
        this->color_format = ColorFormat::RGB;
    }
    else if(num_of_actual_components == STBI_rgb_alpha)
    {
        this->color_format = ColorFormat::RGBA;
    }
    else
    {
        HASENPFOTE_ASSERT(false);
    }
    this->pixel_type = PixelType::UnsignedByte;

    return true;
}

bool Image::LoadFromExrFile(const std::filesystem::path& filepath)
{
    // 1. Read EXR version.
    EXRVersion exr_version;

    int ret = ParseEXRVersionFromFile(&exr_version, filepath.string().c_str());
    if(ret != 0)
    {
        LOG_E("Invalid EXR file: " << filepath.string());
        return false;
    }
    // must be multipart flag is false.
    HASENPFOTE_ASSERT(!exr_version.multipart);

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
    for (int i = 0; i < exr_header.num_channels; i++)
    {
        if (exr_header.pixel_types[i] == TINYEXR_PIXELTYPE_HALF)
        {
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
        else if(strcmp(exr_header.channels[c].name, "G") == 0)
        {
            idxG = c;
        }
        else if(strcmp(exr_header.channels[c].name, "B") == 0)
        {
            idxB = c;
        }
        else if(strcmp(exr_header.channels[c].name, "A") == 0)
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
    else if(exr_header.num_channels == 4)
    {
        color_format = ColorFormat::RGBA;
        num_of_channels = 4;
    }
    else
    {
        color_format = ColorFormat::Unknown;
        HASENPFOTE_ASSERT(false);
    }

    std::size_t bytes_per_channel = 0;
    if(exr_header.pixel_types[0] == TINYEXR_PIXELTYPE_HALF)
    {
        pixel_type = PixelType::Half;
        bytes_per_channel = 2;
    }
    else if(exr_header.pixel_types[0] == TINYEXR_PIXELTYPE_FLOAT)
    {
        pixel_type = PixelType::Float;
        bytes_per_channel = 4;
    }
    else
    {
        pixel_type = PixelType::Unknown;
        HASENPFOTE_ASSERT(false);
    }

    width = static_cast<std::size_t>(exr_image.width);
    height = static_cast<std::size_t>(exr_image.height);

    const auto num_of_pixels = width * height;
    const auto data_size = num_of_channels * bytes_per_channel * num_of_pixels;

    std::unique_ptr<std::uint8_t[]> ptr = std::make_unique<std::uint8_t[]>(data_size);

    for(std::remove_const<decltype(num_of_pixels)>::type i = 0; i < num_of_pixels; i++)
    {
        auto src = &exr_image.images[idxR][bytes_per_channel * i];
        auto dst = &ptr[bytes_per_channel * (num_of_channels * i + 0)];
        std::memcpy(dst, src, bytes_per_channel);

        src = &exr_image.images[idxG][bytes_per_channel * i];
        dst = &ptr[bytes_per_channel * (num_of_channels * i + 1)];
        std::memcpy(dst, src, bytes_per_channel);

        src = &exr_image.images[idxB][bytes_per_channel * i];
        dst = &ptr[bytes_per_channel * (num_of_channels * i + 2)];
        std::memcpy(dst, src, bytes_per_channel);

        if(num_of_channels == 4)
        {
            src = &exr_image.images[idxA][bytes_per_channel * i];
            dst = &ptr[bytes_per_channel * (num_of_channels * i + 3)];
            std::memcpy(dst, src, bytes_per_channel);
        }
    }
    data = std::move(ptr);

    // 4. Free image data
    FreeEXRImage(&exr_image);
    FreeEXRHeader(&exr_header);

    return true;
}

void Image::Release()
{
    width = height = 0;
    color_format = ColorFormat::Unknown;
    pixel_type = PixelType::Unknown;
    data.reset();
}

std::unique_ptr<std::uint8_t[]> Image::ExtractChannel(Channel channel) const
{
    if(!data || (color_format == ColorFormat::Unknown))
        return nullptr;

    if(channel == Channel::Green)
    {
        if(color_format == ColorFormat::R)
            return nullptr;
    }
    else if(channel == Channel::Blue)
    {
        if(color_format == ColorFormat::R || color_format == ColorFormat::RG)
            return nullptr;
    }
    else if(channel == Channel::Alpha)
    {
        if(color_format == ColorFormat::R || color_format == ColorFormat::RG || color_format == ColorFormat::RGB)
            return nullptr;
    }

    std::size_t bytes_per_channel = 0;
    if(pixel_type == PixelType::UnsignedByte)
        bytes_per_channel = 1;
    else if(pixel_type == PixelType::Half)
        bytes_per_channel = 2;
    else if(pixel_type == PixelType::Float)
        bytes_per_channel = 4;
    else
        HASENPFOTE_ASSERT(false);

    const auto num_of_channels = static_cast<std::size_t>(color_format);
    const auto src_stride = bytes_per_channel * num_of_channels;
    const auto src_row_bytes = src_stride * width;
    const auto dst_stride = bytes_per_channel;
    const auto dst_row_bytes = dst_stride * width;
    const auto offset = static_cast<std::underlying_type<Channel>::type>(channel);

    auto ptr = std::make_unique<std::uint8_t[]>(width * height * bytes_per_channel);

    for(decltype(height) i = 0; i < height; i++)
    {
        auto src_base = i * src_row_bytes;
        auto dst_base = i * dst_row_bytes;
        for(decltype(width) j = 0; j < width; j++)
        {
            auto src = &data[src_base + (j * num_of_channels + offset) * bytes_per_channel];
            auto dst = &ptr[dst_base + j * bytes_per_channel];
            std::memcpy(dst, src, bytes_per_channel);
        }
    }
    return ptr;
}
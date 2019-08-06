#pragma once
#include <filesystem>
#include <memory>

namespace common::render
{

class Image final
{
public:
    enum class ColorFormat
    {
        Unknown,
        R,
        RG,
        RGB,
        RGBA
    };

    enum class Channel
    {
        Red,
        Green,
        Blue,
        Alpha
    };

    enum class PixelType
    {
        Unknown,
        UnsignedByte,
        Half,
        Float
    };

    Image();
    ~Image() = default;

    Image(const Image&) = delete;
    Image& operator = (const Image&) = delete;
    Image(Image&&) = delete;
    Image& operator = (Image&&) = delete;

    bool LoadFromFile(const std::filesystem::path& filepath);

    std::size_t GetWidth() const { return width; }
    std::size_t GetHeight() const { return height; }
    ColorFormat GetColorFormat() const { return color_format; }
    PixelType GetPixelType() const { return pixel_type; }
    const std::uint8_t* GetData() const { return data.get(); }
    std::unique_ptr<std::uint8_t[]> ExtractChannel(Channel channel) const;

private:
    bool LoadFromExrFile(const std::filesystem::path& filepath);
    void Release();

private:
    std::size_t width, height;
    ColorFormat color_format;
    PixelType pixel_type;
    std::unique_ptr<std::uint8_t[]> data;
};

}   // namespace common::render
#pragma once
#include <filesystem>   // std::tr2::sys::path etc.
#include <memory>

class PngLoader final
{
public:
    enum class ColorFormat
    {
        Unknown,
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

public:
    PngLoader();
    ~PngLoader() = default;

    PngLoader(const PngLoader&) = delete;
    PngLoader& operator = (const PngLoader&) = delete;
    PngLoader(PngLoader&&) = delete;
    PngLoader& operator = (PngLoader&&) = delete;

    bool Load(const std::filesystem::path& filepath, bool allows_vertical_flip = false);
    void Release();

    std::uint32_t GetWidth() const { return width ; }
    std::uint32_t GetHeight() const { return height; }
    std::uint32_t GetBitDepth() const { return depth; }
    ColorFormat GetColorFormat() const { return format; }
    const std::uint8_t* GetData() const { return data.get(); }

    // Extract any channel from an image.
    std::unique_ptr<std::uint8_t[]> ExtractChannel(Channel channel) const;

private:
    std::uint32_t width;
    std::uint32_t height;
    std::uint32_t depth;
    ColorFormat format;
    std::unique_ptr<std::uint8_t[]> data;
};
#pragma once
#include <filesystem>
#include <memory>

class ExrLoader final
{
public:
    enum class ColorFormat
    {
        Unknown,
        RGB,
        RGBA
    };

    enum class PixelType
    {
        Unknown,
        Half,
        Float
    };

    ExrLoader();
    ~ExrLoader() = default;

    ExrLoader(const ExrLoader&) = delete;
    ExrLoader& operator = (const ExrLoader&) = delete;
    ExrLoader(ExrLoader&&) = delete;
    ExrLoader& operator = (ExrLoader&&) = delete;

    bool Load(const std::filesystem::path& filepath);
    void Release();

    std::size_t GetWidth() const { return width; }
    std::size_t GetHeight() const { return height; }
    ColorFormat GetColorFormat() const { return color_format; }
    PixelType GetPixelType() const { return pixel_type; }
    const std::uint8_t* GetData() const { return data.get(); }

private:
    std::size_t width, height;
    ColorFormat color_format;
    PixelType pixel_type;
    std::unique_ptr<std::uint8_t[]> data;
};
#include <cassert>
#include <iostream>
#include <fstream>
#include <png.h>
#include "png_loader.h"

namespace sys = std::tr2::sys;

static void read_callback(png_structp png_ptr, png_bytep out, png_size_t count)
{
    png_voidp io_ptr = png_get_io_ptr(png_ptr);
    if(io_ptr == nullptr)
        return;
    auto& ifs = *reinterpret_cast<std::ifstream*>(io_ptr);
    ifs.read(reinterpret_cast<char*>(out), count);
}

PngLoader::PngLoader()
    : width(), height(), depth(), format(ColorFormat::Unknown), data()
{
}

bool PngLoader::Load(const sys::path& filepath, bool allows_vertical_flip)
{
    Release();

    std::ifstream ifs(filepath.string(), std::ios::in | std::ios::binary);
    if(ifs.fail()){
        return false;
    }
    // 
    unsigned char sig[8];
    ifs.read(reinterpret_cast<char*>(sig), 8);
    if(png_sig_cmp(sig, 0, 8) != 0){
        return false;
    }
    //
    auto png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if(png_ptr == nullptr){
        return false;
    }
    //
    auto info_ptr = png_create_info_struct(png_ptr);
    if(info_ptr == nullptr){
        png_destroy_read_struct(&png_ptr, nullptr, nullptr);
        return false;
    }
    //
    if(setjmp(png_jmpbuf(png_ptr))){
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        return false;
    }

    png_set_read_fn(png_ptr, reinterpret_cast<png_voidp>(&ifs), read_callback);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);

    //png_uint_32 width, height;
    //int depth;
    int color_type;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, reinterpret_cast<int*>(&depth), &color_type, nullptr, nullptr, nullptr);

    //ColorFormat format;
    switch(color_type){
    case PNG_COLOR_TYPE_GRAY:
        png_set_gray_to_rgb(png_ptr);
        png_read_update_info(png_ptr, info_ptr);
        format = ColorFormat::RGB;
        break;
    case PNG_COLOR_TYPE_PALETTE:
        png_set_palette_to_rgb(png_ptr);
        png_read_update_info(png_ptr, info_ptr);
        png_get_IHDR(png_ptr, info_ptr, &width, &height, reinterpret_cast<int*>(&depth), &color_type, nullptr, nullptr, nullptr);
        format = (color_type == PNG_COLOR_TYPE_RGB) ? ColorFormat::RGB : ColorFormat::RGBA;
        break;
    case PNG_COLOR_TYPE_RGB:
        format = ColorFormat::RGB;
        break;
    case PNG_COLOR_TYPE_RGB_ALPHA:
        format = ColorFormat::RGBA;
        break;
    case PNG_COLOR_TYPE_GRAY_ALPHA:
        png_set_gray_to_rgb(png_ptr);
        png_read_update_info(png_ptr, info_ptr);
        format = ColorFormat::RGBA;
        break;
    default:
        assert(!"unknown color type.");
        break;
    }

    auto row_bytes = png_get_rowbytes(png_ptr, info_ptr);
    auto row_ptrs = std::make_unique<png_bytep[]>(height);
    data = std::make_unique<std::uint8_t[]>(row_bytes * height);

    if(allows_vertical_flip){
        for(png_uint_32 i = 0; i < height; i++){
            row_ptrs[height - 1 - i] = &data[i * row_bytes];  // Y 座標の反転
        }
    }
    else{
        for(png_uint_32 i = 0; i < height; i++){
            row_ptrs[i] = &data[i * row_bytes];
        }
    }

    png_read_image(png_ptr, row_ptrs.get());
    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);

    return true;
}

void PngLoader::Release()
{
    data.reset();
    format = ColorFormat::Unknown;
    depth = 0;
    width = height = 0;
}

std::unique_ptr<std::uint8_t[]> PngLoader::ExtractChannel(Channel channel) const
{
    if(!data || (format == ColorFormat::Unknown))
        return nullptr;
    if((channel == Channel::Alpha) && (format != ColorFormat::RGBA))
        return nullptr;

    const auto stride = (format == ColorFormat::RGB) ? 3 : 4;
    const auto offset = static_cast<typename std::underlying_type<Channel>::type>(channel);
    const auto row_bytes = stride * width;

    auto ch = std::make_unique<std::uint8_t[]>(width * height);
    for(auto i = 0; i < height; i++){
        auto src_base = i * row_bytes;
        auto dst_base = i * width;
        for(auto j = 0; j < width; j++){
            ch[dst_base + j] = data[src_base + j * stride + offset];
        }
    }
    return ch;
}
#include <iostream>
#include <unordered_map>
#include "debug_utils.h"

void dump_framebuffer_info(GLenum attachment)
{
    static const std::unordered_map<GLint, std::string> _object_type =
    {
        {GL_NONE, "GL_NONE"},
        {GL_FRAMEBUFFER_DEFAULT, "GL_FRAMEBUFFER_DEFAULT"},
        {GL_TEXTURE, "GL_TEXTURE"},
        {GL_RENDERBUFFER, "GL_RENDERBUFFER"}
    };
    static const std::unordered_map<GLint, std::string> _component_type =
    {
        {GL_FLOAT, "GL_FLOAT"},
        {GL_INT, "GL_INT"},
        {GL_UNSIGNED_INT, "GL_UNSIGNED_INT"},
        {GL_SIGNED_NORMALIZED, "GL_SIGNED_NORMALIZED"},
        {GL_UNSIGNED_NORMALIZED, "GL_UNSIGNED_NORMALIZED"},
        {GL_NONE, "GL_NONE"}
    };
    static const std::unordered_map<GLint, std::string> _color_encoding =
    {
        {GL_LINEAR, "GL_LINEAR"},
        {GL_SRGB, "GL_SRGB"}
    };
    static const std::unordered_map<GLint, std::string> _layered =
    {
        {GL_TRUE, "GL_TRUE"},
        {GL_FALSE, "GL_FALSE"}
    };

    auto stringify = [](const auto& umap, GLint key)
    {
        auto it = umap.find(key);
        return (it != umap.cend()) ? it->second : "Unknown";
    };

    GLint object_type = 0;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachment, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &object_type);
    std::cout << "object_type: " << stringify(_object_type, object_type) << std::endl;

    if(object_type == GL_NONE)
    {
    }
    else
    {
        GLint r_size = 0;
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachment, GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE, &r_size);
        GLint g_size = 0;
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachment, GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE, &g_size);
        GLint b_size = 0;
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachment, GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE, &b_size);
        GLint a_size = 0;
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachment, GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE, &a_size);
        GLint d_size = 0;
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachment, GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &d_size);
        GLint s_size = 0;
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachment, GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &s_size);
        std::cout << "r_size: " << r_size << " g_size: " << g_size << " b_size: " << b_size << " a_size: " << a_size << " d_size: " << d_size << " s_size: " << s_size << std::endl;

        GLint component_type = 0;
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachment, GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE, &component_type);
        std::cout << "component_type: " << stringify(_component_type, component_type) << std::endl;

        GLint color_encoding = 0;
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachment, GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING, &color_encoding);
        std::cout << "color_encoding: " << stringify(_color_encoding, color_encoding) << std::endl;

        if(object_type == GL_RENDERBUFFER)
        {
            GLint object_name = 0;
            glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachment, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &object_name);
            std::cout << "object_name: " << object_name << std::endl;
        }
        else if(object_type == GL_TEXTURE)
        {
            GLint object_name = 0;
            glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachment, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &object_name);
            std::cout << "object_name: " << object_name << std::endl;

            GLint texture_level = 0;
            glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachment, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL, &texture_level);
            std::cout << "texture_level: " << texture_level << std::endl;

            GLint texture_cube_map_face = 0;
            glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachment, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE, &texture_cube_map_face);
            std::cout << "texture_cube_map_face: " << texture_cube_map_face << std::endl;

            GLint layered = 0;
            glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachment, GL_FRAMEBUFFER_ATTACHMENT_LAYERED, &layered);
            std::cout << "layered: " << stringify(_layered, layered) << std::endl;

            GLint texture_layer = 0;
            glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachment, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER, &texture_layer);
            std::cout << "texture_layer: " << texture_layer << std::endl;
        }
        else if(object_type == GL_FRAMEBUFFER_DEFAULT)
        {
        }
    }
}

void dump_default_framebuffer_info()
{
    static const std::unordered_map<GLenum, std::string> _default_framebuffer =
    {
        {GL_FRONT_LEFT, "GL_FRONT_LEFT"},
        {GL_FRONT_RIGHT, "GL_FRONT_RIGHT"},
        {GL_BACK_LEFT, "GL_BACK_LEFT"},
        {GL_BACK_RIGHT, "GL_BACK_RIGHT"}
    };

    GLint prev_fbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prev_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    for(auto& pair : _default_framebuffer)
    {
        std::cout << "*** default_framebuffer: " << pair.second << std::endl;
        dump_framebuffer_info(pair.first);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, prev_fbo);
}
#include <iomanip>
#include <sstream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <hasenpfote/fp_conversion.h>
#if defined(USE_IMGUI)
#include "../../external/imgui/imgui.h"
#endif
#include "../../common/logger.h"
#include "mywindow.h"

MyWindow::MyWindow()
{
    LOG_D(__func__);
}

MyWindow::~MyWindow()
{
    LOG_D(__func__);
}

void MyWindow::Setup()
{
    //
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    glDisable(GL_BLEND);
    //glDisable(GL_LIGHTING);

    glDisable(GL_MULTISAMPLE);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0);

    //
    auto window = GetWindow();
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    //
    {
        auto& camera = System::GetMutableInstance().GetCamera();

        auto& vp = camera.viewport();
        vp.origin() = glm::vec2(0, 0);
        vp.size() = glm::vec2(width, height);
        vp.depth_range() = glm::vec2(1.0f, 10000.0f);

        camera.Update(0.0);
    }
    // load shader.
    {
        std::filesystem::path dirpath("assets/shaders");
        auto& rm = System::GetMutableInstance().GetResourceManager();
        rm.AddResourcesFromDirectory<Program>(dirpath, false);
    }
    // load texture.
    {
        std::filesystem::path dirpath("assets/textures");
        auto& rm = System::GetMutableInstance().GetResourceManager();
        rm.AddResourcesFromDirectory<Texture>(dirpath, false);
    }
    // generate font.
    {
        std::filesystem::path fontpath = "../common/assets/fonts/test.fnt";
        auto font = std::make_shared<Font>(fontpath);
        text = std::make_unique<SDFText>(font, std::make_shared<SDFTextRenderer>());
        text->SetSmoothness(1.0f);
    }
    //
    {
        std::filesystem::path texpath;
        //GLuint texture;
        auto& rm = System::GetConstInstance().GetResourceManager();

        texpath = "assets/textures/street_lamp_1k.exr";
        //texture = rm.GetResource<Texture>(texpath.string())->GetTexture();
        selectable_textures.emplace_back(texpath);


        texpath = "assets/textures/satara_night_no_lamps_1k.exr";
        //texture = rm.GetResource<Texture>(texpath.string())->GetTexture();
        selectable_textures.emplace_back(texpath);

        selected_texture_index = 0;
    }

    fs_quad = std::make_unique<FullScreenQuad>();

    glGenSamplers(1, &nearest_sampler);
    glSamplerParameteri(nearest_sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(nearest_sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(nearest_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(nearest_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(nearest_sampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(nearest_sampler, GL_TEXTURE_LOD_BIAS, 0.0f);

    glGenSamplers(1, &linear_sampler);
    glSamplerParameteri(linear_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(linear_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(linear_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(linear_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(linear_sampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(linear_sampler, GL_TEXTURE_LOD_BIAS, 0.0f);

    RecreateResources(width, height);

    auto& rm = System::GetMutableInstance().GetResourceManager();

    pipeline_fullscreen_quad = std::make_unique<ProgramPipeline>(
        ProgramPipeline::ProgramPtrSet({
            rm.GetResource<Program>("assets/shaders/fullscreen_quad.vs"),
            rm.GetResource<Program>("assets/shaders/fullscreen_quad.fs")})
            );

    pipeline_log_luminance = std::make_unique<ProgramPipeline>(
        ProgramPipeline::ProgramPtrSet({
            rm.GetResource<Program>("assets/shaders/log_luminance.vs"),
            rm.GetResource<Program>("assets/shaders/log_luminance.fs")})
            );

    pipeline_high_luminance_region_extraction = std::make_unique<ProgramPipeline>(
        ProgramPipeline::ProgramPtrSet({
            rm.GetResource<Program>("assets/shaders/high_luminance_region_extraction.vs"),
            rm.GetResource<Program>("assets/shaders/high_luminance_region_extraction.fs")})
            );

    pipeline_downsampling_2x2 = std::make_unique<ProgramPipeline>(
        ProgramPipeline::ProgramPtrSet({
            rm.GetResource<Program>("assets/shaders/downsampling.vs"),
            rm.GetResource<Program>("assets/shaders/downsampling_2x2.fs")})
            );

    pipeline_downsampling_4x4 = std::make_unique<ProgramPipeline>(
        ProgramPipeline::ProgramPtrSet({
            rm.GetResource<Program>("assets/shaders/downsampling.vs"),
            rm.GetResource<Program>("assets/shaders/downsampling_4x4.fs")})
            );

    pipeline_kawase_blur = std::make_unique<ProgramPipeline>(
        ProgramPipeline::ProgramPtrSet({
            rm.GetResource<Program>("assets/shaders/kawase_blur.vs"),
            rm.GetResource<Program>("assets/shaders/kawase_blur.fs")})
            );

    pipeline_streak = std::make_unique<ProgramPipeline>(
        ProgramPipeline::ProgramPtrSet({
            rm.GetResource<Program>("assets/shaders/streak.vs"),
            rm.GetResource<Program>("assets/shaders/streak.fs")})
            );

    pipeline_tonemapping = std::make_unique<ProgramPipeline>(
        ProgramPipeline::ProgramPtrSet({
            rm.GetResource<Program>("assets/shaders/tonemapping.vs"),
            rm.GetResource<Program>("assets/shaders/tonemapping.fs")})
            );

    pipeline_apply = std::make_unique<ProgramPipeline>(
        ProgramPipeline::ProgramPtrSet({
            rm.GetResource<Program>("assets/shaders/apply.vs"),
            rm.GetResource<Program>("assets/shaders/apply.fs")})
            );

    exposure = 2.0f;
    lum_soft_threshold = 0.5f;
    lum_hard_threshold = 1.0f;
    average_luminance = 0.0f;
    is_bloom_enabled = false;
    is_streak_enabled = false;
    is_debug_enabled = false;
    is_tonemapping_enabled = true;
    is_auto_exposure_enabled = true;
}

void MyWindow::Cleanup()
{
    if(glIsSampler(nearest_sampler))
        glDeleteSamplers(1, &nearest_sampler);
    if(glIsSampler(linear_sampler))
        glDeleteSamplers(1, &linear_sampler);
}

void MyWindow::OnKey(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Window::OnKey(window, key, scancode, action, mods);

    System::GetMutableInstance().GetCamera().OnKey(key, scancode, action, mods);
}

void MyWindow::OnMouseMove(GLFWwindow* window, double xpos, double ypos)
{
    System::GetMutableInstance().GetCamera().OnMouseMove(xpos, ypos);
}

void MyWindow::OnMouseButton(GLFWwindow* window, int button, int action, int mods)
{
    System::GetMutableInstance().GetCamera().OnMouseButton(button, action, mods);
}

void MyWindow::OnMouseWheel(GLFWwindow* window, double xoffset, double yoffset)
{
    System::GetMutableInstance().GetCamera().OnMouseWheel(xoffset, yoffset);
}

void MyWindow::OnResizeFramebuffer(GLFWwindow* window, int width, int height)
{
    if(HasIconified())
        return;
    // re-create
    RecreateResources(width, height);
}

void MyWindow::OnResizeWindow(GLFWwindow* window, int width, int height)
{
    if(HasIconified())
        return;

    auto& camera = System::GetMutableInstance().GetCamera();
    camera.viewport().size() = glm::vec2(width, height);
}

void MyWindow::OnUpdate(double dt)
{
    System::GetMutableInstance().GetCamera().Update(dt);

    // Determine an exposure value automatically.
    if(is_auto_exposure_enabled)
    {
        const float key_value = 0.18f;
        auto target_exposure = key_value / std::max(average_luminance, 0.00001f);
        exposure += (target_exposure - exposure) * 0.1f;
    }
}

void MyWindow::OnRender()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto& camera = System::GetConstInstance().GetCamera();

    auto& vp = camera.viewport();
    auto& resolution = camera.viewport().size();
    const auto width = static_cast<int>(resolution.x);
    const auto height = static_cast<int>(resolution.y);
    glViewport(0, 0, width, height);
    //

    // 1) Render scene to texture.
    auto& rm = System::GetConstInstance().GetResourceManager();
    auto& filepath = selectable_textures[selected_texture_index];
    auto texture = rm.GetResource<Texture>(filepath.string())->GetTexture();

    scene_rt->Bind();
    DrawFullScreenQuad(texture);
    scene_rt->Unbind();

    // 2) Extract high luminance region.
    PassHighLuminanceRegionExtraction(scene_rt.get(), high_luminance_region_rt.get());

    FrameBuffer* output_rt;
    if(is_debug_enabled)
    {
        GLfloat clear_color[] = { 0.0f, 0.0, 0.0f, 1.0f };
        glClearTexImage(debug_rt->GetColorTexture(), 0, GL_RGBA, GL_FLOAT, &clear_color);
        output_rt = debug_rt.get();
    }
    else
    {
        output_rt = scene_rt.get();
    }

    // 3) Apply Bloom effect.
    if(is_bloom_enabled)
        PassBloom(high_luminance_region_rt.get(), output_rt);

    // 4) Apply Streak effect.
    if(is_streak_enabled)
        PassStreak(high_luminance_region_rt.get(), output_rt);

    // 5) Mesure an average luminance of the scene for automatic exposure.
    average_luminance = ComputeAverageLuminance(output_rt);

    // 6) Render HDR to LDR using tone mapping.
    if(is_tonemapping_enabled)
    {
        PassTonemapping(output_rt);
    }
    else
    {
        glEnable(GL_FRAMEBUFFER_SRGB);
        PassApply(output_rt);   // Render HDR to LDR directly.
        glDisable(GL_FRAMEBUFFER_SRGB);
    }

    // Display debug information.
    {
        std::ostringstream oss;
        //std::streamsize ss = std::cout.precision();

        oss << std::fixed << std::setprecision(2);

        oss << "FPS:UPS=" << GetFPS() << ":" << GetUPS();
        oss << "\n";

        text->BeginRendering();
        {
            text->DrawString(std::move(oss.str()), 0.0f, 0.0f, 0.5f);
        }
        text->EndRendering();

        //oss.str("");
        //oss.clear(std::stringstream::goodbit);
    }
}

void MyWindow::RecreateResources(int width, int height)
{
    auto recreate_fb = [](const std::string& name, GLsizei levels, GLenum internal_format, GLsizei width, GLsizei height)
    {
        auto& rm = System::GetMutableInstance().GetResourceManager();

        rm.RemoveResource<Texture>(name);

        if(levels == 0)
            levels = Texture::CalcNumOfMipmapLevels(width, height);

        auto p = std::make_unique<Texture>(levels, internal_format, width, height);
        auto texture = p->GetTexture();

        rm.AddResource<Texture>(name, std::move(p));

        return std::make_unique<FrameBuffer>(texture, 0, 0);
    };

    // for scene.
    {
        const auto name = std::string("scene_rt_color");
        scene_rt = recreate_fb(name, 1, GL_RGBA16F, width, height);
    }
    // for luminance.
    {
        const auto name = std::string("luminance_rt_color");
        luminance_rt = recreate_fb(name, 0, GL_R16F, width, height);
    }
    // for debug.
    {
        const auto name = std::string("debug_rt_color");
        debug_rt = recreate_fb(name, 1, GL_RGBA16F, width, height);
    }

    // for high luminance region.
    {
        auto ds_width = width / 4;
        auto ds_height = height / 4;

        const auto name = std::string("high_luminance_region_rt_color");
        high_luminance_region_rt = recreate_fb(name, 1, GL_RGBA16F, ds_width, ds_height);
    }
    // for bloom.
    {
        std::stringstream ss;

        auto ds_width = width / 4;
        auto ds_height = height / 4;

        bloom_rts.clear();

        for(auto i = 0; i < 5; i++)
        {
            ss << "bloom_rts_color_" << i << "_" << 0;
            auto bloom_rt_0 = recreate_fb(ss.str(), 1, GL_RGBA16F, ds_width, ds_height);
            ss.str("");
            ss.clear(std::stringstream::goodbit);

            ss << "bloom_rts_color_" << i << "_" << 1;
            auto bloom_rt_1 = recreate_fb(ss.str(), 1, GL_RGBA16F, ds_width, ds_height);
            ss.str("");
            ss.clear(std::stringstream::goodbit);

            ss << "bloom_rts_color_" << i << "_" << 2;
            auto bloom_rt_2 = recreate_fb(ss.str(), 1, GL_RGBA16F, ds_width, ds_height);
            ss.str("");
            ss.clear(std::stringstream::goodbit);

            bloom_rts.push_back({
                std::move(bloom_rt_0),
                std::move(bloom_rt_1),
                std::move(bloom_rt_2)
                });

            ds_width /= 2;
            ds_height /= 2;
        }
    }
    // for streak.
    {
        std::stringstream ss;

        auto ds_width = width / 4;
        auto ds_height = height / 4;

        ss << "streak_rt_color_" << 0;
        auto streak_rt_0 = recreate_fb(ss.str(), 1, GL_RGBA16F, ds_width, ds_height);
        ss.str("");
        ss.clear(std::stringstream::goodbit);

        ss << "streak_rt_color_" << 1;
        auto streak_rt_1 = recreate_fb(ss.str(), 1, GL_RGBA16F, ds_width, ds_height);
        ss.str("");
        ss.clear(std::stringstream::goodbit);

        ss << "streak_rt_color_" << 2;
        auto streak_rt_2 = recreate_fb(ss.str(), 1, GL_RGBA16F, ds_width, ds_height);
        ss.str("");
        ss.clear(std::stringstream::goodbit);

        streak_rts = { std::move(streak_rt_0), std::move(streak_rt_1), std::move(streak_rt_2) };
    }
}

void MyWindow::DrawFullScreenQuad(GLuint texture)
{
    pipeline_fullscreen_quad->GetPipelineUniform().Set("u_tex0", 0);

    pipeline_fullscreen_quad->Bind();
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindSampler(0, linear_sampler);

        fs_quad->Draw();

        glBindSampler(0, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    pipeline_fullscreen_quad->Unbind();
}

float MyWindow::ComputeAverageLuminance(FrameBuffer* input)
{
    PassLogLuminance(input, luminance_rt.get());
    luminance_rt->UpdateAllMipmapLevels();

    float result = 0.0f;

    auto texture = luminance_rt->GetColorTexture();
    if(!glIsTexture(texture))
        return result;

    GLint prev_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &prev_texture);

    glBindTexture(GL_TEXTURE_2D, texture);

    GLint max_level;
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, &max_level);

    GLushort pixel;
    glGetTexImage(GL_TEXTURE_2D, max_level, GL_RED, GL_HALF_FLOAT, &pixel);
    result = hasenpfote::ConvertHalfToSingle(pixel);
    result = std::exp(result);

    glBindTexture(GL_TEXTURE_2D, prev_texture);

    return result;
}

void MyWindow::PassLogLuminance(FrameBuffer* input, FrameBuffer* output)
{
    output->Bind();
    {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        auto& uniform = pipeline_log_luminance->GetPipelineUniform();
        uniform.Set("u_tex0", 0);
        uniform.Set("u_pixel_size", glm::vec2(1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3])));

        pipeline_log_luminance->Bind();
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
            glBindSampler(0, linear_sampler);

            fs_quad->Draw();

            glBindSampler(0, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        pipeline_log_luminance->Unbind();
    }
    output->Unbind();
}

void MyWindow::PassHighLuminanceRegionExtraction(FrameBuffer* input, FrameBuffer* output)
{
    output->Bind();
    {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        auto& uniform = pipeline_high_luminance_region_extraction->GetPipelineUniform();
        uniform.Set("u_tex0", 0);
        uniform.Set("u_pixel_size", glm::vec2(1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3])));
        uniform.Set("u_exposure", exposure);
        uniform.Set("u_threshold", lum_hard_threshold);
        uniform.Set("u_soft_threshold", lum_soft_threshold);

        pipeline_high_luminance_region_extraction->Bind();
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
            glBindSampler(0, linear_sampler);

            fs_quad->Draw();

            glBindSampler(0, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        pipeline_high_luminance_region_extraction->Unbind();
    }
    output->Unbind();
}

void MyWindow::PassDownsampling2x2(FrameBuffer* input, FrameBuffer* output)
{
    output->Bind();
    {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        auto& uniform = pipeline_downsampling_2x2->GetPipelineUniform();
        uniform.Set("u_tex0", 0);
        uniform.Set("u_pixel_size", glm::vec2(1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3])));

        pipeline_downsampling_2x2->Bind();
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
            glBindSampler(0, linear_sampler);

            fs_quad->Draw();

            glBindSampler(0, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        pipeline_downsampling_2x2->Unbind();
    }
    output->Unbind();
}

void MyWindow::PassDownsampling4x4(FrameBuffer* input, FrameBuffer* output)
{
    output->Bind();
    {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        auto& uniform = pipeline_downsampling_4x4->GetPipelineUniform();
        uniform.Set("u_tex0", 0);
        uniform.Set("u_pixel_size", glm::vec2(1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3])));

        pipeline_downsampling_4x4->Bind();
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
            glBindSampler(0, linear_sampler);

            fs_quad->Draw();

            glBindSampler(0, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        pipeline_downsampling_4x4->Unbind();
    }
    output->Unbind();
}

void MyWindow::PassKawaseBlur(FrameBuffer* input, FrameBuffer* output, int iteration)
{
    output->Bind();
    {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        auto& uniform = pipeline_kawase_blur->GetPipelineUniform();
        uniform.Set("u_tex0", 0);
        uniform.Set("u_pixel_size", glm::vec2(1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3])));
        uniform.Set("u_iteration", static_cast<float>(iteration));

        pipeline_kawase_blur->Bind();
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
            glBindSampler(0, linear_sampler);

            fs_quad->Draw();

            glBindSampler(0, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        pipeline_kawase_blur->Unbind();
    }
    output->Unbind();
}

void MyWindow::PassBloom(FrameBuffer* input, FrameBuffer* output)
{
    // 1) ダウンサンプリング
    {
        auto last_rt = bloom_rts[0][0].get();
        PassApply(input, last_rt);

        auto size = bloom_rts.size();
        for(auto i = 1; i < size; i++)
        {
            auto src_rt = last_rt;
            auto dst_rt = bloom_rts[i][0].get();
            PassDownsampling2x2(src_rt, dst_rt);
            last_rt = dst_rt;
        }
    }
    // 2) 各レベルにピンポンブラー
    {
        std::vector<int> kernel = { 0, 0 };
        const auto num_of_passes = kernel.size();

        for(auto& bloom_rt : bloom_rts)
        {
            auto last_rt = bloom_rt[0].get();
            for(auto i = 0; i < num_of_passes; i++)
            {
                FrameBuffer* src_rt = last_rt;
                FrameBuffer* dst_rt = ((i % 2) == 0) ? bloom_rt[1].get() : bloom_rt[2].get();
                PassKawaseBlur(src_rt, dst_rt, kernel[i]);
                last_rt = dst_rt;
            }
            PassApply(last_rt, bloom_rt[0].get());
        }
    }
    // 3) 各フィルタを合成
    {
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        auto size = bloom_rts.size();

        for(auto i = size - 1; i > i - 1; i--)
        {
            PassApply(bloom_rts[i][0].get(), bloom_rts[i-1][0].get());
        }
        PassApply(bloom_rts[0][0].get(), output);

        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
    }
}

void MyWindow::PassStreak(FrameBuffer* input, FrameBuffer* output)
{
    // 1) 方向毎にピンポンブラー
    {
        const auto num_of_streaks = 4;
        const auto num_of_passes = 3;
        const auto additional_angle = 360.0f / static_cast<float>(num_of_streaks);

        GLfloat clear_color[] = { 0.0f, 0.0, 0.0f, 1.0f };
        glClearTexImage(streak_rts[0]->GetColorTexture(), 0, GL_RGBA, GL_FLOAT, &clear_color);

        float angle = 45.0f;
        for(auto i = 0; i < num_of_streaks; i++)
        {
            auto theta = glm::radians(angle);
            auto dx = std::cos(theta);
            auto dy = std::sin(theta);

            auto last_rt = input;
            for(auto j = 0; j < num_of_passes; j++)
            {
                FrameBuffer* src_rt = last_rt;
                FrameBuffer* dst_rt = ((j % 2) == 0) ? streak_rts[1].get() : streak_rts[2].get();
                PassStreak(src_rt, dst_rt, dx, dy, 0.90f, j);
                last_rt = dst_rt;
            }
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            PassApply(last_rt, streak_rts[0].get());
            glDisable(GL_BLEND);

            angle += additional_angle;
        }
    }

    // 2) 各フィルタを合成
    {
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        PassApply(streak_rts[0].get(), output);
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
    }
}

void MyWindow::PassStreak(FrameBuffer* input, FrameBuffer* output, float dx, float dy, float attenuation, int pass)
{
    output->Bind();
    {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        auto& uniform = pipeline_streak->GetPipelineUniform();
        uniform.Set("u_tex0", 0);
        uniform.Set("u_pixel_size", glm::vec2(1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3])));
        uniform.Set("u_direction", glm::vec2(dx, dy));
        uniform.Set("u_params", glm::vec2(attenuation, static_cast<float>(pass)));

        pipeline_streak->Bind();
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
            glBindSampler(0, linear_sampler);

            fs_quad->Draw();

            glBindSampler(0, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        pipeline_streak->Unbind();
    }
    output->Unbind();
}

void MyWindow::PassTonemapping(FrameBuffer* input, FrameBuffer* output)
{
    if(output != nullptr)
        output->Bind();

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    auto& uniform = pipeline_tonemapping->GetPipelineUniform();
    uniform.Set("u_tex0", 0);
    uniform.Set("u_pixel_size", glm::vec2(1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3])));
    uniform.Set("u_exposure", exposure);

    pipeline_tonemapping->Bind();
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
        glBindSampler(0, linear_sampler);

        fs_quad->Draw();

        glBindSampler(0, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    pipeline_tonemapping->Unbind();

    if(output != nullptr)
        output->Unbind();
}

void MyWindow::PassApply(FrameBuffer* input, FrameBuffer* output)
{
    if(output != nullptr)
        output->Bind();

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    auto& uniform = pipeline_apply->GetPipelineUniform();
    uniform.Set("u_tex0", 0);
    uniform.Set("u_pixel_size", glm::vec2(1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3])));

    pipeline_apply->Bind();
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
        glBindSampler(0, linear_sampler);

        fs_quad->Draw();

        glBindSampler(0, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    pipeline_apply->Unbind();

    if(output != nullptr)
        output->Unbind();
}

void MyWindow::OnGUI()
{
#if defined(USE_IMGUI)
    auto oss2s = [](std::ostream& os)
    {
        return dynamic_cast<std::ostringstream&>(os).str();
    };

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
    ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once);
    ImGui::Begin("config");
    {
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if(ImGui::CollapsingHeader("Basic"))
        {
#if defined(RECORD_STATISTICS)
            auto values_getter = [](void* data, int idx)
            {
                return static_cast<float>((static_cast<double*>(data))[idx]);
            };

            auto& fps_record = GetFPSRecord();
            ImGui::PlotLines(
                "FPS",
                values_getter,
                static_cast<void*>(const_cast<double*>(fps_record.data())),
                fps_record.capacity(),
                fps_record.tail(),
                oss2s(std::ostringstream() << std::fixed << std::setprecision(2) << GetFPS()).c_str(),
                0.0f, 90.0f, ImVec2(0, 40)
            );
            auto& ups_record = GetUPSRecord();
            ImGui::PlotLines(
                "UPS",
                values_getter,
                static_cast<void*>(const_cast<double*>(ups_record.data())),
                ups_record.capacity(),
                ups_record.tail(),
                oss2s(std::ostringstream() << std::fixed << std::setprecision(2) << GetUPS()).c_str(),
                0.0f, 180.0f, ImVec2(0, 40)
            );
#else
            ImGui::Text(oss2s(
                std::ostringstream() << std::fixed << std::setprecision(2) << "UPS: " << GetUPS()).c_str());
            ImGui::Text(oss2s(
                std::ostringstream() << std::fixed << std::setprecision(2) << "FPS: " << GetFPS()).c_str());
#endif
            int width, height;
            glfwGetFramebufferSize(GetWindow(), &width, &height);
            ImGui::Text(oss2s(std::ostringstream() << "Screen size: " << width << "x" << height).c_str());
        }
        ImGui::Separator();
#if 0
        ImGui::SetNextItemOpen(false, ImGuiCond_Once);
        if(ImGui::CollapsingHeader("Camera"))
        {
            auto& camera = System::GetConstInstance().GetCamera();

            auto aov_d = camera.GetAngleOfView(CustomCamera::AngleOfView::Diagonal);
            ImGui::Text(oss2s(
                std::ostringstream()
                << std::fixed
                << std::setprecision(2)
                << "AoV D: " << hasenpfote::math::ConvertRadiansToDegrees(aov_d)
            ).c_str());

            auto aov_h = camera.GetAngleOfView(CustomCamera::AngleOfView::Horizontal);
            ImGui::Text(oss2s(
                std::ostringstream()
                << std::fixed
                << std::setprecision(2)
                << "AoV H: " << hasenpfote::math::ConvertRadiansToDegrees(aov_h)
            ).c_str());

            auto aov_v = camera.GetAngleOfView(CustomCamera::AngleOfView::Vertical);
            ImGui::Text(oss2s(
                std::ostringstream()
                << std::fixed
                << std::setprecision(2)
                << "AoV V: " << hasenpfote::math::ConvertRadiansToDegrees(aov_v)
            ).c_str());

            ImGui::Text(oss2s(std::ostringstream() << "Focal length: " << camera.GetFocalLength()).c_str());
            ImGui::Text(oss2s(std::ostringstream() << "Focal length(35mm): " << camera.Get35mmEquivalentFocalLength()).c_str());
            ImGui::Text(oss2s(std::ostringstream() << "Zoom: x" << camera.GetZoomMagnification()).c_str());
        }
        ImGui::Separator();
#endif
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if(ImGui::CollapsingHeader("App"))
        {
            {
                auto delim = '\0';
                std::ostringstream oss;
                for(const auto& texture : selectable_textures)
                    oss << texture.string() << delim;
                oss << delim;
                ImGui::Combo("textures", &selected_texture_index, oss.str().c_str());
                //oss.str("");
                //oss.clear(std::stringstream::goodbit);
            }
            ImGui::Checkbox("auto_exposure", &is_auto_exposure_enabled);
            if(is_auto_exposure_enabled)
            {
                ImGui::Text(oss2s(std::ostringstream() << "average_luminance: " << average_luminance).c_str());
                ImGui::Text(oss2s(std::ostringstream() << "exposure: " << exposure).c_str());
            }
            else
            {
                ImGui::SliderFloat("exposure", &exposure, 0.0f, 10.0f);
            }
            ImGui::SliderFloat("lum_soft_threshold", &lum_soft_threshold, 0.0f, 1.0f);
            ImGui::SliderFloat("lum_hard_threshold", &lum_hard_threshold, 0.0f, 10.0f);
            ImGui::Checkbox("bloom", &is_bloom_enabled);
            ImGui::Checkbox("streak", &is_streak_enabled);
            ImGui::Checkbox("debug", &is_debug_enabled);
            ImGui::Checkbox("tonemapping", &is_tonemapping_enabled);
        }
    }
    ImGui::End();
#endif
}
/*!
* @file text.h
* @brief Text and related classes.
* @author Hasenpfote
* @date 2016/09/26
*/
#pragma once
#include "font.h"

namespace text{

/*!
 * @class ITextRenderer
 * @brief Abstract bitmap font renderer class.
 */
class ITextRenderer
{
    friend class Text;
public:
    using Rect = std::tuple<float, float, float, float>;    // left, top, right, bottom

public:
    ITextRenderer() = default;
    virtual ~ITextRenderer() = default;

    ITextRenderer(const ITextRenderer&) = delete;
    ITextRenderer& operator = (const ITextRenderer&) = delete;
    ITextRenderer(ITextRenderer&&) = delete;
    ITextRenderer& operator = (ITextRenderer&&) = delete;

protected:
    virtual void BeginRendering(GLuint texture) = 0;
    virtual void EndRendering() = 0;
    virtual void Render() = 0;

    virtual void SetOrthographicProjectionMatrix(const GLfloat* m) = 0;
    virtual void SetColor(const GLfloat* color) = 0;

    virtual std::size_t GetMaxBufferLength() = 0;
    virtual bool IsBufferEmpty() = 0;
    virtual void SetToBuffer(const Rect& bounds, const Rect& texcoord_bounds, std::uint16_t page) = 0;
    virtual void ClearBuffer() = 0;
};

/*!
 * @class Text
 * @brief
 */
class Text
{
public:
    Text(const std::shared_ptr<Font>& font, const std::shared_ptr<ITextRenderer>& renderer);
    virtual ~Text() = default;

    Text(const Text&) = delete;
    Text& operator = (const Text&) = delete;
    Text(Text&&) = delete;
    Text& operator = (Text&&) = delete;

    void BeginRendering();
    void EndRendering();
    void DrawString(const std::string& string, float x, float y, float scale = 1.0f);
    void DrawString(const std::u16string& string, float x, float y, float scale = 1.0f);
    void SetColor(const GLfloat* color);

    const Font& GetFont() const;

protected:
    ITextRenderer* GetRenderer();

private:
    std::shared_ptr<const Font> font;
    std::shared_ptr<ITextRenderer> renderer;
};


}
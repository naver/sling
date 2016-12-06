/*
 * Copyright (C) 2016 Naver Corp. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "GraphicsContext.h"

#if USE(CAIRO)

#include "AffineTransform.h"
#include "CairoUtilities.h"
#include "DrawErrorUnderline.h"
#include "FloatConversion.h"
#include "FloatRect.h"
#include "FloatRoundedRect.h"
#include "Font.h"
#include "GraphicsContextPlatformPrivateCairo.h"
#include "ImageBuffer.h"
#include "IntRect.h"
#include "NotImplemented.h"
#include "Path.h"
#include "Pattern.h"
#include "PlatformContextCairo.h"
#include "PlatformPathCairo.h"
#include "RefPtrCairo.h"
#include "ShadowBlur.h"
#include "TransformationMatrix.h"
#include "WebGraphicsUtilities.h"
#include <GLES2/gl2.h>
#include <cairo.h>
#include <math.h>
#include <stdio.h>
#include <wtf/MathExtras.h>


using WebGraphics::WebGraphicsContext;

namespace WebCore {

extern cairo_user_data_key_t graphicsContextUserDataKey;
extern void graphicsContextDestroyUserData(void* userData);

class GraphicsContextPrivateData : public GraphicsContextPlatformPrivate {
public:
    GraphicsContextPrivateData(GraphicsContextPlatformPrivate* p, bool retain)
        : GraphicsContextPlatformPrivate(p->platformContext)
        , m_private(p)
        , m_context(nullptr)
        , m_retain(retain)
    {
        cairo_t* cr = p->platformContext->cr();
        m_context = reinterpret_cast<WebGraphicsContext*>(cairo_get_user_data(cr, &graphicsContextUserDataKey));
        if (!m_context) {
            cairo_surface_t* surface = cairo_get_target(cr);
            m_context = new WebGraphicsContext(surface);
            cairo_set_user_data(cr, &graphicsContextUserDataKey, m_context, graphicsContextDestroyUserData);
            p->platformContext->save();
        }

        if (m_retain) {
            m_context->flush();
            m_context->save();
        }
    }
    ~GraphicsContextPrivateData()
    {
        if (m_retain) {
            m_context->restore();
            m_context->flush();
        }

        glFinish();
    }

    WebGraphicsContext* context() { return m_context; }

private:
    std::unique_ptr<GraphicsContextPlatformPrivate> m_private;
    WebGraphicsContext* m_context;
    bool m_retain;
};

static WebGraphicsContext* context(GraphicsContextPlatformPrivate* data)
{
    return static_cast<GraphicsContextPrivateData*>(data)->context();
}

static WebGraphicsContext* context(GraphicsContext* context)
{
    return static_cast<WebGraphicsContext*>(cairo_get_user_data(context->platformContext()->cr(), &graphicsContextUserDataKey));
}

GraphicsContext::GraphicsContext(cairo_t* cr)
    : m_transparencyCount(0)
{
    m_data = new GraphicsContextPrivateData(new GraphicsContextPlatformPrivateToplevel(new PlatformContextCairo(cr)), false);
}

void GraphicsContext::platformInit(PlatformContextCairo* platformContext)
{
    m_data = new GraphicsContextPrivateData(new GraphicsContextPlatformPrivate(platformContext), true);
    if (platformContext)
        m_data->syncContext(platformContext->cr());
}

void GraphicsContext::platformDestroy()
{
    delete m_data;
}

AffineTransform GraphicsContext::getCTM(IncludeDeviceScale) const
{
    return toWebCore(context(m_data)->getCTM());
}

PlatformContextCairo* GraphicsContext::platformContext() const
{
    return m_data->platformContext;
}

void GraphicsContext::savePlatformState()
{
    context(m_data)->save();
}

void GraphicsContext::restorePlatformState()
{
    context(m_data)->restore();
}

// Draws a filled rectangle with a stroked border.
void GraphicsContext::drawRect(const FloatRect& rect, float borderThickness)
{
    if (paintingDisabled())
        return;

    ASSERT(!rect.isEmpty());

    context(m_data)->drawRect(toWebGraphics(rect), borderThickness, toWebGraphics(state()));
}

// This is only used to draw borders, so we should not draw shadows.
void GraphicsContext::drawLine(const FloatPoint& point1, const FloatPoint& point2)
{
    if (paintingDisabled())
        return;

    if (strokeStyle() == NoStroke)
        return;

    const Color& strokeColor = this->strokeColor();
    float thickness = strokeThickness();
    bool isVerticalLine = (point1.x() + thickness == point2.x());
    float strokeWidth = isVerticalLine ? point2.y() - point1.y() : point2.x() - point1.x();
    if (!thickness || !strokeWidth)
        return;

    context(m_data)->drawLine(toWebGraphics(point1), toWebGraphics(point2), toWebGraphics(state()));
}

// This method is only used to draw the little circles used in lists.
void GraphicsContext::drawEllipse(const FloatRect& rect)
{
    if (paintingDisabled())
        return;

    context(m_data)->drawEllipse(toWebGraphics(rect), toWebGraphics(state()));
}

void GraphicsContext::fillPath(const Path& path)
{
    if (paintingDisabled() || path.isEmpty())
        return;

    context(m_data)->fillPath(toWebGraphics(path), toWebGraphics(state()));
}

void GraphicsContext::strokePath(const Path& path)
{
    if (paintingDisabled() || path.isEmpty())
        return;

    context(m_data)->strokePath(toWebGraphics(path), toWebGraphics(state()));
}

void GraphicsContext::fillRect(const FloatRect& rect)
{
    if (paintingDisabled())
        return;

    context(m_data)->fillRect(toWebGraphics(rect), toWebGraphics(state()));
}

void GraphicsContext::fillRect(const FloatRect& rect, const Color& color)
{
    if (paintingDisabled())
        return;

    context(m_data)->fillRect(toWebGraphics(rect), toWebGraphics(color), toWebGraphics(state()));
}

void GraphicsContext::clip(const FloatRect& rect)
{
    if (paintingDisabled())
        return;

    context(m_data)->clip(toWebGraphics(rect));
    m_data->clip(rect);
}

void GraphicsContext::clipPath(const Path& path, WindRule clipRule)
{
    if (paintingDisabled())
        return;

    context(m_data)->clipPath(toWebGraphics(path), toWebGraphics(state()));
    m_data->clip(path);
}

void GraphicsContext::clipToImageBuffer(ImageBuffer& buffer, const FloatRect& destRect)
{
    if (paintingDisabled())
        return;

    RefPtr<Image> image = buffer.copyImage(DontCopyBackingStore);
    RefPtr<cairo_surface_t> surface = image->nativeImageForCurrentFrame();
    if (surface)
        platformContext()->pushImageMask(surface.get(), destRect);
}

IntRect GraphicsContext::clipBounds() const
{
    return toWebCore(context(m_data)->clipBounds());
}

void GraphicsContext::drawFocusRing(const Path& path, float width, float offset, const Color& color)
{
    context(m_data)->drawFocusRing(toWebGraphics(path), width, offset, toWebGraphics(color), toWebGraphics(state()));
}

void GraphicsContext::drawFocusRing(const Vector<FloatRect>& rects, float width, float offset, const Color& color)
{
    if (paintingDisabled())
        return;

    context(m_data)->drawFocusRing(toWebGraphics(rects), width, offset, toWebGraphics(color), toWebGraphics(state()));
}

void GraphicsContext::drawLineForText(const FloatPoint& origin, float width, bool printing, bool doubleLines, StrokeStyle)
{
    DashArray widths;
    widths.append(width);
    widths.append(0);
    drawLinesForText(origin, widths, printing, doubleLines);
}

void GraphicsContext::drawLinesForText(const FloatPoint& point, const DashArray& widths, bool printing, bool doubleUnderlines, StrokeStyle)
{
    if (paintingDisabled())
        return;

    if (widths.size() <= 0)
        return;

    Color localStrokeColor(strokeColor());

    FloatRect bounds = computeLineBoundsAndAntialiasingModeForText(point, widths.last(), printing, localStrokeColor);

    Vector<FloatRect, 4> dashBounds;
    ASSERT(!(widths.size() % 2));
    dashBounds.reserveInitialCapacity(dashBounds.size() / 2);
    for (size_t i = 0; i < widths.size(); i += 2)
        dashBounds.append(FloatRect(FloatPoint(bounds.x() + widths[i], bounds.y()), FloatSize(widths[i+1] - widths[i], bounds.height())));

    if (doubleUnderlines) {
        // The space between double underlines is equal to the height of the underline
        for (size_t i = 0; i < widths.size(); i += 2)
            dashBounds.append(FloatRect(FloatPoint(bounds.x() + widths[i], bounds.y() + 2 * bounds.height()), FloatSize(widths[i+1] - widths[i], bounds.height())));
    }

    for (auto& dash : dashBounds)
        fillRect(dash, localStrokeColor);
}

void GraphicsContext::updateDocumentMarkerResources()
{
    // Unnecessary, since our document markers don't use resources.
}

void GraphicsContext::drawLineForDocumentMarker(const FloatPoint& origin, float width, DocumentMarkerLineStyle style)
{
    if (paintingDisabled())
        return;

    context(m_data)->drawLineForDocumentMarker(toWebGraphics(origin), width, toWebGraphics(style), cMisspellingLineThickness, toWebGraphics(state()));
}

FloatRect GraphicsContext::roundToDevicePixels(const FloatRect& frect, RoundingMode roundingMode)
{
    return toWebCore(context(m_data)->roundToDevicePixels(toWebGraphics(frect), toWebGraphics(roundingMode)));
}

void GraphicsContext::translate(float x, float y)
{
    if (paintingDisabled())
        return;

    context(m_data)->translate(x, y);
    m_data->translate(x, y);
}

void GraphicsContext::setPlatformFillColor(const Color&)
{
    // Cairo contexts can't hold separate fill and stroke colors
    // so we set them just before we actually fill or stroke
}

void GraphicsContext::setPlatformStrokeColor(const Color&)
{
    // Cairo contexts can't hold separate fill and stroke colors
    // so we set them just before we actually fill or stroke
}

void GraphicsContext::setPlatformStrokeThickness(float strokeThickness)
{
    if (paintingDisabled())
        return;

    notImplemented();
}

void GraphicsContext::setPlatformStrokeStyle(StrokeStyle strokeStyle)
{
    if (paintingDisabled())
        return;

    notImplemented();
}

void GraphicsContext::setURLForRect(const URL&, const IntRect&)
{
    notImplemented();
}

void GraphicsContext::concatCTM(const AffineTransform& transform)
{
    if (paintingDisabled())
        return;

    context(m_data)->concatCTM(toWebGraphics(transform));
}

void GraphicsContext::setCTM(const AffineTransform& transform)
{
    if (paintingDisabled())
        return;

    context(m_data)->setCTM(toWebGraphics(transform));
    m_data->setCTM(transform);
}

void GraphicsContext::setPlatformShadow(FloatSize const& size, float, Color const&)
{
    if (paintingDisabled())
        return;

    if (m_state.shadowsIgnoreTransforms) {
        // Meaning that this graphics context is associated with a CanvasRenderingContext
        // We flip the height since CG and HTML5 Canvas have opposite Y axis
        m_state.shadowOffset = FloatSize(size.width(), -size.height());
    }

    // Cairo doesn't support shadows natively, they are drawn manually in the draw* functions using ShadowBlur.
    platformContext()->shadowBlur().setShadowValues(FloatSize(m_state.shadowBlur, m_state.shadowBlur),
                                                    m_state.shadowOffset,
                                                    m_state.shadowColor,
                                                    m_state.shadowsIgnoreTransforms);
}

void GraphicsContext::clearPlatformShadow()
{
    if (paintingDisabled())
        return;

    platformContext()->shadowBlur().clear();
}

void GraphicsContext::beginPlatformTransparencyLayer(float opacity)
{
    if (paintingDisabled())
        return;

    context(m_data)->beginTransparencyLayer(opacity, toWebGraphics(state()));
    m_data->layers.append(opacity);
}

void GraphicsContext::endPlatformTransparencyLayer()
{
    if (paintingDisabled())
        return;

    context(m_data)->endTransparencyLayer();
    m_data->layers.removeLast();
}

bool GraphicsContext::supportsTransparencyLayers()
{
    return true;
}

void GraphicsContext::clearRect(const FloatRect& rect)
{
    if (paintingDisabled())
        return;

    context(m_data)->clearRect(toWebGraphics(rect));
}

void GraphicsContext::strokeRect(const FloatRect& rect, float width)
{
    if (paintingDisabled())
        return;

    context(m_data)->strokeRect(toWebGraphics(rect), width, toWebGraphics(state()));
}

void GraphicsContext::setLineCap(LineCap lineCap)
{
    if (paintingDisabled())
        return;

    context(m_data)->setLineCap(toWebGraphics(lineCap));
}

void GraphicsContext::setLineDash(const DashArray& dashes, float dashOffset)
{
    context(m_data)->setLineDash(toWebGraphics(dashes), dashOffset);
}

void GraphicsContext::setLineJoin(LineJoin lineJoin)
{
    if (paintingDisabled())
        return;

    context(m_data)->setLineJoin(toWebGraphics(lineJoin));
}

void GraphicsContext::setMiterLimit(float miter)
{
    if (paintingDisabled())
        return;

    context(m_data)->setMiterLimit(miter);
}

void GraphicsContext::setPlatformAlpha(float alpha)
{
    notImplemented();
}

void GraphicsContext::setPlatformCompositeOperation(CompositeOperator op, BlendMode blendOp)
{
    if (paintingDisabled())
        return;

    notImplemented();
}

void GraphicsContext::canvasClip(const Path& path, WindRule windRule)
{
    clipPath(path, windRule);
}

void GraphicsContext::clipOut(const Path& path)
{
    if (paintingDisabled())
        return;

    context(m_data)->clipOut(toWebGraphics(path));
}

void GraphicsContext::rotate(float radians)
{
    if (paintingDisabled())
        return;

    context(m_data)->rotate(radians);
}

void GraphicsContext::scale(const FloatSize& size)
{
    if (paintingDisabled())
        return;

    context(m_data)->scale(toWebGraphics(size));
    m_data->scale(size);
}

void GraphicsContext::clipOut(const FloatRect& r)
{
    if (paintingDisabled())
        return;

    context(m_data)->clipOut(toWebGraphics(r));
}

void GraphicsContext::platformFillRoundedRect(const FloatRoundedRect& rect, const Color& color)
{
    if (paintingDisabled())
        return;

    context(m_data)->fillRoundedRect(toWebGraphics(rect), toWebGraphics(color), toWebGraphics(state()));
}

void GraphicsContext::fillRectWithRoundedHole(const FloatRect& rect, const FloatRoundedRect& roundedHoleRect, const Color& color)
{
    if (paintingDisabled() || !color.isValid())
        return;

    context(m_data)->fillRectWithRoundedHole(toWebGraphics(rect), toWebGraphics(roundedHoleRect), toWebGraphics(color), toWebGraphics(state()));
}

void GraphicsContext::drawPattern(Image& image, const FloatRect& tileRect, const AffineTransform& patternTransform, const FloatPoint& phase, const FloatSize&, CompositeOperator op, const FloatRect& destRect, BlendMode)
{
    RefPtr<cairo_surface_t> surface = image.nativeImageForCurrentFrame();
    if (!surface) // If it's too early we won't have an image yet.
        return;

    context(m_data)->drawPattern(surface.get(), toWebGraphics(tileRect), toWebGraphics(patternTransform), 
        toWebGraphics(phase), toWebGraphics(op), toWebGraphics(destRect), toWebGraphics(state()));
}

void GraphicsContext::setPlatformShouldAntialias(bool enable)
{
    if (paintingDisabled())
        return;

    notImplemented();
}

void GraphicsContext::setPlatformImageInterpolationQuality(InterpolationQuality quality)
{
    notImplemented();
}

bool GraphicsContext::isAcceleratedContext() const
{
    return context(m_data)->isAcceleratedContext();
}

#if ENABLE(3D_TRANSFORMS) && USE(TEXTURE_MAPPER)
TransformationMatrix GraphicsContext::get3DTransform() const
{
    // FIXME: Can we approximate the transformation better than this?
    return getCTM().toTransformationMatrix();
}

void GraphicsContext::concat3DTransform(const TransformationMatrix& transform)
{
    concatCTM(transform.toAffineTransform());
}

void GraphicsContext::set3DTransform(const TransformationMatrix& transform)
{
    setCTM(transform.toAffineTransform());
}
#endif // ENABLE(3D_TRANSFORMS) && USE(TEXTURE_MAPPER)

void Gradient::fill(GraphicsContext* graphicsContext, const FloatRect& rect)
{
    context(graphicsContext)->fillRect(toWebGraphics(rect), toWebGraphics(this), toWebGraphics(graphicsContext->state()));
}

void FontCascade::drawGlyphs(GraphicsContext& graphicsContext, const Font& font, const GlyphBuffer& glyphBuffer,
    int from, int numGlyphs, const FloatPoint& point, FontSmoothingMode)
{
    if (!font.platformData().size())
        return;

    GlyphBufferGlyph* glyphs = const_cast<GlyphBufferGlyph*>(glyphBuffer.glyphs(from));

    float offset = point.x();
    for (int i = 0; i < numGlyphs; i++) {
        glyphs[i].x = offset;
        glyphs[i].y = point.y();
        offset += glyphBuffer.advanceAt(from + i).width();
    }

    WebGraphicsContextStateWrapper state = toWebGraphics(graphicsContext.state());
    state.m_state.syntheticBoldOffset = font.syntheticBoldOffset();

    context(&graphicsContext)->drawGlyphs(font.platformData().scaledFont(), glyphs, numGlyphs, state.m_state);
}

} // namespace WebCore

#endif // USE(CAIRO)

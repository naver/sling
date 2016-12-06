/*
 Copyright (C) 2013 Nokia Corporation and/or its subsidiary(-ies)

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License as published by the Free Software Foundation; either
 version 2 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.

 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
 */

#ifndef TextureMapperTiledBackingStore_h
#define TextureMapperTiledBackingStore_h

#include "FloatRect.h"
#include "Image.h"
#include "TextureMapperBackingStore.h"
#include "TextureMapperTile.h"
#include <wtf/RefPtr.h>

namespace WebCore {

class TextureMapper;

class TextureMapperTiledBackingStore : public TextureMapperBackingStore {
public:
    static PassRefPtr<TextureMapperTiledBackingStore> create() { return adoptRef(new TextureMapperTiledBackingStore); }
    virtual ~TextureMapperTiledBackingStore() { }

    RefPtr<BitmapTexture> texture() const override;
    void paintToTextureMapper(TextureMapper&, const FloatRect&, const TransformationMatrix&, float) override;
    void drawBorder(TextureMapper&, const Color&, float borderWidth, const FloatRect&, const TransformationMatrix&) override;
    void drawRepaintCounter(TextureMapper&, int repaintCount, const Color&, const FloatRect&, const TransformationMatrix&) override;

    void updateContentsScale(float);
    void updateContents(TextureMapper&, GraphicsLayer*, const FloatSize&, const IntRect&, BitmapTexture::UpdateContentsFlag);

    void setContentsToImage(Image* image) { m_image = image; m_size = image ? image->size() : FloatSize(); }

private:
    TextureMapperTiledBackingStore() { }

    void createOrDestroyTilesIfNeeded(TextureMapper&, const FloatRect&, const TransformationMatrix&, const IntSize&, bool);
    void updateContentsFromImageIfNeeded(TextureMapper&, const FloatRect&, const TransformationMatrix&);
    void updateContentsFromLayerIfNeeded(TextureMapper&, const FloatRect&, const TransformationMatrix&);
    TransformationMatrix adjustedTransformForRect(const FloatRect&);
    inline FloatRect rect() const
    {
        FloatRect rect(FloatPoint::zero(), m_size);
        rect.scale(m_contentsScale);
        return rect;
    }

    Vector<TextureMapperTile> m_tiles;
    FloatSize m_size;
    RefPtr<Image> m_image;
    GraphicsLayer* m_sourceLayer { nullptr };
    IntRect m_dirtyRect;
    BitmapTexture::UpdateContentsFlag m_updateContentsFlag { BitmapTexture::UpdateCannotModifyOriginalImageData };
    float m_contentsScale { 1 };
    bool m_isScaleDirty { false };
    bool m_isSizeDirty { false };
    FloatRect m_visibleRect;
    FloatRect m_coverRect;
};

} // namespace WebCore

#endif

/*
 Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies)

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

#include "config.h"

#include "TextureMapperTiledBackingStore.h"

#include "ImageBuffer.h"
#include "ImageObserver.h"
#include "TextureMapper.h"

namespace WebCore {

class GraphicsLayer;

void TextureMapperTiledBackingStore::updateContentsFromImageIfNeeded(TextureMapper& textureMapper, const FloatRect& targetRect, const TransformationMatrix& transform)
{
    if (!m_image)
        return;

    createOrDestroyTilesIfNeeded(textureMapper, targetRect, transform, textureMapper.maxTextureSize(), !m_image->currentFrameKnownToBeOpaque());
    for (auto& tile : m_tiles)
        tile.updateContents(textureMapper, m_image.get(), enclosingIntRect(m_image->rect()), BitmapTexture::UpdateCannotModifyOriginalImageData);

    if (m_image->imageObserver())
        m_image->imageObserver()->didDraw(m_image.get());
    m_image = nullptr;
}

void TextureMapperTiledBackingStore::updateContentsFromLayerIfNeeded(TextureMapper& textureMapper, const FloatRect& targetRect, const TransformationMatrix& transform)
{
    if (!m_sourceLayer)
        return;

    createOrDestroyTilesIfNeeded(textureMapper, targetRect, transform, textureMapper.maxTextureSize(), true);
    for (auto& tile : m_tiles)
        tile.updateContents(textureMapper, m_sourceLayer, m_dirtyRect, m_updateContentsFlag, m_contentsScale);

    m_dirtyRect = IntRect();
}

TransformationMatrix TextureMapperTiledBackingStore::adjustedTransformForRect(const FloatRect& targetRect)
{
    return TransformationMatrix::rectToRect(rect(), targetRect);
}

void TextureMapperTiledBackingStore::paintToTextureMapper(TextureMapper& textureMapper, const FloatRect& targetRect, const TransformationMatrix& transform, float opacity)
{
    TransformationMatrix adjustedTransform = transform * adjustedTransformForRect(targetRect);
    updateContentsFromImageIfNeeded(textureMapper, targetRect, adjustedTransform);
    updateContentsFromLayerIfNeeded(textureMapper, targetRect, adjustedTransform);
    for (auto& tile : m_tiles)
        tile.paint(textureMapper, adjustedTransform, opacity, calculateExposedTileEdges(rect(), tile.rect()));
}

void TextureMapperTiledBackingStore::drawBorder(TextureMapper& textureMapper, const Color& borderColor, float borderWidth, const FloatRect& targetRect, const TransformationMatrix& transform)
{
    TransformationMatrix adjustedTransform = transform * adjustedTransformForRect(targetRect);
    for (auto& tile : m_tiles)
        textureMapper.drawBorder(borderColor, borderWidth, tile.rect(), adjustedTransform);
}

void TextureMapperTiledBackingStore::drawRepaintCounter(TextureMapper& textureMapper, int repaintCount, const Color& borderColor, const FloatRect& targetRect, const TransformationMatrix& transform)
{
    TransformationMatrix adjustedTransform = transform * adjustedTransformForRect(targetRect);
    for (auto& tile : m_tiles)
        textureMapper.drawNumber(repaintCount, borderColor, tile.rect().location(), adjustedTransform);
}

void TextureMapperTiledBackingStore::updateContentsScale(float scale)
{
    if (m_contentsScale == scale)
        return;

    m_isScaleDirty = true;
    m_contentsScale = scale;
}

void TextureMapperTiledBackingStore::createOrDestroyTilesIfNeeded(TextureMapper& textureMapper, const FloatRect& targetRect, const TransformationMatrix& transform, const IntSize& maxTileSize, bool hasAlpha)
{
    static const int tileTextureSize = 512;
    static const IntSize tileSize(std::min(tileTextureSize, maxTileSize.width()), std::min(tileTextureSize, maxTileSize.height()));

    if (!m_isSizeDirty && !m_isScaleDirty && m_visibleRect == rect())
        return;

    m_isSizeDirty = false;
    m_isScaleDirty = false;

    FloatRect visibleRect = rect();
    if (!m_image)
        visibleRect.scale(m_contentsScale);
    visibleRect = transform.mapRect(visibleRect);

    FloatRect clipRect = textureMapper.clipBounds();
    clipRect.inflateX(tileSize.width());
    clipRect.inflateY(tileSize.height());
    visibleRect.intersect(clipRect);
    if (visibleRect.isEmpty())
        return;

    TransformationMatrix inverse = transform.inverse().valueOr(TransformationMatrix());
    visibleRect = inverse.mapRect(visibleRect);

    if (m_visibleRect == visibleRect)
        return;

    IntPoint tilesOrigin(visibleRect.x() / tileSize.width(), visibleRect.y() / tileSize.height());
    IntPoint tilesEnd((visibleRect.maxX() + tileSize.width() - 1) / tileSize.width(), (visibleRect.maxY() + tileSize.height() - 1) / tileSize.height());
    FloatRect coverRect(FloatPoint(tilesOrigin.x() * tileSize.width(), tilesOrigin.y() * tileSize.height()), FloatPoint(tilesEnd.x() * tileSize.width(), tilesEnd.y() * tileSize.height()));

    FloatRect rigidVisibleRect = intersection(m_visibleRect, visibleRect);
    IntPoint rigidTilesOrigin((rigidVisibleRect.x() + tileSize.width() - 1) / tileSize.width(), (rigidVisibleRect.y() + tileSize.height() - 1) / tileSize.height());
    IntPoint rigidTilesEnd(rigidVisibleRect.maxX() / tileSize.width(), rigidVisibleRect.maxY() / tileSize.height());
    FloatRect rigidCoverRect(FloatPoint(rigidTilesOrigin.x() * tileSize.width(), rigidTilesOrigin.y() * tileSize.height()), FloatPoint(rigidTilesEnd.x() * tileSize.width(), rigidTilesEnd.y() * tileSize.height()));

    m_visibleRect = visibleRect;
    m_coverRect = coverRect;

    Vector<FloatRect> tileRectsToAdd;
    Vector<int> tileIndicesToRemove;
    static const size_t TileEraseThreshold = 0;

    // This method recycles tiles. We check which tiles we need to add, which to remove, and use as many
    // removable tiles as replacement for new tiles when possible.
    for (float y = coverRect.location().y(); y < coverRect.maxY(); y += tileSize.height()) {
        for (float x = coverRect.location().x(); x < coverRect.maxX(); x += tileSize.width()) {
            FloatRect tileRect(x, y, tileSize.width(), tileSize.height());
            if (!rigidCoverRect.contains(tileRect))
                tileRectsToAdd.append(tileRect);
        }
    }

    // Check which tiles need to be removed, and which already exist.
    for (int i = m_tiles.size() - 1; i >= 0; --i) {
        FloatRect tileRect = m_tiles[i].rect();
        if (!rigidCoverRect.contains(tileRect))
            // This tile is may not be needed.
            tileIndicesToRemove.append(i);
    }

    // Recycle removable tiles to be used for newly requested tiles.
    for (auto& rect : tileRectsToAdd) {
        FloatRect tileRect(rect.location(), tileSize);
        m_dirtyRect.unite(enclosingIntRect(rect));

        if (!tileIndicesToRemove.isEmpty()) {
            // We recycle an existing tile for usage with a new tile rect.
            TextureMapperTile& tile = m_tiles[tileIndicesToRemove.last()];
            tileIndicesToRemove.removeLast();
            tile.setRect(tileRect);
            tile.setVisibleRect(rect);

            if (tile.texture())
                tile.texture()->reset(enclosingIntRect(tile.rect()).size(), hasAlpha ? BitmapTexture::SupportsAlpha : 0);
            continue;
        }

        m_tiles.append(TextureMapperTile(tileRect, rect));
    }

    // Remove unnecessary tiles, if they weren't recycled.
    // We use a threshold to make sure we don't create/destroy tiles too eagerly.
    for (auto& index : tileIndicesToRemove) {
        if (m_tiles.size() <= TileEraseThreshold)
            break;
        m_tiles.remove(index);
    }
}

void TextureMapperTiledBackingStore::updateContents(TextureMapper& textureMapper, GraphicsLayer* sourceLayer, const FloatSize& totalSize, const IntRect& dirtyRect, BitmapTexture::UpdateContentsFlag updateContentsFlag)
{
    m_sourceLayer = sourceLayer;
    m_isSizeDirty = m_size != totalSize;
    m_size = totalSize;
    m_dirtyRect = dirtyRect;
    m_updateContentsFlag = updateContentsFlag;
}

RefPtr<BitmapTexture> TextureMapperTiledBackingStore::texture() const
{
    for (const auto& tile : m_tiles) {
        if (auto texture = tile.texture())
            return texture;
    }

    return nullptr;
}

} // namespace WebCore

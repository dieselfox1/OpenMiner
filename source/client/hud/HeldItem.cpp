/*
 * =====================================================================================
 *
 *  OpenMiner
 *
 *  Copyright (C) 2018-2020 Unarelith, Quentin Bazin <openminer@unarelith.net>
 *  Copyright (C) 2019-2020 the OpenMiner contributors (see CONTRIBUTORS.md)
 *
 *  This file is part of OpenMiner.
 *
 *  OpenMiner is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  OpenMiner is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with OpenMiner; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * =====================================================================================
 */
#include <cmath>

#include "common/core/GameClock.hpp"
#include "common/core/Registry.hpp"
#include "common/resource/ResourceHandler.hpp"

#include "client/core/Config.hpp"
#include "client/graphics/TextureAtlas.hpp"
#include "client/hud/HeldItem.hpp"
#include "client/hud/Hotbar.hpp"

HeldItem::HeldItem()
	: m_textureAtlas(ResourceHandler::getInstance().get<TextureAtlas>("atlas-blocks"))
{
	// Right arm front face in the player skin (see PlayerBox model coordinates)
	m_armImage.load("texture-player");
	m_armImage.setClipRect(40.f, 20.f, 4, 12);
	m_armImage.setOrigin(2.f, 6.f);
	m_armImage.setScale(10.f, 10.f);
	m_armImage.setRotation(200.f);
	m_armImage.setPosition(20.f, 40.f, 0.f);

	m_itemImage.setOrigin(8.f, 8.f);
	m_itemImage.setRotation(-20.f);
}

void HeldItem::setup() {
	m_anchorX = (float)Config::screenWidth / Config::guiScale - 60.f;
	m_anchorY = (float)Config::screenHeight / Config::guiScale - 40.f;
}

void HeldItem::update(const Hotbar& hotbar) {
	const Item* item = (hotbar.cursorPos() >= 0) ? &hotbar.currentItem() : nullptr;
	u16 itemID = item ? item->id() : 0;

	if (!m_isItemInitialized || itemID != m_itemID || m_oldTexturePack != Config::texturePack)
		updateItem(item);

	updateAnimation();
}

static constexpr u32 swingDuration = 300;

void HeldItem::startSwinging() {
	m_isSwinging = true;

	// Don't interrupt a swing that is still playing
	u32 ticks = GameClock::getInstance().getTicks();
	if (!m_swingTicks || ticks - m_swingTicks >= swingDuration)
		m_swingTicks = ticks;
}

void HeldItem::stopSwinging() {
	m_isSwinging = false;
}

void HeldItem::updateItem(const Item* item) {
	m_itemID = item ? item->id() : 0;
	m_oldTexturePack = Config::texturePack;
	m_isItemInitialized = true;
	m_equipTicks = GameClock::getInstance().getTicks();

	if (!m_itemID) {
		m_displayMode = DisplayMode::Arm;
		return;
	}

	const BlockState* blockState = nullptr;
	if (item->isBlock()) {
		const Block& block = Registry::getInstance().getBlock(m_itemID);
		blockState = &block.getState(0); // FIXME: Get state from item stack
		if (blockState->drawType() != BlockDrawType::XShape && blockState->inventoryImage().empty()) {
			m_cube.updateVertexBuffer(block);
			m_displayMode = DisplayMode::Cube;
			return;
		}
	}

	m_itemImage.load(m_textureAtlas.texture());

	const FloatRect& clipRect = m_textureAtlas.getTexCoords(item->tiles().getTextureForFace(0), false);
	m_itemImage.setClipRect(clipRect.x, clipRect.y, (u16)clipRect.sizeX, (u16)clipRect.sizeY);
	m_itemImage.setOrigin(clipRect.sizeX / 2.f, clipRect.sizeY / 2.f);
	m_itemImage.setScale(96.f / clipRect.sizeX, 96.f / clipRect.sizeY);
	m_itemImage.setColor(blockState ? blockState->colorMultiplier() : Color::White);

	m_displayMode = DisplayMode::Image;
}

void HeldItem::updateAnimation() {
	u32 ticks = GameClock::getInstance().getTicks();

	float offsetX = 0.f;
	float offsetY = 0.f;
	float angle = 0.f;
	float scaleFactor = 1.f;

	constexpr u32 equipDuration = 250;
	if (ticks - m_equipTicks < equipDuration)
		offsetY += (1.f - float(ticks - m_equipTicks) / equipDuration) * 120.f;

	// Keep swinging while the mouse button is held down
	if (m_isSwinging && ticks - m_swingTicks >= swingDuration)
		m_swingTicks = ticks;

	// Fake a forward thrust in the orthographic HUD view: rotate around the
	// anchor, push towards the crosshair and shrink a bit to suggest depth
	if (m_swingTicks && ticks - m_swingTicks < swingDuration) {
		float progress = sinf(float(ticks - m_swingTicks) / swingDuration * 3.14159f);
		offsetX -= progress * 45.f;
		offsetY -= progress * 25.f;
		angle = -progress * 25.f;
		scaleFactor = 1.f - progress * 0.15f;
	}

	setPosition(m_anchorX + offsetX, m_anchorY + offsetY);
	setRotation(angle);
	setScale(scaleFactor, scaleFactor);
}

void HeldItem::draw(RenderTarget& target, RenderStates states) const {
	states.transform *= getTransform();

	if (m_displayMode == DisplayMode::Cube)
		target.draw(m_cube, states);
	else if (m_displayMode == DisplayMode::Image)
		target.draw(m_itemImage, states);
	else
		target.draw(m_armImage, states);
}
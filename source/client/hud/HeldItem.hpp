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
#ifndef HELDITEM_HPP_
#define HELDITEM_HPP_

#include "client/graphics/Image.hpp"
#include "client/gui/InventoryCube.hpp"

class Hotbar;
class Item;

class HeldItem : public Drawable, public Transformable {
public:
	HeldItem();

	void setup();

	void update(const Hotbar& hotbar);

	void startSwinging();
	void stopSwinging();

private:
	enum class DisplayMode {
		Arm,
		Cube,
		Image
	};

	void updateItem(const Item* item);
	void updateAnimation();

	void draw(RenderTarget& target, RenderStates states) const override;

	const TextureAtlas& m_textureAtlas;

	InventoryCube m_cube{ 90.f };
	Image m_itemImage;
	Image m_armImage;

	DisplayMode m_displayMode = DisplayMode::Arm;

	u16 m_itemID = 0;
	bool m_isItemInitialized = false;
	std::string m_oldTexturePack;

	float m_anchorX = 0.f;
	float m_anchorY = 0.f;

	u32 m_equipTicks = 0;
	u32 m_swingTicks = 0;
	bool m_isSwinging = false;
};

#endif // HELDITEM_HPP_
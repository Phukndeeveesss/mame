// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Phil Stroffolino, Bryan McPhail
/*
    Video Hardware for Shoot Out
    prom GB09.K6 may be related to background tile-sprite priority
*/

#include "emu.h"
#include "includes/shootout.h"
#include "screen.h"


void shootout_state::shootout_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// blue component
		bit0 = 0;
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 7);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}



TILE_GET_INFO_MEMBER(shootout_state::get_bg_tile_info)
{
	int attributes = m_videoram[tile_index+0x400]; /* CCCC -TTT */
	int tile_number = m_videoram[tile_index] + 256*(attributes&7);
	int color = attributes>>4;

	tileinfo.set(2,
			tile_number,
			color,
			0);
}

TILE_GET_INFO_MEMBER(shootout_state::get_fg_tile_info)
{
	int attributes = m_textram[tile_index+0x400]; /* CCCC --TT */
	int tile_number = m_textram[tile_index] + 256*(attributes&0x3);
	int color = attributes>>4;

	tileinfo.set(0,
			tile_number,
			color,
			0);
}

void shootout_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_background->mark_tile_dirty(offset&0x3ff );
}

void shootout_state::textram_w(offs_t offset, uint8_t data)
{
	m_textram[offset] = data;
	m_foreground->mark_tile_dirty(offset&0x3ff );
}

void shootout_state::video_start()
{
	m_background = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(shootout_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_foreground = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(shootout_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_foreground->set_transparent_pen(0);

	save_item(NAME(m_ccnt_old_val));
}

void shootout_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int bank_bits )
{
	gfx_element *gfx = m_gfxdecode->gfx(1);
	const uint8_t *source = m_spriteram+127*4;
	int count;

	bool m_bFlicker = (screen.frame_number () & 1) != 0;

	for( count=0; count<128; count++ )
	{
		int attributes = source[1];
		/*
		    76543210
		    xxx-----    bank
		    ---x----    vertical size
		    ----x---    priority
		    -----x--    horizontal flip
		    ------x-    flicker
		    -------x    enable
		*/
		if ( attributes & 0x01 ){ /* visible */
			if( m_bFlicker || (attributes&0x02)==0 ){
				int priority_mask = (attributes&0x08)?0x2:0;
				int sx = (240 - source[2])&0xff;
				int sy = (240 - source[0])&0xff;
				int vx, vy;
				int number = source[3] | ((attributes<<bank_bits)&0x700);
				int flipx = (attributes & 0x04);
				int flipy = 0;

				if (flip_screen()) {
					flipx = !flipx;
					flipy = !flipy;
				}

				if( attributes & 0x10 ){ /* double height */
					number = number&(~1);
					sy -= 16;

					vx = sx;
					vy = sy;
					if (flip_screen()) {
						vx = 240 - vx;
						vy = 240 - vy;
					}

					gfx->prio_transpen(bitmap,cliprect,
						number,
						0 /*color*/,
						flipx,flipy,
						vx,vy,
						screen.priority(),
						priority_mask,0);

					number++;
					sy += 16;
				}

				vx = sx;
				vy = sy;
				if (flip_screen()) {
					vx = 240 - vx;
					vy = 240 - vy;
				}

				gfx->prio_transpen(bitmap,cliprect,
						number,
						0 /*color*/,
						flipx,flipy,
						vx,vy,
						screen.priority(),
						priority_mask,0);
			}
		}
		source -= 4;
	}
}

uint32_t shootout_state::screen_update_shootout(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	m_background->draw(screen, bitmap, cliprect, 0,0);
	m_foreground->draw(screen, bitmap, cliprect, 0,1);
	draw_sprites(screen, bitmap, cliprect,3/*bank bits */);
	return 0;
}

uint32_t shootout_state::screen_update_shootouj(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	m_background->draw(screen, bitmap, cliprect, 0,0);
	m_foreground->draw(screen, bitmap, cliprect, 0,1);
	draw_sprites(screen, bitmap, cliprect,2/*bank bits*/);
	return 0;
}

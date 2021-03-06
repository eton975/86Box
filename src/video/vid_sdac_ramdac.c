/*
 * 86Box	A hypervisor and IBM PC system emulator that specializes in
 *		running old operating systems and software designed for IBM
 *		PC systems and compatibles from 1981 through fairly recent
 *		system designs based on the PCI bus.
 *
 *		This file is part of the 86Box distribution.
 *
 *		87C716 'SDAC' true colour RAMDAC emulation.
 *
 *		Misidentifies as AT&T 21C504.
 *
 * Version:	@(#)vid_sdac_ramdac.c	1.0.2	2017/11/04
 *
 * Authors:	Sarah Walker, <http://pcem-emulator.co.uk/>
 *		Miran Grca, <mgrca8@gmail.com>
 *
 *		Copyright 2008-2017 Sarah Walker.
 *		Copyright 2016,2017 Miran Grca.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>
#include "../86box.h"
#include "../mem.h"
#include "video.h"
#include "vid_svga.h"
#include "vid_sdac_ramdac.h"


/* Returning divider * 2 */
int sdac_get_clock_divider(sdac_ramdac_t *ramdac)
{
        switch (ramdac->command >> 4)
        {
		case 0x1: return 1;
		case 0x0: case 0x3: case 0x5: return 2;
		case 0x9: return 3;
                case 0x2: case 0x6: case 0x7: case 0x8: case 0xa: case 0xc: return 4;
                case 0x4: case 0xe: return 6;
                default: return 2;
        }
}

void sdac_ramdac_out(uint16_t addr, uint8_t val, sdac_ramdac_t *ramdac, svga_t *svga)
{
        switch (addr)
        {
                case 0x3C6:
                if (val == 0xff)
                {
                        ramdac->rs2 = 0;
                        ramdac->magic_count = 0;
                        break;
                }
                if (ramdac->magic_count < 4) break;
                if (ramdac->magic_count == 4)
                {
                        ramdac->command = val;
                        switch (val >> 4)
                        {
                                case 0x2: case 0x3: case 0x8: case 0xa: svga->bpp = 15; break;
                                case 0x4: case 0x9: case 0xe: svga->bpp = 24; break;
                                case 0x5: case 0x6: case 0xc: svga->bpp = 16; break;
                                case 0x7: case 0xd:           svga->bpp = 32; break;

                                case 0: case 1: default: svga->bpp = 8; break;
                        }
			svga_recalctimings(svga);
			pclog("RAMDAC: Mode: %i, BPP: %i\n", val >> 4, svga->bpp);
                }
                break;
                
                case 0x3C7:
                ramdac->magic_count = 0;
                if (ramdac->rs2)
                   ramdac->rindex = val;
                break;
                case 0x3C8:
                ramdac->magic_count = 0;
                if (ramdac->rs2)
                   ramdac->windex = val;
                break;
                case 0x3C9:
                ramdac->magic_count = 0;
                if (ramdac->rs2)
                {
                        if (!ramdac->reg_ff) ramdac->regs[ramdac->windex & 0xff] = (ramdac->regs[ramdac->windex & 0xff] & 0xff00) | val;
                        else                 ramdac->regs[ramdac->windex & 0xff] = (ramdac->regs[ramdac->windex & 0xff] & 0x00ff) | (val << 8);
                        ramdac->reg_ff = !ramdac->reg_ff;
                        if (!ramdac->reg_ff) ramdac->windex++;
                }
                break;
        }
        svga_out(addr, val, svga);
}

uint8_t sdac_ramdac_in(uint16_t addr, sdac_ramdac_t *ramdac, svga_t *svga)
{
        uint8_t temp;
        switch (addr)
        {
                case 0x3C6:
                ramdac->reg_ff = 0;
                if (ramdac->magic_count < 5)
                   ramdac->magic_count++;
                if (ramdac->magic_count == 4)
                {
                        temp = 0x70; /*SDAC ID*/
                        ramdac->rs2 = 1;
                }
                if (ramdac->magic_count == 5)
                {
                        temp = ramdac->command;
                        ramdac->magic_count = 0;
                }
                return temp;
                case 0x3C7:
                        ramdac->magic_count=0;
                if (ramdac->rs2) return ramdac->rindex;
                break;
                case 0x3C8:
                        ramdac->magic_count=0;
                if (ramdac->rs2) return ramdac->windex;
                break;
                case 0x3C9:
                        ramdac->magic_count=0;
                if (ramdac->rs2)
                {
                        if (!ramdac->reg_ff) temp = ramdac->regs[ramdac->rindex & 0xff] & 0xff;
                        else                 temp = ramdac->regs[ramdac->rindex & 0xff] >> 8;
                        ramdac->reg_ff = !ramdac->reg_ff;
                        if (!ramdac->reg_ff)
                        {
                                ramdac->rindex++;
                                ramdac->magic_count = 0;
                        }
                        return temp;
                }
                break;
        }
        return svga_in(addr, svga);
}

float sdac_getclock(int clock, void *p)
{
        sdac_ramdac_t *ramdac = (sdac_ramdac_t *)p;
        float t;
        int m, n1, n2;
        if (clock == 0) return 25175000.0;
        if (clock == 1) return 28322000.0;
        clock ^= 1; /*Clocks 2 and 3 seem to be reversed*/
        m  =  (ramdac->regs[clock] & 0x7f) + 2;
        n1 = ((ramdac->regs[clock] >>  8) & 0x1f) + 2;
        n2 = ((ramdac->regs[clock] >> 13) & 0x07);
        t = (14318184.0 * ((float)m / (float)n1)) / (float)(1 << n2);
        return t;
}

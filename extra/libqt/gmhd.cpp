#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"

void oqt_gmhd_init(oqt_gmhd_t *gmhd)
{
	memset(&gmhd->gmin,0,sizeof(oqt_gmin_t));
}

void oqt_gmhd_delete(oqt_gmhd_t *gmhd)
{

}

void oqt_gmhd_dump(oqt_gmhd_t *gmhd)
{

}

void oqt_read_gmhd(oqt_t *file, oqt_gmhd_t *gmhd)
{
	oqt_atom_t leaf_atom;
	oqt_atom_read_header(file, &leaf_atom);
	if(oqt_atom_is(&leaf_atom, "gmin"))
	{
		gmhd->gmin.version = oqt_read_char(file);
		gmhd->gmin.flags = oqt_read_int24(file);
		gmhd->gmin.drawmode = oqt_read_int16(file);
		gmhd->gmin.color[0] = oqt_read_int16(file);
		if(gmhd->gmin.color[0]!=0)
		{
			gmhd->gmin.color[1] = oqt_read_int16(file);
			gmhd->gmin.color[2] = oqt_read_int16(file);
		}
		gmhd->gmin.balance = oqt_read_int16(file);
		gmhd->gmin.reserved = oqt_read_int16(file);
	}
}

void oqt_write_gmhd(oqt_t *file, oqt_gmhd_t *gmhd)
{
	oqt_atom_t atom;
	oqt_atom_write_header(file, &atom, "gmhd");

	oqt_atom_t atominfo;
	oqt_atom_write_header(file, &atominfo, "gmin");

	oqt_write_char(file, (char)gmhd->gmin.version);
	oqt_write_int24(file, gmhd->gmin.flags);
	oqt_write_int16(file, (short)gmhd->gmin.drawmode);
	if(gmhd->gmin.color[0]==0)
		oqt_write_int16(file, 0);
	else
	{
		oqt_write_int16(file, (short)gmhd->gmin.color[0]);
		oqt_write_int16(file, (short)gmhd->gmin.color[1]);
		oqt_write_int16(file, (short)gmhd->gmin.color[2]);
	}
	oqt_write_int16(file, (short)gmhd->gmin.balance);
	oqt_write_int16(file, (short)gmhd->gmin.reserved);

	oqt_atom_write_footer(file, &atominfo);

	oqt_atom_write_footer(file, &atom);
}

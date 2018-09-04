#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"

void oqt_read_genl(oqt_t *file, oqt_genl_t *genl)
{
	for(int i=0;i<6;i++)
		genl->reserved1[i] = oqt_read_int32(file);
//	oqt_read_data(file,(char*)genl->reserved1,24);
	LONG time = oqt_read_int32(file);
	for(int i=0;i<10;i++)
		genl->reserved2[i] = oqt_read_int32(file);
//	oqt_read_data(file,(char*)genl->reserved2,40);
	genl->width = oqt_read_int32(file);
	genl->height = oqt_read_int32(file);
	for(int i=0;i<2;i++)
		genl->reserved3[i] = oqt_read_int32(file);
	genl->framerate = oqt_read_fixed32(file);
	genl->reserved4 = oqt_read_int32(file);
	genl->frames = (long)(time/(1000*90/genl->framerate))-1;
//	oqt_read_data(file,(char*)genl->reserved3,16);
}

void oqt_read_ttfo(oqt_t *file,oqt_genl_t *genl,oqt_ttfo_t *ttfo)
{
	ttfo->reserved[0] = oqt_read_int32(file);
	ttfo->reserved[1] = oqt_read_int32(file);
	ttfo->reserved[2] = oqt_read_int32(file);
	ttfo->idxs = (oqt_midx_t*)malloc(sizeof(oqt_midx_t)*genl->frames);
	for(int i=0;i<genl->frames;i++)
	{
		ttfo->idxs[i].keypos = oqt_read_int32(file);
		ttfo->idxs[i].location = oqt_read_int32(file);
		ttfo->idxs[i].offset = oqt_read_int32(file);
		ttfo->idxs[i].display = (long)(oqt_read_int32(file)/(1000*90/genl->framerate));
	}
}

void oqt_read_time(oqt_t *file, oqt_time_t *time)
{

}

int oqt_read_m12v(oqt_t *file, oqt_m12v_t *m12v,oqt_atom_t *parent_atom)
{
	m12v->reserved[0] = oqt_read_int32(file);
	m12v->reserved[1] = oqt_read_int32(file);
	m12v->reserved[2] = oqt_read_int32(file);
	while(oqt_get_position(file) < parent_atom->end)
	{
		oqt_atom_t leaf_atom;
		oqt_atom_read_header(file, &leaf_atom);
		if(leaf_atom.size==0)
			oqt_set_position(file, leaf_atom.start+OQT_ATOM_HEAD_LENGTH);
		else if(oqt_atom_is(&leaf_atom, "genl"))
			oqt_read_genl(file,&m12v->genl);
		else if(oqt_atom_is(&leaf_atom, "ttfo"))
			oqt_read_ttfo(file,&m12v->genl,&m12v->ttfo);
		else if(oqt_atom_is(&leaf_atom, "time"))
			oqt_read_time(file,&m12v->time);
		oqt_atom_skip(file, &leaf_atom);
	}
	return 0;
}

int oqt_m12v_init(oqt_m12v_t *m12v)
{
	memset(m12v,0,sizeof(oqt_m12v_t));
	return 0;
}

int oqt_m12v_delete(oqt_m12v_t *m12v)
{
	if(m12v->ttfo.idxs)
		free(m12v->ttfo.idxs);
	return 0;
}

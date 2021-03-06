/*******************************************************************************
 translation.c

 libquicktime - A library for reading and writing quicktime/avi/mp4 files.
 http://libquicktime.sourceforge.net

 Copyright (C) 2002 Heroine Virtual Ltd.
 Copyright (C) 2002-2007 Members of the libquicktime project.

 This library is free software; you can redistribute it and/or modify it under
 the terms of the GNU Lesser General Public License as published by the Free
 Software Foundation; either version 2.1 of the License, or (at your option)
 any later version.

 This library is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 details.

 You should have received a copy of the GNU Lesser General Public License along
 with this library; if not, write to the Free Software Foundation, Inc., 51
 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*******************************************************************************/ 


#include "stdafx.h"
#include "lqt_private.h"
#include <pthread.h>

static pthread_mutex_t translation_mutex = PTHREAD_MUTEX_INITIALIZER;
static int translation_initialized = 0;

void lqt_translation_init()
  {
  pthread_mutex_lock(&translation_mutex);

  if(!translation_initialized)
    {
//    bindtextdomain(PACKAGE, LOCALE_DIR);
    translation_initialized = 1;
    }
  pthread_mutex_unlock(&translation_mutex);
  }


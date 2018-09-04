/* plugin.c
 * Copyright (C) 2001 QT4Linux and OpenQuicktime Teams
 *
 * This file is part of OpenQuicktime, a free QuickTime library.
 *
 * Based on QT4Linux by Adam Williams.
 *
 * OpenQuicktime is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation.
 *
 * OpenQuicktime is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: plugin.c,v 1.17 2003/04/07 21:02:24 shitowax Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"


#define REG_FUNC_SYMBOL "oqt_plugin_register"
#define ENV_PLUGPATH "OQT_PLUGIN_DIR"


#ifndef WIN32
#ifndef __MACOS__
#include <dirent.h>
#include <dlfcn.h>
#include "plugindir.h"

#define PLUGIN_PREFIX "oqt_codec_"
#define PLUGIN_SUFFIX ".so"

#else /* __MACOS__ */

static char* dlerror() { return NULL;};
#endif

#else /* WIN32 */

#include <windows.h>
static char* dlerror() { return NULL;};

#define PLUGIN_PREFIX "oqt_codec_"
#define PLUGIN_SUFFIX ".dll"

#endif




static void* open_plugin(const char *filepath)
{
 	void *handle = NULL;

#ifndef WIN32
#ifndef __MACOS__
	// RTLD_GLOBAL may introduce naming conflicts between plugins
	handle = dlopen(filepath, RTLD_NOW);
#endif
#else
    handle = LoadLibrary(filepath);
#endif

	if(!handle)
	{
		fprintf(stderr, "Can't load Huachuang QT plugin %s\n", filepath);
		fprintf(stderr, "%s\n", dlerror());
	}

	return handle;
}



// Change non word/special characters to underscore
static void convert_code(char *code) 
{
	int i;

	for(i=0;i<4;++i) {
		if ((code[i] >= 'A' && code[i] <= 'Z') || // A-Z 
		    (code[i] >= 'a' && code[i] <= 'z') || // a-z
		    (code[i] >= '0' && code[i] <= '9') || // 0-9
		     code[i] == '-' || code[i] == '_'  ||
		     code[i] == '+' || code[i] == '.')
		{} else {
			// I got my logic the wrong way round ;)
			code[i] = '_';
		}	
	}

}


static oqt_codec_info_t* call_plugin_regfunc(void* handle) {
	oqt_codec_info_t *codec_info = NULL;
	oqt_codec_info_t *(*register_func)(void);
	
	// Make sure it is a valid handle
	if (!handle) return 0;

#ifndef WIN32
#ifndef __MACOS__
#if defined(USCORE) && !defined(DLSYM_ADDS_USCORE)
	/* The current system needs an underscore before the symbol name */
	usymb = (char *)malloc(strlen(REG_FUNC_SYMBOL) + 2);
	*usymb = '_';
	strcpy(usymb + 1, REG_FUNC_SYMBOL);
	register_func = (oqt_codec_info_t*(*)(void))dlsym(handle, usymb);
	free(usymb);
#else
    register_func = (oqt_codec_info_t*(*)(void))dlsym(handle, REG_FUNC_SYMBOL);
#endif
#else /* __MACOS__ */
	register_func = NULL; /* well, we're not using it... */
#endif

#else
	/* WIN32 */
    register_func = (oqt_codec_info_t*(*)(void))GetProcAddress((HMODULE)handle, REG_FUNC_SYMBOL);
#endif


	/* Check for errors */
    if(!register_func)  {
		fprintf(stderr, "Error finding register function in plugin: %s\n",dlerror());
    	oqt_close_plugin(handle);
		return 0;
    }
    
    // Call the register function
    codec_info = register_func();
    if (!codec_info) {
    	fprintf(stderr, "Function %s returned null pointer to codec information\n", REG_FUNC_SYMBOL);
    } else {
		// Put the handle to the plugin file into the codec information
		codec_info->plugin_handle = handle;
		
		// Register the codec in the codec registry
		if (oqt_register_codec(codec_info)) {
			fprintf(stderr, "Failed to register codec in regsitry.\n");
			if (codec_info) {
				free(codec_info);
				codec_info = NULL;
			}
		}
    }
    
    
    return codec_info;
}

static void search_in_directory(const char* dir_path)
{
#ifndef WIN32	// sorry plugin disable under windows for the moment
#ifndef __MACOS__
	int prefix_len = strlen(PLUGIN_PREFIX);
	int suffix_len = strlen(PLUGIN_SUFFIX);
	struct dirent *dent;
	DIR *dirp;
	
	// Ignore directories with no length
	if (strlen(dir_path)==0) return;
	
	/*#ifdef DEBUG
	fprintf(stderr, "Searching in directory: %s\n", dir_path);
	#endif*/
	
	
	// Open the directory
	dirp = opendir( dir_path );
  	if (!dirp) {
  		fprintf(stderr, "Error: Can't open directory: %s\n", dir_path);
		return;
  	}

	while( (dent = readdir(dirp)) )
	{
		char* fname = dent->d_name;
		
		// Has it got the right length, prefix and suffix ?
		if (strlen(fname) == prefix_len+4+suffix_len &&
		    strncmp(fname, PLUGIN_PREFIX, prefix_len)==0 &&
			strncasecmp(fname+(strlen(fname)-suffix_len), PLUGIN_SUFFIX, suffix_len)==0)
		{
			char plugin_code[4];
			oqt_codec_info_t *codec_info;
			memcpy(plugin_code, fname+prefix_len, 4);
			
			{
				void *handle = NULL;
				char* full_path = malloc(strlen(dir_path)+strlen(fname)+2);
				sprintf(full_path, "%s/%s", dir_path, fname);
				
				/*#ifdef DEBUG
				fprintf(stderr, "Loading plugin: %s\n", full_path);
				#endif*/
				
				// Open the plugin and call the register function
				handle = open_plugin(full_path);
				codec_info = call_plugin_regfunc(handle);
				
				if (codec_info==NULL) {
					fprintf(stderr, "Failed to load plugin %s.\n", full_path);
					if (handle) oqt_close_plugin(handle);
				} else { 
					char temp_code[4];
					memcpy(temp_code, codec_info->fourcc, 4);
					convert_code(temp_code);
				
					// Verify that the code registered matched the filename
					if (strncmp(plugin_code, temp_code, 4)) {
						fprintf(stderr, "Warning code registered [%.4s/%.4s] did not match filename [%.4s].\n",
							codec_info->fourcc, temp_code, plugin_code);
					}
				}
				
				// Free the path string
				free(full_path);
			}
		}
		
	}
	
	closedir(dirp);
#endif
#endif
}

void oqt_load_all_plugins()
{
	static char loaded = 0;
	
	if (!loaded) {

	#ifndef WIN32	// sorry plugin disable under windows for the moment
	#ifndef __MACOS__	/* and __MACOS__ */
		
		char *env_path = getenv( ENV_PLUGPATH );
		
		
		// Look in the directories in the environment varible
		if (env_path) {
			int envlen = strlen(env_path);
			int i, dirlen=0;
			char *start = env_path;
		
			/*#ifdef DEBUG
			fprintf(stderr, "Found Environment variable: %s.\n", env_path);
			#endif*/
			
			for(i=0;i<=envlen;i++) {
				if (env_path[i] == ':' || i==envlen) {
					char *dir_path = malloc(dirlen+1);
					
					// Remove white space and slashes from the end
					while((isspace(start[dirlen-1]) || start[dirlen-1] == '/')
						  && dirlen>0) --dirlen;
						
					// Remove white space from the start
					while(isspace(start[0]) && dirlen>0) {
						start = &start[1];
						--dirlen;
					}
					
					if (dirlen > 0) {
						// Copy the memory into the temporary string
						memcpy(dir_path, start, dirlen);
						
						// Terminate the string
						dir_path[dirlen]=0;
						
						// Search the directory path we found
						search_in_directory(dir_path);
					} else {
						#ifdef DEBUG
						//fprintf(stderr, "Got zero lengthed directory.\n");
						#endif
					}
					
					// Reset the start and length of the string
					start = &env_path[i+1];
					dirlen=0;
					free(dir_path);
				} else {
					dirlen++;
				}
			}
		}
		
		
		// Look in the default install directory
		search_in_directory(OQT_PLUGIN_DIR);
		
		loaded=1;
	#endif
	#endif
	}
}


int oqt_close_plugin(void* handle)
{
	if (handle) {

#ifndef WIN32
#ifndef __MACOS__
	dlclose(handle);
#else
	/* __MACOS__ */
#endif
#else
	/* WIN32 */
	FreeLibrary((HMODULE)handle);
#endif

	} else {
		fprintf(stderr, "Error in oqt_close_plugin: handle is null\n");
	}

	//*** Check for errors ***
	//*** Return value ?? ***
	return 0;
}

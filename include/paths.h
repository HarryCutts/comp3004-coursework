#ifndef _PATHS_H
#define _PATHS_H

/** @file paths.h
 * Defines macros which expand to asset paths, to allow easier flattening of
 * the directory structure for handin.
 *
 * To compile the paths using a directory structure, define `ASSET_DIRECTORIES`
 * at compile time.
 */

#ifdef ASSET_DIRECTORIES 
	#define SEPARATOR "/"

	#define SHADER(name)  ("shaders"  SEPARATOR name)
	#define TEXTURE(name) ("textures" SEPARATOR name)
	#define MODEL(name)   ("models"   SEPARATOR name)
#else
	#define SHADER(name)  (name)
	#define TEXTURE(name) (name)
	#define MODEL(name)   (name)
#endif


#endif

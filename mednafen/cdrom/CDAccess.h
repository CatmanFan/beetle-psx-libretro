#ifndef __MDFN_CDROMFILE_H
#define __MDFN_CDROMFILE_H

#include <stdio.h>
#include <stdint.h>

#include "CDUtility.h"
#include "misc.h"

class CDAccess
{
 public:

 CDAccess();
 virtual ~CDAccess();

 virtual void Read_Raw_Sector(uint8_t *buf, int32_t lba) = 0;

 virtual void Read_TOC(TOC *toc) = 0;

 virtual void Eject(bool eject_status) = 0;		// Eject a disc if it's physical, otherwise NOP.  Returns true on success(or NOP), false on error

 private:
 CDAccess(const CDAccess&);	// No copy constructor.
 CDAccess& operator=(const CDAccess&); // No assignment operator.
};

CDAccess *cdaccess_open_image(const char *path, bool image_memcache);

#endif

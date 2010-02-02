/*************************************************************************\
 Engine: gearWorks
 File: gwVram.h
 Author: Zachry Thayer
 Description: Manages Vram
 \*************************************************************************/

#ifndef __GWVRAM_H__
#define __GWVRAM_H__

#ifdef __cplusplus
extern "C" {
#endif

void* gwVramRelativePointer(void *ptr);

void* gwVramAbsolutePointer(void *ptr);

void* gwVramAlloc(unsigned long size);

void gwVramFree(void* ptr);


unsigned long gwVramAvailable();

unsigned long gwVramLargestBlock();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif

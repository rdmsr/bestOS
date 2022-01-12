#ifndef KERNEL_PIC_H
#define KERNEL_PIC_H

void pic_initialize(void);
void pic_eoi(int intno);

#endif
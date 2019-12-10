
#include "bootpack.h"

/*PIC�ĳ�ʼ��*/

void init_pic(void)
{
	
	io_out8(PIC0_IMR,  0xff  ); /* ��ֹ�����ж� */
	io_out8(PIC1_IMR,  0xff  ); /* ��ֹ�����ж� */

	io_out8(PIC0_ICW1, 0x11  ); /* ���ش���ģʽ��edge trigger mode��*/
	io_out8(PIC0_ICW2, 0x20  ); /* IRQ0-7��INT20-27���� */
	io_out8(PIC0_ICW3, 1 << 2); /* PIC1��IRQ2����*/
	io_out8(PIC0_ICW4, 0x01  ); /* �޻�����ģʽ*/

	io_out8(PIC1_ICW1, 0x11  ); /* ���ش���ģʽ*/
	io_out8(PIC1_ICW2, 0x28  ); /* IRQ8-15��INT28-2f���� */
	io_out8(PIC0_ICW3, 2     ); /* PIC1��IRQ2����*/
	io_out8(PIC0_ICW4, 0x01  ); /* �޻�����ģʽ*/

	io_out8(PIC0_IMR,  0xfb  ); /* 11111011 PIC1����ȫ����ֹ*/
	io_out8(PIC1_IMR,  0xff  ); /* 1111111 ��ֹ�����ж�*/

	return;
}

#define PORT_KEYDAT 0x0060

struct FIFO8 keyfifo;

void inthandler21(int *esp)
{
	unsigned char data;
	io_out8(PIC0_OCW2, 0x61);	/* IRQ-01�ܸ����ˤ�PIC��֪ͨ */
	data = io_in8(PORT_KEYDAT);
	fifo8_put(&keyfifo, data);
	return;
}

void inthandler2c(int *esp)
/* ����PS/2�����ж� */
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 0, 32 * 8 - 1, 15);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "INT 2C (IRQ-12) : PS/2 mouse");
	for (;;) {
		io_hlt();
	}
}

void inthandler27(int *esp)
/* PIC0�жϵĲ��������� */
/* ����ж���Athlon64X2��ͨ��оƬ���ṩ�ı�����ֻ��ִ��һ�� */
/* ����ж�ֻ�ǽ��գ���ִ���κβ��� */
/* Ϊʲô��������
	��  ��Ϊ����жϿ����ǵ������������ġ�ֻ�Ǵ���һЩ��Ҫ�������*/
{
	io_out8(PIC0_OCW2, 0x67); /* ֪ͨPIC��IRQ-07���ο�7-1�� */
	return;
}
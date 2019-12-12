/* �����*/

#include "bootpack.h"

#define SHEET_USE		1

/*���ʼ��*/
struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize)
{
	struct SHTCTL *ctl;
	int i;
	ctl = (struct SHTCTL *) memman_alloc_4k(memman, sizeof (struct SHTCTL));
	if (ctl == 0) {
		goto err;
	}
	ctl->vram = vram;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	ctl->top = -1; /* һ��SHEET��û�� */
	for (i = 0; i < MAX_SHEETS; i++) {
		ctl->sheets0[i].flags = 0; /* ���Ϊδʹ�� */
	}
err:
	return ctl;
}

struct SHEET *sheet_alloc(struct SHTCTL *ctl)
{/*ȡ��δʹ�õ�����ͼ��*/
	struct SHEET *sht;
	int i;
	for (i = 0; i < MAX_SHEETS; i++) {
		if (ctl->sheets0[i].flags == 0) {
			sht = &ctl->sheets0[i];
			sht->flags = SHEET_USE; /* ���Ϊ����ʹ�� */
			sht->height = -1; /* ���� */
			return sht;
		}
	}
	return 0;	/* ���е�SHEET����������ʹ��״̬*/
}

/*�趨ͼ�㻺������С��͸��ɫ*/
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv)
{
	sht->buf = buf;
	sht->bxsize = xsize;
	sht->bysize = ysize;
	sht->col_inv = col_inv;
	return;
}
/*�ֲ�ˢ�º���*/
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1)
{
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	unsigned char *buf, c, *vram = ctl->vram;
	struct SHEET *sht;
	/* ���refresh�ķ�Χ�����˻������������ */
	if (vx0 < 0) { vx0 = 0; }
	if (vy0 < 0) { vy0 = 0; }
	if (vx1 > ctl->xsize) { vx1 = ctl->xsize; }
	if (vy1 > ctl->ysize) { vy1 = ctl->ysize; }
	for (h = 0; h <= ctl->top; h++) {
		sht = ctl->sheets[h];
		buf = sht->buf;
		/* ʹ��vx0~vx1,��bx0~bx1���е��� */
		bx0 = vx0 - sht->vx0;
		by0 = vy0 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		by1 = vy1 - sht->vy0;
		if (bx0 < 0) { bx0 = 0; }
		if (by0 < 0) { by0 = 0; }
		if (bx1 > sht->bxsize) { bx1 = sht->bxsize; }
		if (by1 > sht->bysize) { by1 = sht->bysize; }
		for (by = by0; by < by1; by++) {
			vy = sht->vy0 + by;
			for (bx = bx0; bx < bx1; bx++) {
				vx = sht->vx0 + bx;
				c = buf[by * sht->bxsize + bx];
				if (c != sht->col_inv) {
					vram[vy * ctl->xsize + vx] = c;
				}
			}
		}
	}
	return;
}

/*�趨�װ�߶�*/
void sheet_updown(struct SHTCTL *ctl, struct SHEET *sht, int height)
{
	int h, old = sht->height; /* �洢����ǰ�ĸ߶���Ϣ*/

	/* ���ָ���ĸ߶ȹ��߻����, ���������*/
	if (height > ctl->top + 1) {
		height = ctl->top + 1;
	}
	if (height < -1) {
		height = -1;
	}
	sht->height = height; /* �趨�߶�*/

	/* ������Ҫ�ǽ���sheet[]����������*/
	if (old > height) {	/* ����ǰ��*/
		if (height >= 0) {
			/* ���м�������� */
			for (h = old; h > height; h--) {
				ctl->sheets[h] = ctl->sheets[h - 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		} else {	/* ���� */
			if (ctl->top > old) {
				/* ������Ľ�����*/
				for (h = old; h < ctl->top; h++) {
					ctl->sheets[h] = ctl->sheets[h + 1];
					ctl->sheets[h]->height = h;
				}
			}
			ctl->top--; /* ������ʾ�е�ͼ�������һ���������������ͼ��߶��½� */
		}
		sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize); /* ������ͼ�����Ϣ���»��ƻ��� */
	} else if (old < height) {	/* ����ǰ��*/
		if (old >= 0) {
			/*���м������ȥ */
			for (h = old; h < height; h++) {
				ctl->sheets[h] = ctl->sheets[h + 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		} else {	/* ������״̬ת��Ϊ��ʾ״̬ */
			/* ����������������� */
			for (h = ctl->top; h >= height; h--) {
				ctl->sheets[h + 1] = ctl->sheets[h];
				ctl->sheets[h + 1]->height = h + 1;
			}
			ctl->sheets[height] = sht;
			ctl->top++; /* ������ʾ��ͼ��������1���������������ͼ��߶�����*/
		}
		sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize); /* ������ͼ����Ϣ���»��ƻ���*/
	}
	return;
}
/*ͼ��ˢ��*/
void sheet_refresh(struct SHTCTL *ctl, struct SHEET *sht, int bx0, int by0, int bx1, int by1)
{
	if (sht->height >= 0) { /* ���������ʾ��������ͼ�����Ϣˢ�»���*/ 
		sheet_refreshsub(ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1);
	}
	return;
}
/*���������ƶ�ͼ��*/
void sheet_slide(struct SHTCTL *ctl, struct SHEET *sht, int vx0, int vy0)
{
	int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
	sht->vx0 = vx0;
	sht->vy0 = vy0;
	if (sht->height >= 0) { /* ���������ʾ��������ͼ�����Ϣˢ�»���*/
		sheet_refreshsub(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize);
		sheet_refreshsub(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize);
	}
	return;
}
/*�ͷ���ʹ�õ�ͼ���ڴ�*/
void sheet_free(struct SHTCTL *ctl, struct SHEET *sht)
{
	if (sht->height >= 0) {
		sheet_updown(ctl, sht, -1); /* ���������ʾ״̬�������趨Ϊ����*/
	}
	sht->flags = 0; /* ��δʹ�á���ʶ*/
	return;
}
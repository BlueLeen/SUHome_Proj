#ifndef _BUFFER_SEAL__0X20140424103730
#define _BUFFER_SEAL__0X20140424103730

#include<stdio.h>

void* bufp[1024] = {NULL};


typedef struct _bufBlock
{
	void* data;
	int size;
	void* next;
}BlkBuf;

BlkBuf* AllocBlkBuf(void* data, int len)
{
	BlkBuf* one = (BlkBuf*)malloc(sizeof(BlkBuf));
	one->next = (void*)NULL;
	void* temp = (void*)malloc(len);
	memcpy(temp, data, len);
	one->data = temp;
	one->size = len;
	return one;
}

void* AllocMem(void* buf, int size, void* base)
{
	if(base != NULL)
	{
		BlkBuf* bp = (BlkBuf*)base;
		while(bp->next)
		{
			bp = bp->next;
		}
		BlkBuf* last = AllocBlkBuf(buf, size);
		bp->next = last;
	}
	else
	{
		int i;
		for(i=0; i<1024; i++)
		{
			if(bufp[i] == NULL)
			{
				BlkBuf* last = AllocBlkBuf(buf, size);
				bufp[i] = last;
				base = bufp[i];
				break;
			}
		}
	}
	return base;
}

void FreeMem(void* base)
{
	if(base != NULL)
	{
		BlkBuf* bp = (BlkBuf*)base;
		while(bp->next)
		{
			BlkBuf* temp = bp;
			bp = bp->next;
			free(temp->data);
			free(temp);
		}
		base = NULL;
	}
}

#endif

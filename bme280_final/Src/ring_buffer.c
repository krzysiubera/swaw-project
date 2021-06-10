/* Includes ------------------------------------------------------------------*/
#include <assert.h>
#include "ring_buffer.h"


bool RingBuffer_Init(RingBuffer *ringBuffer, struct bme280_pkt *dataBuffer, size_t dataBufferSize)
{
	assert(ringBuffer);
	assert(dataBuffer);
	assert(dataBufferSize > 0);

	if ((ringBuffer) && (dataBuffer) && (dataBufferSize > 0)) {
		*ringBuffer = (RingBuffer) {
			.buf = dataBuffer,
			.head = dataBuffer,
			.tail = dataBuffer,
			.cap = dataBufferSize
		};
		RingBuffer_Clear(ringBuffer);
		return true;
	}
	return false;
}

bool RingBuffer_Clear(RingBuffer *ringBuffer)
{
	assert(ringBuffer);

	if (ringBuffer) {
		//for (uint16_t i = 0; i < ringBuffer->cap; i++)
		//	ringBuffer->buf[i] = 0;
		memset(ringBuffer->buf, 0, sizeof(struct bme280_pkt) * ringBuffer->cap);
		ringBuffer->count = 0;
		return true;
	}
	return false;
}

bool RingBuffer_IsEmpty(const RingBuffer *ringBuffer)
{
  assert(ringBuffer);
	if (ringBuffer->count == 0)
		return true;
	return false;
}

size_t RingBuffer_GetLen(const RingBuffer *ringBuffer)
{
	assert(ringBuffer);

	if (ringBuffer) {
		return ringBuffer->count;
	}
	return 0;

}

size_t RingBuffer_GetCapacity(const RingBuffer *ringBuffer)
{
	assert(ringBuffer);

	if (ringBuffer) {
		return ringBuffer->cap;
	}
	return 0;
}


bool RingBuffer_PutPkt(RingBuffer *ringBuffer, struct bme280_pkt data)
{
	assert(ringBuffer);

	//if (ringBuffer /*&& ringBuffer->count < ringBuffer->cap*/) {
		/*if(ringBuffer->count < ringBuffer->cap)
		{
			ringBuffer->count++;
		}
		*ringBuffer->head++ = data;
		if (ringBuffer->head == ringBuffer->buf + ringBuffer->cap)
			ringBuffer->head = ringBuffer->buf;
		return true;
	}*/
	if (ringBuffer) {
		if (ringBuffer->head < ringBuffer->buf + ringBuffer->cap)
		{
			*ringBuffer->head++ = data;
		}
		else
		{
			ringBuffer->head = ringBuffer->buf;
			*ringBuffer->head = data;
		}
		if(ringBuffer->count < ringBuffer->cap)
		{
			ringBuffer->count++;
		}
		return true;
	}
	return false;
}

bool RingBuffer_GetPkt(RingBuffer *ringBuffer, struct bme280_pkt *data)
{
	assert(ringBuffer);
	assert(data);
	/*if ((ringBuffer) && (data) && ringBuffer->tail >= ringBuffer->buf) {
		//ringBuffer->count--;
		*data = *ringBuffer->tail++;
		if (ringBuffer->tail == ringBuffer->buf + ringBuffer->cap)
			ringBuffer->tail = ringBuffer->buf;
		return true;
	}*/
	if ((ringBuffer) && (data)) {
		if (!RingBuffer_IsEmpty(ringBuffer))
		{
			if(ringBuffer->count < ringBuffer->cap)
			{
				if (ringBuffer->tail < ringBuffer->buf + ringBuffer->count)
				{
					*data = *ringBuffer->tail++;
				}
				else
				{
					ringBuffer->tail = ringBuffer->buf;
					*data = *ringBuffer->tail;
				}
			}
			else
			{
				if (ringBuffer->tail < ringBuffer->buf + ringBuffer->cap)
				{
					*data = *ringBuffer->tail++;
				}
				else
				{
					ringBuffer->tail = ringBuffer->buf;
					*data = *ringBuffer->tail;
				}
			}
	    	return true;
		}
	}
	return false;
}
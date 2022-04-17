/* Includes ------------------------------------------------------------------*/
#include "ring_buffer.h"


bool RingBuffer_Init(RingBuffer *ringBuffer, float *dataBuffer, size_t dataBufferSize)
{
	if ((ringBuffer) && (dataBuffer) && (dataBufferSize > 0)) {
	    // set all parameters to default values or values we got as parameters of function
	    ringBuffer->data = dataBuffer;
	    ringBuffer->head = 0;
	    ringBuffer->tail = 0;
	    ringBuffer->capacity = dataBufferSize;
	    ringBuffer->countElements = 0;
	    return true;
	}

	return false;
}

bool RingBuffer_Clear(RingBuffer *ringBuffer)
{
	if (ringBuffer) {
	    // set appropriate values to default
	    ringBuffer->head = 0;
	    ringBuffer->tail = 0;
	    ringBuffer->countElements = 0;

	    return true;
	}
	return false;
}

bool RingBuffer_IsEmpty(const RingBuffer *ringBuffer)
{
	if (ringBuffer->countElements == 0)
	    return true;
	else
	    return false;
}

size_t RingBuffer_GetLen(const RingBuffer *ringBuffer)
{
	if (ringBuffer) {

		return (ringBuffer->countElements);

	}
	return 0;

}

size_t RingBuffer_GetCapacity(const RingBuffer *ringBuffer)
{
	if (ringBuffer) {
		return ringBuffer->capacity;
	}
	return 0;
}


bool RingBuffer_PutFloat(RingBuffer *ringBuffer, float val)
{
	if (ringBuffer) {

		// we check if buffer is full
		if ((ringBuffer->countElements) == (ringBuffer->capacity))
		{
		    // cannot put char if buffer is full
		    return false;
		}


		// add item to location
	    ringBuffer->data[ringBuffer->head] = val;

	    // adjust head location by one
	    ringBuffer->head = (ringBuffer->head + sizeof(float)) % (ringBuffer->capacity);

	    // increase number of elements in buffer
	    (ringBuffer->countElements)++;
	    return true;

	}
	return false;
}

bool RingBuffer_GetFloat(RingBuffer *ringBuffer, float *val)
{
  if ((ringBuffer) && (val)) {

		// we check if buffer is empty
		if (RingBuffer_IsEmpty(ringBuffer))
		{
		    // cannot get char from empty buffer
		    return false;
		}

		// get item from data buffer
		*val = ringBuffer->data[ringBuffer->tail];

		// adjust tail position
		ringBuffer->tail = (ringBuffer->tail + sizeof(float)) % (ringBuffer->capacity);

		// decrease number of elements in buffer
		(ringBuffer->countElements)--;

		return true;

	}

	return false;
}

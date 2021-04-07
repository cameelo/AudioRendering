#pragma once
#include <memory>

template <class T>
class CircularBuffer {
public:
	T * buffer;
	//tail points to the last added element
	size_t size, head, tail;
public:
	CircularBuffer(size_t size) {
		this->buffer = new T[size]; //size no debe estar en bytes sino en la cantidad de elementos de tipo T del buffer
		this->size = size;
		head = 0;
		tail = 0;
	}
	void insert(T * source, size_t size) {
		//First check if insert has to be done in two steps
		if (this->tail + size >= this->size) {
			size_t first_cpy_size = this->size - 1 - this->tail;
			memcpy(&this->buffer[this->tail + 1], source, first_cpy_size*sizeof(T));
			memcpy(this->buffer, &source[first_cpy_size], (size - first_cpy_size)*sizeof(T));
			this->tail = size - first_cpy_size - 1;
			this->head = (this->tail + 1) % this->size;
		}
		else { 
			memcpy(&this->buffer[this->tail + 1], source, size*sizeof(T));
			size_t new_tail = this->tail + size;
			//If buffer was full we need to move the head
			if (this->tail < this->head) {
				this->head = (new_tail + 1) % this->size;
			}
			this->tail = new_tail;
		}
	}
	T& operator[](size_t idx){
		return this->buffer[idx];
	}
	~CircularBuffer() {
		delete[](this->buffer);
	}
};
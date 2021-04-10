#pragma once
#include <memory>

template <class T>
class CircularBuffer {
public:
	T * buffer;
	//tail points to the last added element
	size_t size, head, tail;
	bool full;
public:
	CircularBuffer(size_t size) {
		this->buffer = new T[size]; //size no debe estar en bytes sino en la cantidad de elementos de tipo T del buffer
		this->size = size;
		this->head = 0;
		this->tail = 0;
		this->full = false;
	}
	void insert(T * source, size_t size) {
		//First check if insert has to be done in two steps
		if (this->tail + size >= this->size) {
			size_t first_cpy_size = this->size - 1 - this->tail;
			memcpy(&this->buffer[this->tail + 1], source, first_cpy_size*sizeof(T));
			memcpy(this->buffer, &source[first_cpy_size], (size - first_cpy_size)*sizeof(T));
			this->tail = size - first_cpy_size;
			this->head = (this->tail + 1) % this->size;
			this->full = true;
		}
		else { 
			if (tail == head) {
				memcpy(&this->buffer[this->tail], source, size * sizeof(T));
			}
			else {
				memcpy(&this->buffer[this->tail + 1], source, size * sizeof(T));
			}
			size_t new_tail = this->tail + size - 1;
			//If buffer was full we need to move the head
			if (this->full) {
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
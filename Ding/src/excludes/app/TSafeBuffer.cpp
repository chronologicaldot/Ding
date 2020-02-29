// (c) 2019 Nicolaus Anderson

#include <TSafeBuffer.h>
#include <lock_guard>

namespace threading {

TSafeBuffer::TSafeBuffer()
	: Buffer()
{}

TSafeBuffer::~TSafeBuffer() {
}

TSafeBuffer::ptr_type
TSafeBuffer::access() {
	Mutex.lock();
	return Buffer;
}

void
TSafeBuffer::set( TSafeBuffer::ptr_type&  NewValue ) {
	std::lock_guard  Guard(Mutex);
	Buffer = NewValue;
}

void
TSafeBuffer::release() {
	Mutex.unlock();
}

} // end namespace threading

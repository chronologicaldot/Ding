// (c) 2019 Nicolaus Anderson

#ifndef TSAFE_BUFFER_H
#define TSAFE_BUFFER_H

#include <memory>
#include <vector>
#include <mutex>

namespace threading {

//! Thread Safe Buffer
template<class ElementType>
struct TSafeBuffer {

	typedef  ElementType  value_type;
	typedef  std::vector<value_type>  buffer_type;
	typedef  std::shared_ptr<buffer_type>  ptr_type;

	TSafeBuffer();
	~TSafeBuffer();

	ptr_type  access();
	void  set( ptr_type& );
	void  release();

private:
	ptr_type  Buffer;
	std::mutex  Mutex;
};

} // end namespace util

#endif // TSAFE_BUFFER_H

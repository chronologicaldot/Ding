// (c) 2019 Nicolaus Anderson

#ifndef THREAD_DIODE_H
#define THREAD_DIODE_H

#include <memory>
#include <mutex>
#include <lock_guard>

namespace threading {

template<class T>
struct RsrcSwap {

	typedef  T  value_type;
	typedef  std::shared_ptr<T>  ptr;

	void  setA( ptr&  t ) {
		std::lock_guard<> g(m);
		a = t;
	}

	void  setB( ptr&  t ) {
		std::lock_guard<> g(m);
		b = t;
	}

	ptr&  getA() {
		std::lock_guard<> g(m);
		return a;
	}

	ptr&  getB() {
		std::lock_guard<> g(m);
		return b;
	}

	void  swap() {
		std::lock_guard<> g(m);
		ptr  c = b;
		b = a;
		a = c;
	}

private:
	ptr  a;
	ptr  b;
	std::mutex  m;
};

} // end namespace threading

#endif // THREAD_DIODE_H


// ThreadDiode< Table >  tableDiode;
// tableDiode.set(  make_shared<Table>() );


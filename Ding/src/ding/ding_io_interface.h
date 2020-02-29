// (c) 2019 Nicolaus Anderson

#ifndef DING_IO_INTERFACE
#define DING_IO_INTERFACE

#include <sys/types.h> // For size_t

namespace ding {

struct IOInterface {

	//! Writing
	virtual bool  beginWrite() = 0;
	virtual bool  writeSection( const char*  name ) = 0;
	virtual void  addStringAttribute( const char*  name, const char* ) = 0;
	virtual void  addIntAttribute( const char*  name, int ) = 0;
	virtual void  addDoubleAttribute( const char*  name, double ) = 0;
	virtual void  addBoolAttribute( const char*  name, bool ) = 0;
	virtual void  endWriteAttributes() = 0;
	virtual bool  endWriteSection( const char*  checkName = nullptr ) = 0; // Would have to save stack of names, but that's possible. Could also prevent ill-formated files because I might forget to call endSection() somewhere else (i.e. have it perform auto-close).
	virtual bool  endWrite() = 0;

	//! Reading
	virtual bool  beginRead() = 0;
	virtual bool  readSection( const char*  name ) = 0;
	virtual const char*  getAttributeAsString( const char*  name ) = 0;
	virtual int  getAttributeAsInt( const char*  name ) = 0;
	virtual double  getAttributeAsDouble( const char*  name ) = 0;
	virtual bool  getAttributeAsBool( const char*  name ) = 0;
	virtual size_t  getChildNodeCount( const char*  name = nullptr ) = 0;
	virtual bool  endReadSection() = 0;
	virtual bool  endRead() = 0;

};

struct IOSerializable {
	//! Write to IO Interface
	virtual void  serialize( IOInterface& ) = 0;

	//! Read from IO Interface
	virtual void  deserialize( IOInterface& ) = 0;
};

} // end namespace ding

#endif // DING_IO_INTERFACE

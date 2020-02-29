// (c) 2019 Nicolaus Anderson

#ifndef APP_IO_INTERFACE
#define APP_IO_INTERFACE

#include "../ding/ding_io_interface.h"
#include "../ding/ding_bell_maker.h"
#include <vector>
#include <QString>
#include <QFile>
#include <QXmlDefaultHandler>
#include <QXmlAttributes>

// Duplicate of ding_types.h version
template<class T>
using  list_t = std::vector<T>;

//! IO Unit
/*
	Utilizes QXmlSimpleReader
	https://doc.qt.io/qt-5/qxmlsimplereader.html
	Implements the QXmlContentHandler of QXMLDefaultHandler
	https://doc.qt.io/qt-5/qxmlcontenthandler.html
*/
struct  IOUnit
	: public ding::IOInterface
	, public QXmlDefaultHandler
{
	//! Data Section Header Attribute
	struct Attribute
	{
		Attribute( const QString& );
		void		setStringValue( const QString& );
		const char*	getCharPtrValue() const;
		void		setIntValue( int );
		int			getIntValue() const;
		void		setDoubleValue( double );
		double		getDoubleValue() const;
		void		setBoolValue( bool );
		bool		getBoolValue() const;

		QString  Name;

	private:
		QString  Value;
	};

	//! Data Section
	struct Section
	{
		Section( const QString& );
		Section( const Section& );
		~Section(); // For deferencing sections
		void		ref();
		void		deref();
		Attribute&	getAttribute( const QString& );
		void		setAttributesFrom( const QXmlAttributes& );
		Section*	getSubsectionByName( const QString& );
		Section*	getSubsection( size_t );
		size_t		getSubsectionCount() const;
		void		appendSubsection( Section* );

		QString  Name;
		bool  HasBeenRead;
		list_t< Attribute >  Attributes;
		list_t< Section* >  Subsections;
	private:
		int  refCount;
	};

	// Public Methods -----------------------
	IOUnit();
	void  clear();

	// Ding IOInterface begin ---------------

	//! Writing
	virtual bool  beginWrite();
	virtual bool  writeSection( const char*  name );
	virtual void  addStringAttribute( const char*  name, const char* );
	virtual void  addIntAttribute( const char*  name, int );
	virtual void  addDoubleAttribute( const char*  name, double );
	virtual void  addBoolAttribute( const char*  name, bool );
	virtual void  endWriteAttributes();
	virtual bool  endWriteSection( const char*  checkName );
	virtual bool  endWrite();

	//! Reading
	virtual bool  beginRead();
	virtual bool  readSection( const char*  name );
	virtual const char*  getAttributeAsString( const char*  name );
	virtual int  getAttributeAsInt( const char*  name );
	virtual double getAttributeAsDouble( const char*  name );
	virtual bool  getAttributeAsBool( const char*  name );
	virtual size_t  getChildNodeCount( const char*  name );
	virtual bool  endReadSection();
	virtual bool  endRead();

	// QT XMLDefaultHandler begin --------------
	//virtual bool  attributeDecl(const QString&, const QString&, const QString&, const QString&, const QString&) override;
	virtual bool  endDocument() override;
	virtual bool  endElement(const QString&, const QString&, const QString&) override;
	virtual bool  startElement(const QString&, const QString&, const QString&, const QXmlAttributes&) override;
	virtual bool  startDocument() override;

		// Error Handling ----------------------
	virtual QString  errorString() const;
	virtual bool  error(const QXmlParseException&  exception);
	virtual bool  fatalError(const QXmlParseException&  exception);
	virtual bool  warning(const QXmlParseException&  exception);

	// Usage interface begin -------------------
	bool  load( QFile&, ding::IOSerializable& );
	const QString  save( ding::IOSerializable& );

protected:
	bool  parse( QFile& );
	const QString  compose();
	void  pushWriteSection( const QString& );
	const QString  getCurrentWriteSection();
	void  increasePadding();
	void  reducePadding();
	void  commitWrite( const QString&  text, bool pad = true );

private:
	// Members ------------------------------
	QString				errorMessage;

	// Writing -------------------
	QString				output;
	list_t< QString >	sectionNames;
	QString				padding;

	// Reading -------------------
	Section		readRoot;
	Section*	readCurrent;
	list_t< Section* >  readSections;
	Section*	builtCurrent;
	list_t< Section* >  builtSections;
};

#endif // APP_IO_UNIT

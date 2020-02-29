// (c) 2019 Nicolaus Anderson

#include "io_unit.h"
#include <QXmlSimpleReader>
#include <QXmlInputSource>

IOUnit::Attribute::Attribute( const QString&  name )
	: Name( name )
	, Value("")
{}

void
IOUnit::Attribute::setStringValue( const QString&  v ) {
	Value = v;
}

const char*
IOUnit::Attribute::getCharPtrValue() const {
	return Value.toStdString().c_str();
}

void
IOUnit::Attribute::setIntValue( int  v ) {
	Value = QString("%1").arg(v);
}

int
IOUnit::Attribute::getIntValue() const {
	return Value.toInt();
}

void
IOUnit::Attribute::setDoubleValue( double  v ) {
	Value = QString("%1").arg(v);
}

double
IOUnit::Attribute::getDoubleValue() const {
	return Value.toDouble();
}

void
IOUnit::Attribute::setBoolValue( bool  v ) {
	if ( v )
		Value = QString("true");
	else
		Value = QString("false");
}

bool
IOUnit::Attribute::getBoolValue() const {
	if ( Value == "true" )
		return true;
	else
		return false;
}

//---------------------------------------------------------
IOUnit::Section::Section( const QString&  name )
	: Name( name )
	, HasBeenRead( false )
	, Attributes()
	, Subsections()
	, refCount(1)
{}

IOUnit::Section::Section( const Section&  other )
	: Name(other.Name)
	, Attributes(other.Attributes)
	, Subsections(other.Subsections)
{
	for ( Section*  s : Subsections )
		s->ref();
}

IOUnit::Section::~Section() {
	// if ( refCount != 1 ) error

	for ( Section*  s : Subsections )
		s->deref();
}

void
IOUnit::Section::ref() {
	++refCount;
}

void
IOUnit::Section::deref() {
	--refCount;
	//if ( refCount < 0 )
	//	throw BadReferenceCount;
	if ( refCount == 0 )
		delete this;
}

IOUnit::Attribute&
IOUnit::Section::getAttribute( const QString&  name ) {
	for ( Attribute&  a : Attributes ) {
		if ( a.Name == name )
			return a;
	}
	Attributes.push_back( Attribute{name} );
	return Attributes.back();
}

void
IOUnit::Section::setAttributesFrom( const QXmlAttributes&  attrs ) {
	int  a = 0;
	for (; a < attrs.count(); ++a) {
		const QString  aName = attrs.qName(a);
		const QString  aValue = attrs.value(aName);
		Attributes.push_back( Attribute{aName} );
		Attributes.back().setStringValue(aValue);
	}
}

IOUnit::Section*
IOUnit::Section::getSubsectionByName( const QString&  name ) {
	for ( Section*  s : Subsections ) {
		if ( s->Name == name )
			return s;
	}
	Subsections.push_back( new Section{name} );
	return Subsections.back();
}

IOUnit::Section*
IOUnit::Section::getSubsection( size_t  index ) {
	if ( index < Subsections.size() )
		return Subsections[index];

	return nullptr;
}

size_t
IOUnit::Section::getSubsectionCount() const {
	return Subsections.size();
}

void
IOUnit::Section::appendSubsection( Section*  s ) {
	Subsections.push_back( s );
}

//---------------------------------------------------------
IOUnit::IOUnit()
	: errorMessage()
	, output()
	, sectionNames()
	, padding("")
	, readRoot( QString("root") )
	, readCurrent( nullptr )
	, readSections()
	, builtCurrent( nullptr )
	, builtSections()
{}

void
IOUnit::clear() {
	sectionNames.clear();
}

bool
IOUnit::beginWrite() {
	commitWrite( QString("<?xml version=\"1.0\" encoding=\"iso-8859-1\" ?>\n"), false );
	return true;
}

bool
IOUnit::writeSection( const char*  name ) {
	commitWrite( QString("<%1 ").arg(QString(name)) );
	pushWriteSection(QString(name));
	increasePadding();
	return true;
}

void
IOUnit::addStringAttribute( const char*  name, const char*  value ) {
	commitWrite( QString("%1=\"%2\" ").arg(name).arg(value), false );
}

void
IOUnit::addIntAttribute( const char*  name, int  value ) {
	commitWrite( QString("%1=\"%2\" ").arg(name).arg(value), false );
}

void
IOUnit::addDoubleAttribute( const char*  name, double  value ) {
	commitWrite( QString("%1=\"%2\" ").arg(name).arg(value), false );
}

void
IOUnit::addBoolAttribute( const char*  name, bool  value ) {
	QString  out = (value ? "true" : "false");
	commitWrite( QString("%1=\"%2\" ").arg(name).arg(out), false );
}

void
IOUnit::endWriteAttributes() {
	commitWrite( QString(">\n"), false );
	//increasePadding();
}

bool
IOUnit::endWriteSection( const char*  checkName ) {
	reducePadding();
	commitWrite( QString("</%1>\n").arg(getCurrentWriteSection()) );
	bool  check = getCurrentWriteSection() == QString(checkName);
	sectionNames.pop_back();
	return (checkName? check : true);
}

bool
IOUnit::endWrite() {
	return sectionNames.size() == 0;
}

bool
IOUnit::beginRead() {
	readCurrent = &readRoot;
	readSections.push_back(readCurrent);
	return true;
}

bool
IOUnit::readSection( const char*  name ) {
	QString  qName(name);
	// Search the current section node for a node with the given name
	for ( Section*  s : readCurrent->Subsections ) {
		if ( s->Name == qName && ! s->HasBeenRead ) {
			readCurrent = s;
			readSections.push_back(readCurrent);
			readCurrent->HasBeenRead = true;
			return true;
		}
	}
	return false;
}

const char*
IOUnit::getAttributeAsString( const char*  name ) {
	if ( !readCurrent )
		return "";
	return readCurrent->getAttribute(QString(name)).getCharPtrValue();
}

int
IOUnit::getAttributeAsInt( const char*  name ) {
	if ( !readCurrent )
		return 0;
	return readCurrent->getAttribute(QString(name)).getIntValue();
}

double
IOUnit::getAttributeAsDouble( const char*  name ) {
	if ( !readCurrent )
		return 0;
	return readCurrent->getAttribute(QString(name)).getDoubleValue();
}

bool
IOUnit::getAttributeAsBool( const char*  name ) {
	if ( !readCurrent )
		return 0;
	return readCurrent->getAttribute(QString(name)).getBoolValue();
}

size_t
IOUnit::getChildNodeCount( const char*  name ) {
	if ( !readCurrent )
		return 0;

	if ( !name ) {
		return readCurrent->getSubsectionCount();
	}

	const QString  qName(name);
	size_t  count = 0;
	for ( Section*  s : readCurrent->Subsections ) {
		if ( s->Name == qName )
			++count;
	}
	return count;
}

bool
IOUnit::endReadSection() {
	if ( readSections.size() > 1 ) { // Root should be the only one remaining
		readSections.pop_back();
		readCurrent = readSections.back();
		return true;
	}
	return false;
}

bool
IOUnit::endRead() {
	return readSections.size() == 1; // Root should be the only one remaining
}
/*
bool
IOUnit::attributeDecl(
	const QString&  eName,
	const QString&  aName,
	const QString&  , //type
	const QString&  , //valueDefault
	const QString&  value
) {
	if ( readCurrent->Name != eName )
		return false;

	readCurrent->getAttribute(aName).setValueFromString(value);
}*/

bool
IOUnit::endDocument() {
	return true;
}

bool
IOUnit::endElement(
	const QString& , //namespaceURI
	const QString& , //localName
	const QString&   //qName
) {
	if ( builtSections.size() == 0 ) {
		return false;
	}
	builtSections.pop_back();
	builtCurrent = builtSections.back();
	return true;
}

bool
IOUnit::startElement(
	const QString&  /*namespaceURI*/,
	const QString&  /*localName*/,
	const QString&  qName,
	const QXmlAttributes&  atts
) {
	if ( ! builtCurrent )
		return false;
	Section*  section = new Section{qName};
	section->setAttributesFrom(atts);
	builtCurrent->appendSubsection(section);
	builtSections.push_back(section);
	builtCurrent = section;
	return true;
}

bool
IOUnit::startDocument() {
	builtCurrent = &readRoot;
	return true;
}

QString
IOUnit::errorString() const {
	return errorMessage;
}

bool
IOUnit::error(const QXmlParseException&  exception) {
	errorMessage = QString("[%1,%2] %3. <%1,%2>")
		.arg( exception.lineNumber() )
		.arg( exception.columnNumber() )
		.arg( exception.message() )
		.arg( exception.systemId() )
		.arg( exception.publicId() );
	return false;
}

bool
IOUnit::fatalError(const QXmlParseException&  exception) {
	errorMessage = QString("(FATAL) [%1,%2] %3. <%1,%2>")
		.arg( exception.lineNumber() )
		.arg( exception.columnNumber() )
		.arg( exception.message() )
		.arg( exception.systemId() )
		.arg( exception.publicId() );
	return false;
}

bool
IOUnit::warning(const QXmlParseException&  exception) {
	// Should pass to a callback function (where it can be stored in a logger) or saved in an array of warnings.
	errorMessage = QString("(FATAL) [%1,%2] %3. <%1,%2>")
		.arg( exception.lineNumber() )
		.arg( exception.columnNumber() )
		.arg( exception.message() )
		.arg( exception.systemId() )
		.arg( exception.publicId() );
	return false;
}

bool
IOUnit::load( QFile&  file, ding::IOSerializable&  startObject ) {
	if ( beginRead() ) {
		if ( parse(file) ) {
			startObject.deserialize(*this);
			return endRead();
		}
	}
	return false;
}

const QString
IOUnit::save( ding::IOSerializable&  startObject ) {
	if ( beginWrite() ) {
		startObject.serialize(*this);
		if ( endWrite() )
			return compose();
	}
	return QString();
}

bool
IOUnit::parse( QFile&  file ) {
	QXmlSimpleReader  reader;
	QXmlInputSource  source{&file};
	reader.setContentHandler(this);
	reader.setErrorHandler(this);
	return reader.parse(&source);
}

const QString
IOUnit::compose() {
	return output;
}

void
IOUnit::pushWriteSection( const QString&  sectionName ) {
	sectionNames.push_back(sectionName);
}

const QString
IOUnit::getCurrentWriteSection() {
	if ( sectionNames.size() > 0 )
		return sectionNames.back();

	return QString("root");
}

void
IOUnit::increasePadding() {
	padding += "\t";
}

void
IOUnit::reducePadding() {
	padding = padding.left(padding.size() - 1);
}

void
IOUnit::commitWrite( const QString&  text, bool pad ) {
	if ( pad )
		output += padding;
	output += text;
}

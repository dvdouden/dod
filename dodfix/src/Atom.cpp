#include "Atom.hpp"

Atom::Atom(unsigned int type, std::ifstream& file)
{
	this->type = type;
	this->type = getKey( "root" );
	offset = 0;
	file.seekg( 0, file.end );
	size = file.tellg();
	if ( hasChildren() ) {
		readChildren(file, 0, size);
	}
}

Atom::Atom(unsigned int type, std::ifstream& file, std::ifstream::off_type pos, std::ifstream::pos_type size)
{
	this->type = type;
	this->size = size;
	offset = pos;
	
	if ( hasChildren() ) {
		readChildren(file, pos + 8, size - 8);
	}
}

void Atom::readChildren(std::ifstream& file, std::ifstream::off_type pos, std::ifstream::pos_type size)
{
	std::ifstream::off_type end = pos + size;
	printf( "%c%c%c%c Read children from %08X - %08X\n",
		type & 0xFF, (type >> 8) & 0xFF, (type >> 16) & 0xFF, type>>24,
		(unsigned int)pos, (unsigned int)end );
	
	while ( ((unsigned int)pos) < ((unsigned int)end) )
	{
		printf( "Seek to %08X\n", pos );
		file.seekg( pos );
		if ( file.fail() ) {
			printf( "%c%c%c%c fail\n", type & 0xFF, (type >> 8) & 0xFF, (type >> 16) & 0xFF, type>>24 );
			break;
		}
		
		std::ifstream::pos_type childsize;
		unsigned int childtype;
		readAtomHeader(file, childsize, childtype);
		if ( file.fail() ) {
			printf( "%c%c%c%c fail\n", type & 0xFF, (type >> 8) & 0xFF, (type >> 16) & 0xFF, type>>24 );
			break;
		}
		
		printf( "read %c%c%c%c size %08X\n", 
		 childtype & 0xFF, (childtype >> 8) & 0xFF, (childtype >> 16) & 0xFF, childtype>>24,
		 childsize );
		if ( childtype != 0 )
		{
			// special case for mdat; want to be able to handle corrupt files
			printf( "%c%c%c%c %08X key %c%c%c%c\n",
				type & 0xFF, (type >> 8) & 0xFF, (type >> 16) & 0xFF, type>>24,
				(unsigned int)pos, childtype & 0xFF, (childtype >> 8) & 0xFF, (childtype >> 16) & 0xFF, childtype>>24 );
			
			if ( (childsize >= 8 && childsize + pos <= end) || childtype == getKey( "mdat" )  ) {
				children.insert(Atoms::value_type(childtype, new Atom(childtype, file, pos, childsize)));
			} else {
				printf( "%c%c%c%c %08X Fucked\n",
					type & 0xFF, (type >> 8) & 0xFF, (type >> 16) & 0xFF, type>>24,
					pos);
				break;
			}
			pos += childsize;
		}
		else
		{
			printf( "%c%c%c%c %08X Done\n",
				type & 0xFF, (type >> 8) & 0xFF, (type >> 16) & 0xFF, type>>24,
				pos);
			break;
		}
	}
}

Atom* Atom::getChild( unsigned int type )
{
	if ( !hasChild( type) )
	{
		printf("Couldn't find Atom! %c%c%c%c\n", type & 0xFF, (type >> 8) & 0xFF, (type >> 16) & 0xFF, type>>24 );
	}
	return (*children.find(type)).second;
}

bool Atom::hasChild( unsigned int type )
{
	return children.find(type) != children.end();
}

void Atom::readAtomHeader(std::ifstream& file, std::ifstream::pos_type& p_size, unsigned int& p_type)
{
	unsigned char buffer[8];
	file.read( reinterpret_cast<std::ifstream::char_type*>( buffer ), 8 );
	p_size = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
	p_type = (buffer[7] << 24) | (buffer[6] << 16) | (buffer[5] << 8) | buffer[4];
}

void Atom::print( unsigned int type, int tab )
{
	printf("%08X ", offset);
	for ( int i = 0; i < tab; ++i )
	{
		printf("  ");
	}
	printf("%c%c%c%c (%d)\n", type & 0xFF, (type >> 8) & 0xFF, (type >> 16) & 0xFF, type>>24,size);
	for ( Atoms::iterator it = children.begin(); it != children.end(); ++it)
	{
		(*it).second->print((*it).first, tab + 1);
	}
	
}


bool Atom::less( const Atom& rhs ) const
{
	return offset < rhs.offset;
}

unsigned int Atom::getKey( const char key[4] ) {
	unsigned int result = 0;
	for ( int i = 0; i < 4; ++i ) {
		result |= ((unsigned char)key[i]) << (i * 8);
	}
	return result;
}

bool Atom::hasChildren() const {
	return
		type == getKey( "root" ) ||
		type == getKey( "frea" ) ||
		type == getKey( "moov" ) ||
		type == getKey( "trak" ) ||
		type == getKey( "minf" ) ||
		type == getKey( "dinf" ) ||
		type == getKey( "stbl" );
}

#ifndef INC_ATOM_H
#define INC_ATOM_H

#include <vector>
#include <fstream>
#include <map>

struct Atom
{
	std::ifstream::off_type offset;
	std::ifstream::pos_type size;
	unsigned int type;
	typedef std::multimap<unsigned int, Atom*> Atoms;
	Atoms children;
	
	Atom(unsigned int type, std::ifstream& file);
	
	Atom(unsigned int type, std::ifstream& file, std::ifstream::off_type pos, std::ifstream::pos_type size);
	
	void readChildren(std::ifstream& file, std::ifstream::off_type pos, std::ifstream::pos_type size);
	
	Atom* getChild( unsigned int type );
	
	Atom* getChild( const char type[4] ) {
		return getChild( getKey( type ) );
	}
	
	bool hasChild( unsigned int type );
	
	bool hasChild( const char type[4] ) {
		return hasChild( getKey( type ) );
	}
	
	void readAtomHeader(std::ifstream& file, std::ifstream::pos_type& p_size, unsigned int& p_type);
	void print( unsigned int type, int tab = 0 );
	
	bool less( const Atom& rhs ) const;
	
	static unsigned int getKey( const char type[4] );
	
	bool hasChildren() const;
};

#endif // INC_ATOM_H

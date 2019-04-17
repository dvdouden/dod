// code provided as is, public domain, etc etc
// read: you're on your own, don't expect this to work, be happy if it does, don't come complaining if it doesn't
// if you think you can do better: be my guest
// and yes, it's a mess, I knoooowwwwwww


#include <stdio.h>
#include <string>
#include <iomanip>
#include <iostream>


#include "Atom.hpp"


typedef std::pair<unsigned int, unsigned int> chunk; // offset, size
typedef std::vector<chunk> chunks;


void copy( std::ifstream& in, std::ofstream& out, unsigned long bytes ) {
	unsigned char buffer[4096];
	unsigned long todo = bytes;
	while ( todo != 0 ) {
		unsigned long toCopy = todo > 4096 ? 4096 : todo;
		in.read( reinterpret_cast<std::ifstream::char_type*>( buffer ), toCopy );
		out.write( reinterpret_cast<std::ofstream::char_type*>( buffer ), toCopy );
		todo -= toCopy;
	}
}

void write( std::ofstream& out, const std::string& data ) {
	out.write( reinterpret_cast<const std::ofstream::char_type*>( data.c_str() ), data.size() );
}

void write( std::ofstream& out, unsigned long value ) {
	unsigned char buffer[4];
	buffer[0] = (value >> 24) & 0xFF;
	buffer[1] = (value >> 16) & 0xFF;
	buffer[2] = (value >>  8) & 0xFF;
	buffer[3] = (value >>  0) & 0xFF;
	out.write( reinterpret_cast<const std::ofstream::char_type*>( buffer ), 4 );
}

void write16( std::ofstream& out, unsigned long value ) {
	unsigned char buffer[2];
	buffer[0] = (value >>  8) & 0xFF;
	buffer[1] = (value >>  0) & 0xFF;
	out.write( reinterpret_cast<const std::ofstream::char_type*>( buffer ), 2 );
}

void write8( std::ofstream& out, unsigned long value ) {
	unsigned char buffer[1];
	buffer[0] = (value >>  0) & 0xFF;
	out.write( reinterpret_cast<const std::ofstream::char_type*>( buffer ), 1 );
}

void writeMvhd( std::ofstream& out, unsigned int samples ) {
	write( out, 108 ); // fixed size
	write( out, "mvhd" );
	write( out, 0 ); // version + flags
	write( out, 0xD8C7E62B ); // creation time
	write( out, 0xD8C7E62B ); // modification time
	write( out, 30000 ); // time scale
	write( out, samples * 1000 ); // duration
	write( out, 65536 ); // preferred rate
	write16( out, 256 ); // preferred volume
	write16( out, 0 ); // reserved
	write( out, 0 ); // reserved
	write( out, 0 ); // reserved
	
	// matrix
	// 00 01 00 00   00 00 00 00   00 00 00 00   
    // 00 00 00 00   00 01 00 00   00 00 00 00 
    // 00 00 00 00   00 00 00 00   40 00 00 00

	write( out, 65536 ); // matrix
	write( out, 0 ); // matrix
	write( out, 0 ); // matrix
	write( out, 0 ); // matrix
	write( out, 65536 ); // matrix
	write( out, 0 ); // matrix
	write( out, 0 ); // matrix
	write( out, 0 ); // matrix
	write( out, 0x40000000 ); // matrix
	write( out, 0 ); // Preview time
	write( out, 0 ); // Preview duration
	write( out, 0 ); // Poster time
	write( out, 0 ); // Selection time
	write( out, 0 ); // Selection duration
	write( out, 0 ); // Current time
	write( out, 3 ); // Next track ID
}

void writeTkhd( std::ofstream& out, unsigned long flags, unsigned long trackId, unsigned long duration, unsigned long volume, unsigned long width, unsigned long height ) {
	write( out, 92 ); // fixed size
	write( out, "tkhd" );
	write( out, flags ); // version + flags
	write( out, 0xD8C7E62B ); // Creation time
	write( out, 0xD8C7E62B ); // Modification time
	write( out, trackId ); // Track ID
	write( out, 0 ); // Reserved
	write( out, duration ); // Duration
	write( out, 0 ); // Reserved
	write( out, 0 ); // Reserved
	write16( out, 0 ); // Layer
	write16( out, 0 ); // Alternate group
	write16( out, volume ); // Volume
	write16( out, 0 ); // Reserved
	
	// matrix
	// 00 01 00 00   00 00 00 00   00 00 00 00   
    // 00 00 00 00   00 01 00 00   00 00 00 00 
    // 00 00 00 00   00 00 00 00   40 00 00 00

	write( out, 65536 ); // matrix
	write( out, 0 ); // matrix
	write( out, 0 ); // matrix
	write( out, 0 ); // matrix
	write( out, 65536 ); // matrix
	write( out, 0 ); // matrix
	write( out, 0 ); // matrix
	write( out, 0 ); // matrix
	write( out, 0x40000000 ); // matrix
	
	write( out, width * 65536 ); // Track width
	write( out, height * 65536 ); // Track height
}

void writeMdhd( std::ofstream& out, unsigned long duration, unsigned long scale ) {
	write( out, 32 );
	write( out, "mdhd" );
	write( out, 0 ); // Version + Flags
	write( out, 0xD8C7E62B ); // Creation time
	write( out, 0xD8C7E62B ); // Modification time
	write( out, scale ); // Time scale
	write( out, duration ); // Duration
	write16( out, 0 ); // Language
	write16( out, 0 ); // Quality
}

void writeHdlr( std::ofstream& out, const std::string& type, const std::string& subType, const std::string& name ) {
	write( out, 33 + name.size() );
	write( out, "hdlr" );
	write( out, 0 ); // version + flags
	write( out, type ); // Component type
	write( out, subType ); // Component subtype
	write( out, 0 ); // Component manufacturer
	write( out, 0 ); // Component flags
	write( out, 0 ); // Component flags mask
	write8( out, name.size() );
	write( out, name ); // Component name
}

void writeVMinf( std::ofstream& out, const chunks& c ) {
	unsigned int vStsdSize = 141;
	unsigned int vSttsSize = 24;
	unsigned int vStssSize = 16 + ((c.size() / 15) * 4);
	unsigned int vStscSize = 28;
	unsigned int vStszSize = 20 + (4 * c.size());
	unsigned int vStcoSize = 16 + (4 * c.size());
	
	unsigned int vStblSize = 8 + vStsdSize + vSttsSize + vStssSize + vStscSize + vStszSize + vStcoSize;
	unsigned int vMinfSize = 8 + 20 + 44 + 36 + vStblSize;
	write( out, vMinfSize );
	write( out, "minf" );
	
	// vmhd
	write( out, 20 );
	write( out, "vmhd" );
	write( out, 1 ); // version + flags
	write16( out, 0 ); // graphics mode
	write16( out, 0 ); // opcolor r
	write16( out, 0 ); // opcolor g
	write16( out, 0 ); // opcolor b
	
	// hdlr
	writeHdlr( out, "dhlr", "url ", "DataHandler" );
	
	// dinf
	write( out, 36 );
	write( out, "dinf" );
	write( out, 28 );
	write( out, "dref" );
	write( out, 0 ); // Version + flags
	write( out, 1 ); // Number of entries
	write( out, 12 ); // size
	write( out, "url " ); // type
	write( out, 1 ); // Version + flags
	
	// stbl
	write( out, vStblSize );
	write( out, "stbl" );
	
	// stsd
	write( out, vStsdSize );
	write( out, "stsd" );
	write( out, 0 ); // Version + flags
	write( out, 1 ); // number of entries
	write( out, 125 ); // size
	write( out, "avc1" ); // Data format
	write16( out, 0 ); // reserved
	write( out, 0 ); // reserved
	write16( out, 1 ); // data reference index
	write( out, 0 ); // no clue, 16 bytes 0
	write( out, 0 ); // no clue, 16 bytes 0
	write( out, 0 ); // no clue, 16 bytes 0
	write( out, 0 ); // no clue, 16 bytes 0
	write16( out, 1920 ); // width
	write16( out, 1080 ); // height
	write16( out, 72 ); // no clue
	write16( out, 0 ); // no clue
	write16( out, 72 ); // no clue
	write16( out, 0 ); // no clue
	write16( out, 0 ); // no clue
	write16( out, 0 ); // no clue
	write16( out, 1 ); // no clue
	write8( out, 4 ); // codec length
	write( out, "h264" ); // codec
	write( out, 0 ); // no clue, 16 bytes 0
	write( out, 0 ); // no clue, 16 bytes 0
	write( out, 0 ); // no clue, 16 bytes 0
	write( out, 0 ); // no clue, 16 bytes 0
	write( out, 0 ); // no clue, 11 bytes 0
	write( out, 0 ); // no clue, 11 bytes 0
	write16( out, 0 ); // no clue, 11 bytes 0
	write8( out, 0 ); // no clue, 11 bytes 0
	write16( out, 24 ); // no clue (color depth?)
	write16( out, 0xFFFF ); // no clue
	write( out, 39 ); // length
	write( out, "avcC" ); // no clue
	write16( out, 356 ); // no clue
	write16( out, 41 ); // no clue
	write16( out, 65505 ); // no clue
	write16( out, 13 ); // no clue
	write16( out, 26468 ); // no clue
	write16( out, 41 ); // no clue
	write16( out, 44059 ); // no clue
	write8( out, 72 ); // no clue
	write16( out, 1920 ); // width
	write16( out, 8830 ); // no clue
	write16( out, 22530 ); // no clue
	write16( out, 256 ); // no clue
	write16( out, 1896 ); // no clue
	write16( out, 60988 ); // no clue
	write16( out, 45056 ); // no clue
	write16( out, 0 ); // no clue
	
	// stts
	write( out, vSttsSize );
	write( out, "stts" );
	write( out, 0 ); // version + flags
	write( out, 1 ); // number of entries
	write( out, c.size() ); // sample count
	write( out, 1000 ); // sample duration
	
	// stss
	write( out, vStssSize );
	write( out, "stss" );
	write( out, 0 ); // version + flags
	write( out, c.size() / 15 ); // number of entries
	for ( unsigned int i = 0; i < c.size() / 15; ++i ) {
		write( out, i * 15 + 1 ); // key frame
	}
	
	// stsc
	write( out, vStscSize );
	write( out, "stsc" );
	write( out, 0 ); // version + flags
	write( out, 1 ); // number of entries
	write( out, 1 ); // first chunk
	write( out, 1 ); // samples per chunk
	write( out, 1 ); // samples description id
	
	// stsz
	write( out, vStszSize );
	write( out, "stsz" );
	write( out, 0 ); // version + flags
	write( out, 0 ); // sample size
	write( out, c.size() ); // number of samples
    chunks::const_iterator it;
	for (  it = c.begin(); it != c.end(); ++it ) {
		write( out, (*it).second ); // sample size
	}
	// stco
	write( out, vStcoSize );
	write( out, "stco" );
	write( out, 0 ); // version + flags
	write( out, c.size() ); // number of chunks
	for ( it = c.begin(); it != c.end(); ++it ) {
		write( out, (*it).first ); // chunk offset
	}
}

void writeVMdia( std::ofstream& out, const chunks& c ) {
	unsigned int vStsdSize = 141;
	unsigned int vSttsSize = 24;
	unsigned int vStssSize = 16 + ((c.size() / 15) * 4);
	unsigned int vStscSize = 28;
	unsigned int vStszSize = 20 + (4 * c.size());
	unsigned int vStcoSize = 16 + (4 * c.size());
	
	unsigned int vStblSize = 8 + vStsdSize + vSttsSize + vStssSize + vStscSize + vStszSize + vStcoSize;
	unsigned int vMinfSize = 8 + 20 + 44 + 36 + vStblSize;
	unsigned int vMdiaSize = 8 + 32 + 45 + vMinfSize;
	write( out, vMdiaSize );
	write( out, "mdia" );
	
	writeMdhd( out, c.size() * 1000, 30000 );
	writeHdlr( out, "mhlr", "vide", "VideoHandler" );
	writeVMinf( out, c );
}

void writeVTrak( std::ofstream& out, const chunks& c ) {
	unsigned int vStsdSize = 141;
	unsigned int vSttsSize = 24;
	unsigned int vStssSize = 16 + ((c.size() / 15) * 4);
	unsigned int vStscSize = 28;
	unsigned int vStszSize = 20 + (4 * c.size());
	unsigned int vStcoSize = 16 + (4 * c.size());
	
	unsigned int vStblSize = 8 + vStsdSize + vSttsSize + vStssSize + vStscSize + vStszSize + vStcoSize;
	unsigned int vMinfSize = 8 + 20 + 44 + 36 + vStblSize;
	unsigned int vMdiaSize = 8 + 32 + 45 + vMinfSize;
	unsigned int vTrakSize = 8 + 92 + vMdiaSize;
	write( out, vTrakSize );
	write( out, "trak" );
	
	writeTkhd( out, 15, 1, c.size() * 1000, 0, 1920, 1080 );
	writeVMdia( out, c );
}

void writeAMinf( std::ofstream& out, const chunks& c ) {
	unsigned int aStsdSize = 52;
	unsigned int aSttsSize = 24;
	unsigned int aStscSize = 16 + (4 * 3 * c.size());
	unsigned int aStszSize = 20;
	unsigned int aStcoSize = 16 + (4 * c.size());
	
	unsigned int aStblSize = 8 + aStsdSize + aSttsSize + aStscSize + aStszSize + aStcoSize;
	unsigned int aMinfSize = 8 + 16 + 44 + 36 + aStblSize;
	write( out, aMinfSize );
	write( out, "minf" );
	
	// smhd
	write( out, 16 );
	write( out, "smhd" );
	write( out, 0 ); // version + flags
	write16( out, 0 ); // balance
	write16( out, 0 ); // reserved
	
	// hdlr
	writeHdlr( out, "dhlr", "url ", "DataHandler" );
	
	// dinf
	write( out, 36 );
	write( out, "dinf" );
	write( out, 28 );
	write( out, "dref" );
	write( out, 0 ); // Version + flags
	write( out, 1 ); // Number of entries
	write( out, 12 ); // size
	write( out, "url " ); // type
	write( out, 1 ); // Version + flags
	
	// stbl
	write( out, aStblSize );
	write( out, "stbl" );
	
	// stsd
	write( out, aStsdSize );
	write( out, "stsd" );
	write( out, 0 ); // Version + flags
	write( out, 1 ); // number of entries
	write( out, 36 ); // size
	write( out, "sowt" ); // Data format
	write16( out, 0 ); // reserved
	write( out, 0 ); // reserved
	write16( out, 1 ); // data reference index
	write( out, 0 ); // no clue
	write( out, 0 ); // no clue
	write16( out, 1 ); // channels
	write16( out, 16 ); // bits
	write16( out, 0 ); // no clue
	write16( out, 0 ); // no clue
	write16( out, 32000 ); // no clue
	write16( out, 0 ); // no clue
	
	// stss
	write( out, aSttsSize );
	write( out, "stts" );
	write( out, 0 ); // version + flags
	write( out, 1 ); // number of entries
	write( out, c.size() * 32000); // sample count
	write( out, 1 ); // sample duration
	
	// stsc
	write( out, aStscSize );
	write( out, "stsc" );
	write( out, 0 ); // version + flags
	write( out, c.size() ); // number of entries
	unsigned int i = 1;
    chunks::const_iterator it;
	for ( it = c.begin(); it != c.end(); ++it ) {
		write( out, i ); // first chunk
		i++;
		write( out, 32000 ); // samples per chunk
		write( out, 1 ); // samples description id
	}
	
	// stsz
	write( out, aStszSize );
	write( out, "stsz" );
	write( out, 0 ); // version + flags
	write( out, 1 ); // sample size
	write( out, c.size() * 32000 ); // number of samples
	
	// stco
	write( out, aStcoSize );
	write( out, "stco" );
	write( out, 0 ); // version + flags
	write( out, c.size() ); // number of chunks
	for ( it = c.begin(); it != c.end(); ++it ) {
		write( out, (*it).first ); // chunk offset
	}
}


void writeAMdia( std::ofstream& out, const chunks& c ) {
	unsigned int aStsdSize = 52;
	unsigned int aSttsSize = 24;
	unsigned int aStscSize = 16 + (4 * 3 * c.size());
	unsigned int aStszSize = 20;
	unsigned int aStcoSize = 16 + (4 * c.size());
	
	unsigned int aStblSize = 8 + aStsdSize + aSttsSize + aStscSize + aStszSize + aStcoSize;
	unsigned int aMinfSize = 8 + 16 + 44 + 36 + aStblSize;
	unsigned int aMdiaSize = 8 + 32 + 45 + aMinfSize;
	write( out, aMdiaSize );
	write( out, "mdia" );
	
	writeMdhd( out, c.size() * 32000, 32000 );
	writeHdlr( out, "mhlr", "soun", "SoundHandler" );
	writeAMinf( out, c );
}

void writeATrak( std::ofstream& out, const chunks& c ) {
	unsigned int aStsdSize = 52;
	unsigned int aSttsSize = 24;
	unsigned int aStscSize = 16 + (4 * 3 * c.size());
	unsigned int aStszSize = 20;
	unsigned int aStcoSize = 16 + (4 * c.size());
	
	unsigned int aStblSize = 8 + aStsdSize + aSttsSize + aStscSize + aStszSize + aStcoSize;
	unsigned int aMinfSize = 8 + 16 + 44 + 36 + aStblSize;
	unsigned int aMdiaSize = 8 + 32 + 45 + aMinfSize;
	unsigned int aTrakSize = 8 + 92 + aMdiaSize;
	write( out, aTrakSize );
	write( out, "trak" );
	
	writeTkhd( out, 15, 2, c.size() * 30 * 1000, 256, 0, 0 );
	writeAMdia( out, c );
}


void writeUdta( std::ofstream& out ) {
	write( out, 53 );
	write( out, "udta" );
	
	write( out, 19 ); // size
	write8( out, 0xA9 ); // copyright character
	write( out, "fmt" ); // type (movie format information)
	write16( out, 7 ); // length?
	write16( out, 0 ); // no clue
	write( out, "TIO-IM" ); // some name?
	write8( out, 0 ); // termination character
	
	write( out, 26 ); // size
	write8( out, 0xA9 ); // copyright character
	write( out, "inf" ); // type (movie information)
	write16( out, 14 ); // length?
	write16( out, 0 ); // no clue
	write( out, "CarDV-TURNKEY" ); // some name?
	write8( out, 0 ); // termination character
}


bool fixMov( std::ifstream& in, std::ofstream& out )
{
	Atom root(0, in);
	
	root.print( 0,0 );
	
	bool hasMoov = root.hasChild( "moov" );
	
	if ( hasMoov ) {
		printf( "moov atom present, don't need to fix this (but will do anyway)\n" );
		//return false;
	}
	
	if ( !root.hasChild( "mdat" ) ) {
		printf( "No mdat atom present, can't fix this\n" );
		return false;
	}
	Atom* mdat = root.getChild( "mdat" );
	
	unsigned int end = root.size;
	if ( hasMoov ) {
		end = root.getChild( "moov" )->offset;
	}
	
	// figure out the correct size of mdat
	if ( mdat->offset + mdat->size != end ) {
		printf( "fixing mdat size, was %08X, should be %08X\n", (unsigned int)(mdat->size), (unsigned int)(end - mdat->offset) );
	} else {
		printf( "mdat has correct size, %08X\n", (unsigned int)(mdat->size) );
	}
	
	root.print(0,0);
		
	unsigned int off = 0x00040000;
	unsigned char buffer[8];
	typedef std::pair<unsigned int, unsigned int> chunk;
	std::vector<chunk> vchunks;
	std::vector<chunk> achunks;
	
	for ( int i = 0, j = 0; off < end; ++i ) {
		in.seekg( off );
		in.read( reinterpret_cast<std::ifstream::char_type*>( buffer ), 8 );
		unsigned int size = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
		unsigned int t = (buffer[7] << 24) | (buffer[6] << 16) | (buffer[5] << 8) | buffer[4];
		
		if ( size == 0 || off + size + 4 > end ) {
			break;
		}
		
		//printf( "%4d: [%08X] %08X\n", i, off, size );
		vchunks.push_back( chunk( off, size + 4) ); 
		off += size + 4;
		if ( (i % 30) == 29 ) {
			in.seekg( off );
			in.read( reinterpret_cast<std::ifstream::char_type*>( buffer ), 8 );
			size = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
			t = (buffer[7] << 24) | (buffer[6] << 16) | (buffer[5] << 8) | buffer[4];
			if ( t == (('f' << 0) | ('r' << 8) | ('e' << 16) | ('e' << 24)) ) {
				//printf( "    : [%08X] free %08X\n", off, size );
				off += size;
				in.seekg( off );
				// read audio
				size = 32000*2; // 16 bit 32KHz
				//printf( "%4d: [%08X] %08X\n", j, off, size );
				achunks.push_back( chunk( off, size - 4 ) );
				++j;
				off += size;
				
				do {
					in.seekg( off );
					in.read( reinterpret_cast<std::ifstream::char_type*>( buffer ), 8 );
					size = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
					t = (buffer[7] << 24) | (buffer[6] << 16) | (buffer[5] << 8) | buffer[4];
					//printf( "    : [%08X] free %08X\n", off, size );
					off += size;
				}
				while ( off < end && t == (('f' << 0) | ('r' << 8) | ('e' << 16) | ('e' << 24)) );
				off -= size;
			}
		}
	}
	printf( "Found %d video frames and %d seconds of audio, recovering...\n", vchunks.size(), achunks.size() );
	
	
	printf( "Copying data...\n" );
	// copy begin to start of mdat
	in.seekg( 0 );
	copy( in, out, mdat->offset );
	// write new mdat size and copy data
	write( out, (unsigned int)(end - mdat->offset) );
	in.seekg( mdat->offset + 4 );
	copy( in, out, (unsigned int)(end - mdat->offset) - 4 );

	printf( "Rebuilding moov atom...\n" );
	
	// calculate moov size
	unsigned int vStsdSize = 141;
	unsigned int vSttsSize = 24;
	unsigned int vStssSize = 16 + ((vchunks.size() / 15) * 4);
	unsigned int vStscSize = 28;
	unsigned int vStszSize = 20 + (4 * vchunks.size());
	unsigned int vStcoSize = 16 + (4 * vchunks.size());
	
	unsigned int vStblSize = 8 + vStsdSize + vSttsSize + vStssSize + vStscSize + vStszSize + vStcoSize;
	printf( "stbl size: %d (%08X)\n", vStblSize, vStblSize );
	unsigned int vMinfSize = 8 + 20 + 44 + 36 + vStblSize;
	printf( "minf size: %d (%08X)\n", vMinfSize, vMinfSize );
	unsigned int vMdiaSize = 8 + 32 + 45 + vMinfSize;
	printf( "mdia size: %d (%08X)\n", vMdiaSize, vMdiaSize );
	unsigned int vTrakSize = 8 + 92 + vMdiaSize;
	printf( "trak size: %d (%08X)\n", vTrakSize, vTrakSize );
	
	
	unsigned int aStsdSize = 52;
	unsigned int aSttsSize = 24;
	unsigned int aStscSize = 16 + (4 * 3 * achunks.size());
	unsigned int aStszSize = 20;
	unsigned int aStcoSize = 16 + (4 * achunks.size());
	
	unsigned int aStblSize = 8 + aStsdSize + aSttsSize + aStscSize + aStszSize + aStcoSize;
	printf( "stbl size: %d (%08X)\n", aStblSize, aStblSize );
	unsigned int aMinfSize = 8 + 16 + 44 + 36 + aStblSize;
	printf( "minf size: %d (%08X)\n", aMinfSize, aMinfSize );
	unsigned int aMdiaSize = 8 + 32 + 45 + aMinfSize;
	printf( "mdia size: %d (%08X)\n", aMdiaSize, aMdiaSize );
	unsigned int aTrakSize = 8 + 92 + aMdiaSize;
	printf( "trak size: %d (%08X)\n", aTrakSize, aTrakSize );
	
	
	unsigned int udtaSize = 53;
	unsigned int mvhdSize = 108;
	
	unsigned int moovSize = 8 + mvhdSize + vTrakSize + aTrakSize + udtaSize;
	printf( "moov size: %d (%08X)\n", moovSize, moovSize );
	
	// write moov header
	write( out, moovSize );
	write( out, "moov" );
	
	// write mvhd
	writeMvhd( out, vchunks.size() );
	
	// write video and audio traks
	writeVTrak( out, vchunks );
	writeATrak( out, achunks );
	
	// write user data
	writeUdta( out );
	
	return true;
}



void fix(const char* path)
{
	printf("%s: ", path);
	
	std::ifstream f( path, std::ifstream::binary );
	if ( !f )
	{
		printf("couldn't open!\n");
		return;
	}
	
	char outputpathfix[1024];
	sprintf(outputpathfix, "%s.fixed.mov", path);

	std::ofstream of( outputpathfix, std::ofstream::binary );
	if ( !of )
	{
		printf("couldn't open .fixed.mov\n");
		return;
	}
	if ( fixMov(f, of) ) {
		printf( "Fixed it!\n" );
	} else {
		printf("couldn't fix file!\n");
	}
	of.close();
	f.close();
	return;
}


int main(int argc, char* argv[])
{
	if ( argc < 2 )
	{
		printf("usage: %s <inputfile(s)>\n", argv[0]);
		return 1;
	}

	for ( int i = 1; i < argc; ++i )
	{
		fix( argv[i] );
	}
	
	printf( "Done, press enter to exit\n");
	std::cin.get();
	
	return 0;
}

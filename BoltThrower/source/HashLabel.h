#ifndef HashLabel_H
#define HashLabel_H

#include <string>
#include "GCTypes.h"
using namespace std;

#define HashLabel(string) HashLabel(string)

class HashLabel 
{
public:
	HashLabel()	{}
	~HashLabel() {;}
	HashLabel ( const HashLabel& Copy );

	explicit HashLabel ( const char* pString );
	explicit HashLabel ( const string& sString );
	explicit HashLabel ( u32 uHash );

	HashLabel& operator= ( const HashLabel& Copy);
	bool operator== ( const HashLabel& rCompare ) const { return  (m_uHash == rCompare.m_uHash); }
	bool operator!= ( const HashLabel& rCompare ) const { return !(m_uHash == rCompare.m_uHash); }

	// Support for STL map
	bool operator< ( const HashLabel& rCompare ) const { return m_uHash < rCompare.m_uHash; };

	u32 GetHash() const { return m_uHash; }
	void Copy( const HashLabel& Copy );
	void Clear() { m_uHash = 0; }

private:

//	HashLabel()	{}

	u32	Hash ( const char* pString );
	u32	Hash ( const char* pString, size_t uLength );

	bool operator==(const string&);
	bool operator==(const char *);
	bool operator!=(const string&);
	bool operator!=(const char *);

	u32	m_uHash;
};


#endif

#include "arghandler.h"
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>
#include <map>
#include <cstring>

CArgHandler gArg;	// global variable

CArgHandler::CArgHandler()
{
    m_argc = 0;
    m_argv = NULL;
}


CArgHandler::~CArgHandler()
{
}

void CArgHandler::Init( const int argc, char* argv[] )
{
    m_isDev = false;
    m_argc = argc;
    m_argv = argv;
    //m_debugLevel = GetCount( "v" );

    // check "devdev"
    for( int i = 1; i<argc; i++ )
    {
	if( argv[i][1] == 'd' && argv[i][2] == 'e' && argv[i][3] == 'v' &&
	    argv[i][4] == 'd' && argv[i][5] == 'e' && argv[i][6] == 'v' )
	{
	    m_isDev = true;
	}
    }
}

bool CArgHandler::CheckExist( const string caption )
{
    //map< string, string >::const_iterator ite;
    //ite = m_override.find( caption );
    //if( ite != m_override.end() )
        //return true;

    if( FindCaptionIndex( caption ) > 0 )
	return true;
    return false;
}

int CArgHandler::FindCaptionIndex( const string caption )
{
    for( int i=1; i<m_argc; i++ )
    {
        if( strcmp( m_argv[i]+1, caption.c_str() ) == 0 )
            return i;
    }
    return -1;
}


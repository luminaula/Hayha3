#include "keyboard.hpp"
#include "X11common.hpp"

#include <string.h>
#include <stdio.h>

namespace OS{

    char szKey[32];
    char szKeyOld[32] = {0};


    bool checkIfPressed(int key){

        char szBit;
        char szBitOld;
        int  iCheck;

        unsigned int keysym;

        int iKeyCode;

        XQueryKeymap( m_display, szKey );
        if( memcmp( szKey, szKeyOld, 32 ) != 0 ){
            for( int i = 0; i < sizeof( szKey ); i++ )
            {
                szBit = szKey[i];
                szBitOld = szKeyOld[i];
                iCheck = 1;

                for ( int j = 0 ; j < 8 ; j++ )
                {
                     if ( ( szBit & iCheck ) && !( szBitOld & iCheck ) )
                     {
                         iKeyCode = i * 8 + j ;

                         keysym = XKeycodeToKeysym( m_display, iKeyCode, 0 );

                         //printf( "Key: %x\n",keysym & 0xffff );
                         if(keysym == key)
                            return true;

                     }
                    iCheck = iCheck << 1 ;
                }
            }
        }
        return false;
    }
}
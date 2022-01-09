/****************************************************************************
*
*                            Open Watcom Project
*
* Copyright (c) 2002-2022 The Open Watcom Contributors. All Rights Reserved.
*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  WHEN YOU FIGURE OUT WHAT THIS FILE DOES, PLEASE
*               DESCRIBE IT HERE!
*
****************************************************************************/


#include "variety.h"
#include <dos.h>
#include <string.h>
#include <bios98.h>
#include "tinyio.h"
#include "realmod.h"
#include "ispc98.h"
#ifdef __386__
    #include "extender.h"
#endif


/*
 * FUNCTION: __nec98_bios_printer
 * DESC: handles output to the printer
 *      _PRINTER_INIT - initializes the printer
 *      _PRINTER_STATUS - gets the status of the printer
 *      _PRINTER_WRITE - sends the character at *data to the printer
 *      _PRINTER_WRITE_STRING - sends the first len characters
 *                      of the string pointed to by data to the printer
 *
 * RETURN:  Only the high byte is significant except for _PRINTER_WRITE_STRING
 *          where the whole word is significant
 *      _PRINTER_INIT - returns 0x10 if ready to recieve data or
 *                      0 otherwise
 *      _PRINTER_STATUS - as _PRINTER_INIT
 *      _PRINTER_WRITE - bit 8 is 1 if successful or 0 otherwise
 *                     - bit 9 is 1 if a time out occured or 0 otherwise
 *      _PRINTER_WRITE_STRING - the number of bytes not sent to the printer
 *                              0 for normal termination
 *                            - if no printer is connected or the printer is
 *                              off 0 is returned.
 *
 */

_WCRTLINK unsigned short __nec98_bios_printer( unsigned __cmd, unsigned char *__data )
{
    if( __isPC98 ) {    /* NEC PC-98 */
        unsigned short  ret;
#if defined( _M_I86 )
        union REGS      r;
        struct SREGS    s;

        switch( r.h.ah = __cmd ) {
        case _PRINTER_WRITE:
            r.h.al = *__data;
            /* fall through */
        case _PRINTER_INIT:
        case _PRINTER_STATUS:
            int86x( 0x1a, &r, &r, &s );
            ret = r.w.ax;
            break;
        case _PRINTER_WRITE_STRING:
            r.w.cx = strlen( (char *)__data );
            r.w.bx = FP_OFF( __data );
            s.es = FP_SEG( __data );
            int86x( 0x1a, &r, &r, &s );
            ret = r.w.cx;
            break;
        default:
            ret = 0;
            break;
        }
#else
        union REGS      r;
        call_struct     dr;
        rmi_struct      dp;

        memset( &dr, 0, sizeof( dr ) );

        switch( r.h.ah = __cmd ) {
        case _PRINTER_WRITE:
            r.h.al = *__data;
            /* fall through */
        case _PRINTER_INIT:
        case _PRINTER_STATUS:
            int386( 0x1a, &r, &r );
            ret = r.h.ah;
            break;
        case _PRINTER_WRITE_STRING:
            if( _IsRational() ) {
                unsigned long   psel;
                unsigned long   rseg;
                int             len;

                len = strlen( (char *)__data );
                r.x.ebx = ( len > 0xffff ) ? 0x1000 : ( len + 15 ) / 16;   /* paragraph */
                r.x.eax = 0x100; /* DPMI DOS Memory Alloc */
                int386( 0x31, &r, &r );
                psel = r.w.dx;
                rseg = r.w.ax;

                for( ; len > 0xffff; len -= 0xffff ) {
                    memmove( (char *)( rseg << 4 ), __data, 0xffff );
                    __data += 0xffff;
                    dr.eax = 0x3000;
                    dr.ecx = 0xffff;
                    dr.es = rseg;
                    dr.ebx = 0;
                    r.x.ecx = 0;  /* no stack for now */
                    r.x.edi = (unsigned long)&dr;
                    r.x.eax = 0x300;
                    r.x.ebx = 0x001a;
                    int386( 0x31, &r, &r);
                    if( dr.eax ) {
                        ret = dr.cx;
                        break;
                    }
                }
                memmove( (char *)( rseg << 4 ), __data, len );
                dr.eax = 0x3000;
                dr.ecx = len;
                dr.es = rseg;
                dr.ebx = 0;
                r.x.ecx = 0;  /* no stack for now */
                r.x.edi = (unsigned long)&dr;
                r.x.eax = 0x300;
                r.x.ebx = 0x001a;
                int386( 0x31, &r, &r );
                if( psel ){
                    r.x.edx = psel;
                    r.x.eax = 0x101; /* DPMI DOS Memory Free */
                    int386( 0x31, &r, &r );
                }
                ret = dr.cx;
            } else if( _IsPharLap() ) {
                unsigned long   psel;
                unsigned long   rseg;
                int             len;

                len = strlen( (char *)__data );
                r.x.ebx = ( len > 0xffff ) ? 0x1000 : ( len + 15 ) / 16;   /* paragraph */
                r.x.eax = 0x25c0; /* Alloc DOS Memory under Phar Lap */
                intdos( &r, &r );
                psel = _ExtenderRealModeSelector;
                rseg = r.w.ax;

                for( ; len > 0xffff; len -= 0xffff ) {
                    _fmemmove( MK_FP( psel, rseg << 4 ) , __data, 0xffff );
                    __data += 0xffff;
                    dp.eax = 0x3000;
                    r.x.ecx = 0xffff;
                    dp.es = rseg;
                    r.x.ebx = 0;
                    r.x.edx = (unsigned long)&dp;
                    r.x.eax = 0x2511;
                    dp.inum = 0x1a;
                    intdos( &r, &r);
                    if( dr.eax ) {
                        ret = dr.cx;
                        break;
                    }
                }
                _fmemmove( MK_FP( psel, rseg << 4 ), __data, len );
                dp.eax = 0x3000;
                r.x.ecx = len;
                dp.es = rseg;
                r.x.ebx = 0;
                r.x.edx = (unsigned long)&dr;
                r.x.eax = 0x2511;
                dp.inum = 0x1a;
                intdos( &r, &r );
                ret = r.w.cx;
                if( psel ){
                    r.x.ecx = rseg;
                    r.x.eax = 0x25c1; /* Free DOS Memory under Phar Lap*/
                    intdos( &r, &r );
                }
            } else {
                ret = 0;
            }
            break;
        default:
            ret = 0;
            break;
        }
#endif
        return( ret );
    }
    /* IBM PC */
    return( 0 );    // fail if not a NEC PC-98 machine
}

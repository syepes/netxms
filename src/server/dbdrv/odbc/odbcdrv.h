/* 
** ODBC Database Driver
** Copyright (C) 2004 Victor Kirhenshtein
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** $module: odbcdrv.h
**
**/

#ifndef _odbcdrv_h_
#define _odbcdrv_h_

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif   /* _WIN32 */

#include <stdio.h>
#include <string.h>
#include <dbdrv.h>
#include <nms_threads.h>

#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>


//
// Driver connection handle structure
//

typedef struct
{
   MUTEX mutexQuery;
   SQLHENV sqlEnv;
   SQLHDBC sqlConn;
   SQLHSTMT sqlStatement;
} ODBCDRV_CONN;


//
// Result buffer structure
//

typedef struct
{
   long iNumRows;
   long iNumCols;
   char **pValues;
} ODBCDRV_QUERY_RESULT;


//
// Async result buffer structure
//

typedef struct
{
   long iNumCols;
   ODBCDRV_CONN *pConn;
   BOOL bNoMoreRows;
} ODBCDRV_ASYNC_QUERY_RESULT;


#endif   /* _odbcdrv_h_ */

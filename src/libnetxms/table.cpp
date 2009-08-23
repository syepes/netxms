/* $Id$ */
/* 
** NetXMS - Network Management System
** Copyright (C) 2003-2009 Victor Kirhenshtein
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
** File: table.cpp
**
**/

#include "libnetxms.h"


//
// Table constructor
//

Table::Table()
{
   m_nNumRows = 0;
   m_nNumCols = 0;
   m_ppData = NULL;
   m_ppColNames = NULL;
}


//
// Create table from NXCP message
//

Table::Table(CSCPMessage *msg)
{
	int i;
	DWORD dwId;

	m_nNumRows = msg->GetVariableLong(VID_TABLE_NUM_ROWS);
	m_nNumCols = msg->GetVariableLong(VID_TABLE_NUM_COLS);

	m_ppColNames = (TCHAR **)malloc(sizeof(TCHAR *) * m_nNumCols);
	for(i = 0, dwId = VID_TABLE_COLUMN_INFO_BASE; i < m_nNumCols; i++, dwId += 9)
		m_ppColNames[i] = msg->GetVariableStr(dwId++);

	m_ppData = (TCHAR **)malloc(sizeof(TCHAR *) * m_nNumCols * m_nNumRows);
	for(i = 0, dwId = VID_TABLE_DATA_BASE; i < m_nNumCols * m_nNumRows; i++)
		m_ppData[i] = msg->GetVariableStr(dwId++);
}


//
// Table destructor
//

Table::~Table()
{
   int i;

   for(i = 0; i < m_nNumCols; i++)
      safe_free(m_ppColNames[i]);
   safe_free(m_ppColNames);

   for(i = 0; i < m_nNumRows * m_nNumCols; i++)
      safe_free(m_ppData[i]);
   safe_free(m_ppData);
}


//
// Fill NXCP message with table data
//

void Table::fillMessage(CSCPMessage &msg)
{
	int i;
	DWORD id;

	msg.SetVariable(VID_TABLE_NUM_ROWS, (DWORD)m_nNumRows);
	msg.SetVariable(VID_TABLE_NUM_COLS, (DWORD)m_nNumCols);

	for(i = 0, id = VID_TABLE_COLUMN_INFO_BASE; i < m_nNumCols; i++, id += 9)
		msg.SetVariable(id++, CHECK_NULL_EX(m_ppColNames[i]));

	for(i = 0, id = VID_TABLE_DATA_BASE; i < m_nNumCols * m_nNumRows; i++)
		msg.SetVariable(id++, CHECK_NULL_EX(m_ppData[i]));
}


//
// Add new column
//

int Table::addColumn(TCHAR *pszName)
{
   m_ppColNames = (TCHAR **)realloc(m_ppColNames, sizeof(TCHAR *) * (m_nNumCols + 1));
   m_ppColNames[m_nNumCols] = _tcsdup(pszName);

   if (m_nNumRows > 0)
   {
      TCHAR **ppNewData;
      int i, nPosOld, nPosNew;

      ppNewData = (TCHAR **)malloc(sizeof(TCHAR *) * m_nNumRows * (m_nNumCols + 1));
      for(i = 0, nPosOld = 0, nPosNew = 0; i < m_nNumRows; i++)
      {
         memcpy(&ppNewData[nPosNew], &m_ppData[nPosOld], sizeof(TCHAR *) * m_nNumCols);
         ppNewData[nPosNew + m_nNumCols] = NULL;
         nPosOld += m_nNumCols;
         nPosNew += m_nNumCols + 1;
      }
      safe_free(m_ppData);
      m_ppData = ppNewData;
   }

   m_nNumCols++;
	return m_nNumCols - 1;
}


//
// Add new row
//

int Table::addRow(void)
{
   if (m_nNumCols > 0)
   {
      m_ppData = (TCHAR **)realloc(m_ppData, sizeof(TCHAR *) * (m_nNumRows + 1) * m_nNumCols);
      memset(&m_ppData[m_nNumRows * m_nNumCols], 0, sizeof(TCHAR *) * m_nNumCols);
   }
   m_nNumRows++;
	return m_nNumRows - 1;
}


//
// Set data at position
//

void Table::setAt(int nRow, int nCol, const TCHAR *pszData)
{
   if ((nRow < 0) || (nRow >= m_nNumRows) ||
       (nCol < 0) || (nCol >= m_nNumCols))
      return;

   safe_free(m_ppData[nRow * m_nNumCols + nCol]);
   m_ppData[nRow * m_nNumCols + nCol] = _tcsdup(pszData);
}

void Table::setPreallocatedAt(int nRow, int nCol, TCHAR *pszData)
{
   if ((nRow < 0) || (nRow >= m_nNumRows) ||
       (nCol < 0) || (nCol >= m_nNumCols))
      return;

   safe_free(m_ppData[nRow * m_nNumCols + nCol]);
   m_ppData[nRow * m_nNumCols + nCol] = pszData;
}

void Table::setAt(int nRow, int nCol, LONG nData)
{
   TCHAR szBuffer[32];

   _sntprintf(szBuffer, 32, _T("%d"), nData);
   setAt(nRow, nCol, szBuffer);
}

void Table::setAt(int nRow, int nCol, DWORD dwData)
{
   TCHAR szBuffer[32];

   _sntprintf(szBuffer, 32, _T("%u"), dwData);
   setAt(nRow, nCol, szBuffer);
}

void Table::setAt(int nRow, int nCol, INT64 nData)
{
   TCHAR szBuffer[32];

   _sntprintf(szBuffer, 32, INT64_FMT, nData);
   setAt(nRow, nCol, szBuffer);
}

void Table::setAt(int nRow, int nCol, QWORD qwData)
{
   TCHAR szBuffer[32];

   _sntprintf(szBuffer, 32, UINT64_FMT, qwData);
   setAt(nRow, nCol, szBuffer);
}

void Table::setAt(int nRow, int nCol, double dData)
{
   TCHAR szBuffer[32];

   _sntprintf(szBuffer, 32, _T("%f"), dData);
   setAt(nRow, nCol, szBuffer);
}


//
// Get data from position
//

const TCHAR *Table::getAsString(int nRow, int nCol)
{
   if ((nRow < 0) || (nRow >= m_nNumRows) ||
       (nCol < 0) || (nCol >= m_nNumCols))
      return NULL;

   return m_ppData[nRow * m_nNumCols + nCol];
}

LONG Table::getAsInt(int nRow, int nCol)
{
   const TCHAR *pszVal;

   pszVal = getAsString(nRow, nCol);
   return pszVal != NULL ? _tcstol(pszVal, NULL, 0) : 0;
}

DWORD Table::getAsUInt(int nRow, int nCol)
{
   const TCHAR *pszVal;

   pszVal = getAsString(nRow, nCol);
   return pszVal != NULL ? _tcstoul(pszVal, NULL, 0) : 0;
}

INT64 Table::getAsInt64(int nRow, int nCol)
{
   const TCHAR *pszVal;

   pszVal = getAsString(nRow, nCol);
   return pszVal != NULL ? _tcstoll(pszVal, NULL, 0) : 0;
}

QWORD Table::getAsUInt64(int nRow, int nCol)
{
   const TCHAR *pszVal;

   pszVal = getAsString(nRow, nCol);
   return pszVal != NULL ? _tcstoull(pszVal, NULL, 0) : 0;
}

double Table::getAsDouble(int nRow, int nCol)
{
   const TCHAR *pszVal;

   pszVal = getAsString(nRow, nCol);
   return pszVal != NULL ? _tcstod(pszVal, NULL) : 0;
}

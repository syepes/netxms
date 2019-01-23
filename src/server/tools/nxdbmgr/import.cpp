/* 
** nxdbmgr - NetXMS database manager
** Copyright (C) 2004-2017 Victor Kirhenshtein
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
** File: import.cpp
**
**/

#include "nxdbmgr.h"
#include "sqlite3.h"

/**
 * Tables to import
 */
extern const TCHAR *g_tables[];

/**
 * Well-known integer fields to be fixed during import
 */
FIX_FIELD g_fixFields[] =
{
   { _T("dct_threshold_instances"), "tt_row_number" },
   { _T("network_maps"), "bg_zoom" },
   { _T("nodes"), "capabilities" },
   { _T("nodes"), "port_rows" },
   { _T("nodes"), "port_numbering_scheme" },
   { _T("object_properties"), "state_before_maint" },
   { _T("snmp_communities"), "zone" },
   { _T("usm_credentials"), "zone" },
   { NULL, NULL },
};

/**
 * Callback for import table
 */
static int ImportTableCB(void *arg, int cols, char **data, char **names)
{
	String query;
	int i;

	query.appendFormattedString(_T("INSERT INTO %s ("), arg);
	for(i = 0; i < cols; i++)
	{
		query.appendMBString(names[i], strlen(names[i]), CP_UTF8);
		query.append(_T(","));
	}
	query.shrink();
	query += _T(") VALUES (");
	for(i = 0; i < cols; i++)
	{
		if (data[i] != NULL)
		{
			if (*data[i] == 0)
			{
            bool fix = false;
				for(int n = 0; g_fixFields[n].table != NULL; n++)
				{
					if (!_tcsicmp(g_fixFields[n].table, static_cast<TCHAR*>(arg)) &&
                   !stricmp(g_fixFields[n].column, names[i]))
               {
                  fix = true;
                  break;
               }
				}
            query.append(fix ? _T("0,") : _T("'',"));
			}
			else
			{
#ifdef UNICODE
				WCHAR *wcData = WideStringFromUTF8String(data[i]);
				String prepData = DBPrepareString(g_dbHandle, wcData);
				free(wcData);
#else
				String prepData = DBPrepareString(g_dbHandle, data[i]);
#endif
				query.append(prepData);
				query.append(_T(","));
			}
		}
		else
		{
			query.append(_T("NULL,"));
		}
	}
	query.shrink();
	query += _T(")");

	return SQLQuery(query) ? 0 : 1;
}

/**
 * Table exist callback
 */
static int IsTableExistCB(void *arg, int cols, char **data, char **names)
{
   if (data[0] != NULL)
      return !strcmp(data[0], "0");
   return 0;
}

/**
 * Check if table exists on imported DB
 */
static bool IsTableExist(sqlite3 *db, const TCHAR* table)
{
   char query[256], *errmsg;
#ifdef UNICODE
   char *mbTable = MBStringFromWideString(table);
   snprintf(query, 256, "SELECT count(*) FROM sqlite_master WHERE type='table' AND upper(name)=upper('%s')", mbTable);
   free(mbTable);
#else
   snprintf(query, 256, _T("SELECT count(*) FROM sqlite_master WHERE type='table' AND upper(name)=upper('%s')"), table);
#endif
   int rc = sqlite3_exec(db, query, IsTableExistCB,  NULL, &errmsg);
   sqlite3_free(errmsg);

   return rc == SQLITE_OK;
}

/**
 * Import single database table
 */
static BOOL ImportTable(sqlite3 *db, const TCHAR *table)
{
   char query[256], *errmsg;
   int rc;

   _tprintf(_T("Importing table %s\n"), table);

   if (DBBegin(g_dbHandle))
   {
#ifdef UNICODE
      char *mbTable = MBStringFromWideString(table);
      snprintf(query, 256, "SELECT * FROM %s", mbTable);
      free(mbTable);
#else
      snprintf(query, 256, "SELECT * FROM %s", table);
#endif
      rc = sqlite3_exec(db, query, ImportTableCB, (void *)table, &errmsg);
      if (rc == SQLITE_OK)
      {
         DBCommit(g_dbHandle);
      }
      else
      {
         if (!IsTableExist(db, table) &&
             GetYesNo(_T("ERROR: SQL query \"%hs\" on import file failed (%hs). Continue?\n"), query, errmsg))
            rc = SQLITE_OK;
         sqlite3_free(errmsg);
         DBRollback(g_dbHandle);
      }
   }
   else
   {
      _tprintf(_T("ERROR: unable to start transaction in target database\n"));
      rc = -1;
   }
   return rc == SQLITE_OK;
}

/**
 * Import data tables
 */
static bool ImportDataTables(sqlite3 *db)
{
  if (g_dbSyntax == DB_SYNTAX_TSDB)
     return true;

   IntegerArray<UINT32> *targets = GetDataCollectionTargets();
   if (targets == NULL)
      return false;

	// Create and import idata_xx tables for each data collection target
   bool success = true;
	for(int i = 0; i < targets->size(); i++)
	{
		UINT32 id = targets->get(i);
		if (!CreateIDataTable(id))
		{
		   success = false;
			break;	// Failed to create idata_xx table
		}

		TCHAR table[64];
		_sntprintf(table, 64, _T("idata_%d"), id);
		if (!ImportTable(db, table))
		{
         success = false;
			break;
		}

      if (!CreateTDataTable(id))
      {
         success = false;
			break;	// Failed to create tdata tables
      }

		_sntprintf(table, 64, _T("tdata_%d"), id);
		if (!ImportTable(db, table))
		{
         success = false;
			break;
		}
	}

	delete targets;
	return success;
}

/**
 * Callback for getting schema version
 */
static int GetSchemaVersionCB(void *arg, int cols, char **data, char **names)
{
	*((int *)arg) = strtol(data[0], NULL, 10);
	return 0;
}

/**
 * Callback for importing module table
 */
static bool ImportModuleTable(const TCHAR *table, void *userData)
{
   return ImportTable(static_cast<sqlite3*>(userData), table);
}

/**
 * Import database
 */
void ImportDatabase(const char *file)
{
	sqlite3 *db;
	char *errmsg;
	int i;
	BOOL success = FALSE;

	// Open SQLite database
	if (sqlite3_open(file, &db) != SQLITE_OK)
	{
		_tprintf(_T("ERROR: unable to open output file\nDatabase import failed.\n"));
		return;
	}

	// Check schema version
   int legacy = 0, major = 0, minor = 0;
   if ((sqlite3_exec(db, "SELECT var_value FROM metadata WHERE var_name='SchemaVersion'", GetSchemaVersionCB, &legacy, &errmsg) != SQLITE_OK) ||
       (sqlite3_exec(db, "SELECT var_value FROM metadata WHERE var_name='SchemaVersionMajor'", GetSchemaVersionCB, &major, &errmsg) != SQLITE_OK) ||
       (sqlite3_exec(db, "SELECT var_value FROM metadata WHERE var_name='SchemaVersionMinor'", GetSchemaVersionCB, &minor, &errmsg) != SQLITE_OK))
	{
		_tprintf(_T("ERROR: SQL query failed (%hs)\n"), errmsg);
		sqlite3_free(errmsg);
		goto cleanup;
	}

	if ((legacy != DB_LEGACY_SCHEMA_VERSION) || (major != DB_SCHEMA_VERSION_MAJOR) || (minor != DB_SCHEMA_VERSION_MINOR))
	{
		_tprintf(_T("ERROR: Import file was created for database format version %d.%d, but this tool was compiled for database format version %d.%d\n"),
		         major, (legacy < DB_LEGACY_SCHEMA_VERSION) ? legacy : minor, DB_SCHEMA_VERSION_MAJOR, DB_SCHEMA_VERSION_MINOR);
		goto cleanup;
	}

	if (!ClearDatabase(false))
		goto cleanup;

	// Import tables
	for(i = 0; g_tables[i] != NULL; i++)
	{
		if (!ImportTable(db, g_tables[i]))
			goto cleanup;
	}

	if (!ImportDataTables(db))
		goto cleanup;
	if (!EnumerateModuleTables(ImportModuleTable, db))
      goto cleanup;

	success = TRUE;

cleanup:
	sqlite3_close(db);
	_tprintf(success ? _T("Database import complete.\n") : _T("Database import failed.\n"));
}

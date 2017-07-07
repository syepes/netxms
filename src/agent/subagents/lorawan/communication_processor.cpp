/*
 ** LoraWAN subagent
 ** Copyright (C) 2009 - 2017 Raden Solutions
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
 **/

#include "lorawan.h"

/**
 * Copy string depending on encoding
 */
static void CopyString(const char *value, TCHAR *buffer, size_t length)
{
#ifdef UNICODE
      MultiByteToWideChar(CP_UTF8, 0, value, -1, buffer, length);
#else
      nx_strncpy(buffer, json_string_value(value), length);
#endif
}

/**
 * Compare LoraWAN device addresses
 */
static bool CompareAddresses(BYTE *addr1, BYTE *addr2, size_t length)
{
   for(int i = 0; i < length; i++)
   {
      if (addr1[i] != addr2[i])
         return false;
   }
   return true;
}

/**
 * MQTT Mesage handler
 */
void MqttMessageHandler(const char *payload, char *topic)
{
   json_error_t error;
   json_t *root = json_loads(payload, 0, &error);

   json_t *tmp = json_object_get(root, "appargs");
   if (json_is_string(tmp))
   {
      lorawan_addr_t recDevAddr = { 0 };
      lorawan_mac_t recDevEui = { 0 };

      TCHAR guid[38];
      CopyString(json_string_value(tmp), guid, 38);
      struct deviceData *data = FindDevice(uuid::parse(guid));

      if (data != NULL)
      {
         tmp = json_object_get(root, "deveui");
         if (json_is_string(tmp))
         {
            StrToBinA(json_string_value(tmp), recDevEui, 4);
         }
         else
         {
            nxlog_debug(6, _T("LoraWAN Module:[MqttMessageHandler] No \"devEui\" field in received JSON, looking for devAddr instead..."));
            json_t *tmp = json_object_get(root, "devaddr");
            if (json_is_string(tmp))
            {
               StrToBinA(json_string_value(tmp), recDevAddr, 8);
            }
         }

         bool match;
         if (recDevAddr != NULL)
            match = CompareAddresses(recDevAddr, data->devAddr, 4);
         if (recDevEui != NULL) // Extra compare if DevEUI is available
            match = CompareAddresses(recDevEui, data->devEui, 8);

         if (match)
         {
            tmp = json_object_get(root, "data");
            if (json_is_string(tmp))
            {
               StrToBinA(json_string_value(tmp), data->payload, 36);
            }
            tmp = json_object_get(root, "fcnt");
            if (json_is_integer(tmp))
            {
               data->fcnt = json_integer_value(tmp);
            }
            tmp = json_object_get(root, "port");
            if (json_is_integer(tmp))
            {
               data->port = json_integer_value(tmp);
            }
            tmp = json_object_get(root, "rxq");
            if (json_is_object(tmp))
            {
               json_t *tmp2 = json_object_get(tmp, "datr");
               if (json_is_string(tmp2))
               {
                  CopyString(json_string_value(tmp2), data->dataRate, 24);
               }
               tmp2 = json_object_get(tmp, "freq");
               if (json_is_real(tmp2))
               {
                  data->freq = json_real_value(tmp2);
               }
               tmp2 = json_object_get(tmp, "lsnr");
               if (json_is_real(tmp2))
               {
                  data->snr = json_real_value(tmp2);
               }
               tmp2 = json_object_get(tmp, "rssi");
               if (json_is_real(tmp2))
               {
                  data->rssi = json_integer_value(tmp2);
               }

               free(tmp2);
            }
            else
               nxlog_debug(6, _T("LoraWAN Module:[MqttMessageHandler] No RX quality data received..."));

            NXMBDispatcher *dispatcher = NXMBDispatcher::getInstance();
            if (!dispatcher->call(_T("NOTIFY_DECODERS"), data, NULL))
               nxlog_debug(6, _T("LoraWAN Module:[MqttMessageHandler] Call to NXMBDispacher failed..."));

               char date[64];
               TCHAR buffer[64];
               struct tm *pCurrentTM;
               INT64 now = GetCurrentTimeMs();
               time_t t = now / 1000;
         #ifdef HAVE_LOCALTIME_R
               struct tm currentTM;
               localtime_r(&t, &currentTM);
               pCurrentTM = &currentTM;
         #else
               pCurrentTM = localtime(&t);
         #endif
               strftime(date, sizeof(date), "%x %X", pCurrentTM);
               CopyString(date, data->lastContact, 64);
         }
         else
            nxlog_debug(6, _T("LoraWAN Module:[MqttMessageHandler] Neither the devAddr nor the devEUI returned a match..."));
      }
   }
   else
      nxlog_debug(6, _T("LoraWAN Module:[MqttMessageHandler] No GUID found due to missing \"appargs\" field in received JSON"));

   free(root);
   free(tmp);
}

/**
 * Handler for communication parameters
 */
LONG H_Communication(const TCHAR *param, const TCHAR *arg, TCHAR *value, AbstractCommSession *session)
{
   TCHAR guid[38];
   if (!AgentGetParameterArg(param, 1, guid, 38))
      return SYSINFO_RC_ERROR;

   struct deviceData *data = FindDevice(uuid::parse(guid));
   if (data == NULL)
      return SYSINFO_RC_ERROR;

   switch(*arg)
   {
      case 'C':
         ret_string(value, data->lastContact);
         break;
      case 'D':
         ret_string(value, data->dataRate);
         break;
      case 'F':
         ret_uint(value, data->freq);
         break;
      case 'M':
         ret_uint(value, data->fcnt);
         break;
      case 'R':
         ret_int(value, data->rssi);
         break;
      case 'S':
         ret_int(value, data->snr);
         break;
      default:
         return SYSINFO_RC_UNSUPPORTED;
   }

   return SYSINFO_RC_SUCCESS;
}

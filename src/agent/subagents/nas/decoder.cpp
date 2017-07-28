/*
 ** NAS decoder for LoraWAN subagent
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

#include "nas.h"

/**
 * Calculates the battery voltage from the given offset
 */
static double CalculateBatteryVoltage(int offset)
{
   return 3.6 - max(0, (254 - offset - max(0, 16 - offset) - max(0, 245 - offset - max(0, 16 - offset)))) * 0.05 - max(0, 245 - offset - max(0, 16 - offset)) * 0.004 - max(0, 16 - offset) * 0.05;
}

/**
 * Parses the received payload
 */
static void ParsePayload(SensorData *sData, const lorawan_payload_t *payload, UINT32 fieldId)
{
   switch(fieldId)
   {
      case SENSOR_COUNTER32:
      {
         UINT32 valueInt32;
         memcpy(&valueInt32, payload, 4);
         nxlog_debug(4, _T("LoraWAN Module: Received val: %d"), valueInt32);
#if WORDS_BIGENDIAN
         valueInt32 = bswap_32(valueInt32);
#endif
         UINT32 msw = (UINT32)(sData->counter >> 32);
         sData->counter = (((UINT64)msw << 32) | valueInt32);
         nxlog_debug(4, _T("LoraWAN Module: counter val: %d"), sData->counter);
      }
         break;
      case SENSOR_COUNTER64:
         memcpy(&sData->counter, payload, 8);
#if WORDS_BIGENDIAN
         valueInt64 = bswap_64(sData->counter);
#endif
         break;
      case SENSOR_BATTERY:
      {
         UINT32 offset;
         memcpy(&offset, payload[5], 1);
         sData->batt = CalculateBatteryVoltage(offset);
      }
         break;
      case SENSOR_TEMP:
         memcpy(&sData->temp, payload[6], 1);
         break;
      case SENSOR_RSSI:
         memcpy(&sData->rssi, payload[7], 1);
         break;
      case SENSOR_WATCH_MODE:
         sData->watchMode =((*payload[8] & (BYTE)0x01) == 0x01) ? 1 : 0;
         break;
      case SENSOR_WATCH_STATUS:
         sData->watchStatus =((*payload[9] & (BYTE)0x01) == 0x01) ? 1 : 0;
         break;
      case SENSOR_LEAK_MODE:
         sData->leakMode =((*payload[8] & (BYTE)0x02) == 0x02) ? 1 : 0;
         break;
      case SENSOR_LEAK_STATUS:
         sData->leakStatus =((*payload[9] & (BYTE)0x02) == 0x02) ? 1 : 0;
         break;
      case SENSOR_REV_FLOW_MODE:
         sData->revFlowMode =((*payload[8] & (BYTE)0x04) == 0x04) ? 1 : 0;
         break;
      case SENSOR_REV_FLOW_STATUS:
         sData->revFlowStatus =((*payload[9] & (BYTE)0x04) == 0x04) ? 1 : 0;
         break;
      case SENSOR_TAMPER_MODE:
         sData->tamperMode =((*payload[8] & (BYTE)0x08) == 0x08) ? 1 : 0;
         break;
      case SENSOR_TAMPER_STATUS:
         sData->tamperStatus =((*payload[9] & (BYTE)0x08) == 0x08) ? 1 : 0;
         break;
      case SENSOR_MODE:
         sData->sensorMode =((*payload[8] & (BYTE)0x10) == 0x10) ? 1 : 0;
         break;
      case SENSOR_REPORTING_MODE:
         sData->reportingMode =((*payload[8] & (BYTE)0x20) == 0x20) ? 1 : 0;
         break;
      case SENSOR_TEMP_DETECT_MODE:
         sData->tempDetectMode =((*payload[8] & (BYTE)0x40) == 0x40) ? 1 : 0;
         break;
      case SENSOR_TEMP_DETECT_STATUS:
         sData->tempDetectStatus =((*payload[9] & (BYTE)0x40) == 0x40) ? 1 : 0;
         break;
   }
}

/**
 * Decodes LoraWAN device payload
 */
bool Decode(const TCHAR *name, const void *data, void *result)
{
   LoraDeviceData *dData = (LoraDeviceData *)data;
   if (dData == NULL  || dData->getDecoder() != NAS)
      return false;

   SensorData *sData = FindSensor(dData->getGuid());
   if (sData == NULL) // Create new entry
   {
      sData = new struct SensorData();
      sData->guid = dData->getGuid();
   }

   switch(dData->getPort())
   {
      case 24:
         ParsePayload(sData, dData->getPayload(), SENSOR_COUNTER32);
         ParsePayload(sData, dData->getPayload(), SENSOR_BATTERY);
         ParsePayload(sData, dData->getPayload(), SENSOR_TEMP);
         ParsePayload(sData, dData->getPayload(), SENSOR_RSSI);
         ParsePayload(sData, dData->getPayload(), SENSOR_WATCH_MODE);
         ParsePayload(sData, dData->getPayload(), SENSOR_WATCH_STATUS);
         ParsePayload(sData, dData->getPayload(), SENSOR_LEAK_MODE);
         ParsePayload(sData, dData->getPayload(), SENSOR_LEAK_STATUS);
         ParsePayload(sData, dData->getPayload(), SENSOR_REV_FLOW_MODE);
         ParsePayload(sData, dData->getPayload(), SENSOR_REV_FLOW_STATUS);
         ParsePayload(sData, dData->getPayload(), SENSOR_TAMPER_MODE);
         ParsePayload(sData, dData->getPayload(), SENSOR_TAMPER_STATUS);
         ParsePayload(sData, dData->getPayload(), SENSOR_MODE);
         ParsePayload(sData, dData->getPayload(), SENSOR_REPORTING_MODE);
         ParsePayload(sData, dData->getPayload(), SENSOR_TEMP_DETECT_MODE);
         ParsePayload(sData, dData->getPayload(), SENSOR_TEMP_DETECT_STATUS);
         break;
      case 6:
         if (sData->reportingMode)
         {
            sData->counter = 0;
            sData->reportingMode = 0;
         }
         ParsePayload(sData, dData->getPayload(), SENSOR_COUNTER64);
         break;
      case 14:
         if (!sData->reportingMode)
         {
            sData->counter = 0;
            sData->reportingMode = 1;
         }
         ParsePayload(sData, dData->getPayload(), SENSOR_COUNTER64);
         break;
      default:
         return false;
   }
   AddSensor(sData);

   return true;
}

/**
 * Handler for sensor data
 */
LONG H_Sensor(const TCHAR *param, const TCHAR *arg, TCHAR *value, AbstractCommSession *session)
{
   TCHAR guid[38];
   if (!AgentGetParameterArg(param, 1, guid, 38))
      return SYSINFO_RC_ERROR;

   SensorData *data = FindSensor(uuid::parse(guid));
   if (data == NULL)
      return SYSINFO_RC_ERROR;

   switch(*arg)
   {
      case 'B':
         ret_double(value, data->batt);
         break;
      case 'R':
         ret_int(value, data->rssi);
         break;
      case 'U':
         ret_uint64(value, data->counter);
         break;
      default:
         return SYSINFO_RC_UNSUPPORTED;
   }

   return SYSINFO_RC_SUCCESS;
}

/**
 * Handler for sensor modes
 */
LONG H_Mode(const TCHAR *param, const TCHAR *arg, TCHAR *value, AbstractCommSession *session)
{
   TCHAR guid[38];
   if (!AgentGetParameterArg(param, 1, guid, 38))
      return SYSINFO_RC_ERROR;

   SensorData *data = FindSensor(uuid::parse(guid));
   if (data == NULL)
      return SYSINFO_RC_ERROR;

   switch(*arg)
   {
      case 'A':
         ret_uint(value, data->tamperMode);
         break;
      case 'F':
         ret_uint(value, data->revFlowMode);
         break;
      case 'L':
         ret_uint(value, data->leakMode);
         break;
      case 'R':
         ret_uint(value, data->reportingMode);
         break;
      case 'S':
         ret_uint(value, data->sensorMode);
         break;
      case 'T':
         ret_uint(value, data->temp);
         break;
      case 'W':
         ret_uint(value, data->watchMode);
         break;
      default:
         return SYSINFO_RC_UNSUPPORTED;
   }

   return SYSINFO_RC_SUCCESS;
}

/**
 * Handler for sensor status
 */
LONG H_Status(const TCHAR *param, const TCHAR *arg, TCHAR *value, AbstractCommSession *session)
{
   TCHAR guid[38];
   if (!AgentGetParameterArg(param, 1, guid, 38))
      return SYSINFO_RC_ERROR;

   SensorData *data = FindSensor(uuid::parse(guid));
   if (data == NULL)
      return SYSINFO_RC_ERROR;

   switch(*arg)
   {
      case 'A':
         ret_uint(value, data->tamperStatus);
         break;
      case 'D':
         ret_uint(value, data->tempDetectStatus);
         break;
      case 'L':
         ret_uint(value, data->leakStatus);
         break;
      case 'R':
         ret_uint(value, data->revFlowStatus);
         break;
      case 'T':
         ret_uint(value, data->tempDetectStatus);
         break;
      case 'W':
         ret_uint(value, data->watchStatus);
         break;
      default:
         return SYSINFO_RC_UNSUPPORTED;
   }

   return SYSINFO_RC_SUCCESS;
}

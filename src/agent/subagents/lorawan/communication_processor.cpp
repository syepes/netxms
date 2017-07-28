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
      nx_strncpy(buffer, value, length);
#endif
}

/**
 * Create new Lora Device Data object from NXCPMessage
 */
LoraDeviceData::LoraDeviceData(NXCPMessage *request)
{
   m_guid = request->getFieldAsGUID(VID_GUID);

   m_devAddr = NULL;
   m_devEui = NULL;

   if (request->getFieldAsUInt32(VID_REG_TYPE) == 0) // OTAA
   {
      BYTE devEui[8];
      request->getFieldAsBinary(VID_MAC_ADDR, devEui, 8);
      m_devEui = new MacAddress(devEui, 8);
   }
   else
   {
      char devAddr[12];
      request->getFieldAsMBString(VID_DEVICE_ADDRESS, devAddr, 12);
      m_devAddr = MacAddress::parse(devAddr);
   }

   memset(m_payload, 0, 36);
   m_decoder = request->getFieldAsInt32(VID_DECODER);
   m_dataRate[0] = 0;
   m_rssi = 0;
   m_snr = -100;
   m_freq = 0;
   m_fcnt = 0;
   m_port = 0;
   m_lastContact[0] = 0;
}

/**
 * Create Lora Device Data object from DB record
 */
LoraDeviceData::LoraDeviceData(DB_RESULT result, int row)
{
   m_guid = DBGetFieldGUID(result, row, 0);
   m_devAddr = NULL;
   m_devEui = NULL;

   BYTE buffer[8];
   if (DBGetFieldByteArray2(result, row, 1, buffer, 4, 0))
   {
      m_devAddr = new MacAddress(buffer, 4);
   }
   if (DBGetFieldByteArray2(result, row, 2, buffer, 8, 0))
   {
      m_devEui = new MacAddress(buffer, 8);
   }
   m_decoder = DBGetFieldULong(result, row, 3);

   memset(m_payload, 0, 36);
   m_dataRate[0] = 0;
   m_rssi = 0;
   m_snr = -100;
   m_freq = 0;
   m_fcnt = 0;
   m_port = 0;
   m_lastContact[0] = 0;
}

/**
 * Destructor for Lora Device Data object
 */
LoraDeviceData::~LoraDeviceData()
{
   delete(m_devAddr);
   delete(m_devEui);
}

/**
 * Set device data rate
 */
void LoraDeviceData::setDataRate(const char *dataRate)
{
   CopyString(dataRate, m_dataRate, 24);
}

/**
 * Update device last contact
 */
void LoraDeviceData::setLastContact()
{
   char date[64];
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
   CopyString(date, m_lastContact, 64);
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
      MacAddress *rxDevAddr = NULL;
      MacAddress *rxDevEui = NULL;

      TCHAR guid[38];
      CopyString(json_string_value(tmp), guid, 38);
      LoraDeviceData *data = FindDevice(uuid::parse(guid));

      if (data != NULL)
      {
         tmp = json_object_get(root, "deveui");
         if (json_is_string(tmp))
            rxDevEui = MacAddress::parse(json_string_value(tmp));
         else
         {
            nxlog_debug(6, _T("LoraWAN Module:[MqttMessageHandler] No \"devEui\" field in received JSON, looking for devAddr instead..."));
            json_t *tmp = json_object_get(root, "devaddr");
            if (json_is_string(tmp))
               rxDevAddr = MacAddress::parse(json_string_value(tmp));
         }

         if ((data->getDevAddr() != NULL && data->getDevAddr()->equals(*rxDevAddr))
              || (data->getDevEui() != NULL && data->getDevEui()->equals(*rxDevEui)))
         {
            delete(rxDevAddr);
            delete(rxDevEui);

            tmp = json_object_get(root, "data");
            if (json_is_string(tmp))
               data->setPayload(json_string_value(tmp));

            tmp = json_object_get(root, "fcnt");
            if (json_is_integer(tmp))
               data->setFcnt(json_integer_value(tmp));

            tmp = json_object_get(root, "port");
            if (json_is_integer(tmp))
               data->setPort(json_integer_value(tmp));

            tmp = json_object_get(root, "rxq");
            if (json_is_object(tmp))
            {
               json_t *tmp2 = json_object_get(tmp, "datr");
               if (json_is_string(tmp2))
                  data->setDataRate(json_string_value(tmp2));

               tmp2 = json_object_get(tmp, "freq");
               if (json_is_real(tmp2))
                  data->setFreq(json_real_value(tmp2));

               tmp2 = json_object_get(tmp, "lsnr");
               if (json_is_real(tmp2))
                  data->setSnr(json_real_value(tmp2));

               tmp2 = json_object_get(tmp, "rssi");
               if (json_is_real(tmp2))
                  data->setRssi(json_integer_value(tmp2));

               free(tmp2);
            }
            else
               nxlog_debug(6, _T("LoraWAN Module:[MqttMessageHandler] No RX quality data received..."));

            NXMBDispatcher *dispatcher = NXMBDispatcher::getInstance();
            if (!dispatcher->call(_T("NOTIFY_DECODERS"), data, NULL))
               nxlog_debug(6, _T("LoraWAN Module:[MqttMessageHandler] Call to NXMBDispacher failed..."));

            data->setLastContact();
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

   LoraDeviceData *data = FindDevice(uuid::parse(guid));
   if (data == NULL)
      return SYSINFO_RC_ERROR;

   switch(*arg)
   {
      case 'C':
         ret_string(value, data->getLastContact());
         break;
      case 'D':
         ret_string(value, data->getDataRate());
         break;
      case 'F':
         ret_uint(value, data->getFreq());
         break;
      case 'M':
         ret_uint(value, data->getFcnt());
         break;
      case 'R':
         ret_int(value, data->getRssi());
         break;
      case 'S':
         ret_int(value, data->getSnr());
         break;
      default:
         return SYSINFO_RC_UNSUPPORTED;
   }

   return SYSINFO_RC_SUCCESS;
}

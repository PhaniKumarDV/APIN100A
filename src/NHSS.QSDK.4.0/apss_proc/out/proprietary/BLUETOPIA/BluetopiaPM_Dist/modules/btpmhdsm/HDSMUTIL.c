/*****< hdsmutil.h >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HDSMUTIL - Headset Manager Utility functions for Stonestreet One          */
/*             Bluetooth Protocol Stack Platform Manager.                     */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   12/16/14  M. Seabold     Initial creation                                */
/******************************************************************************/
#include "HDSMUTIL.h"            /* HDS Utility Prototypes/Constants.         */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */

   /* Internal function prototypes.                                     */
static Boolean_t ServiceRecordContainsServiceClass(SDP_Service_Attribute_Response_Data_t *ServiceRecord, UUID_128_t ServiceClass);
static SDP_Data_Element_t *FindSDPAttribute(SDP_Service_Attribute_Response_Data_t *ServiceRecord, Word_t AttributeID);
static int ConvertSDPDataElementToUUID128(SDP_Data_Element_t DataElement, UUID_128_t *UUID);

   /* The following function is a utility function that exists to       */
   /* determine whether a service record contains a given Service Class */
   /* UUID. This function returns TRUE if the given Service Class UUID  */
   /* was found in the Service Class List attribute of the service      */
   /* record.                                                           */
static Boolean_t ServiceRecordContainsServiceClass(SDP_Service_Attribute_Response_Data_t *ServiceRecord, UUID_128_t ServiceClass)
{
   Boolean_t           ret_val;
   UUID_128_t          RecordServiceClass;
   unsigned int        ServiceClassIndex;
   SDP_Data_Element_t *ServiceClassAttribute;

   ret_val = FALSE;

   if(ServiceRecord)
   {
      if((ServiceClassAttribute = FindSDPAttribute(ServiceRecord, SDP_ATTRIBUTE_ID_SERVICE_CLASS_ID_LIST)) != NULL)
      {
         /* The Service Class Attribute has been located. Make sure that*/
         /* it contains a Sequence, as required.                        */
         if(ServiceClassAttribute->SDP_Data_Element_Type == deSequence)
         {
            /* The attribute looks valid. Now search for the appropriate*/
            /* Service Class UUID.                                      */
            for(ServiceClassIndex = 0; ServiceClassIndex < ServiceClassAttribute->SDP_Data_Element_Length; ServiceClassIndex++)
            {
               /* Normalize the Service Class to a 128-bit UUID for     */
               /* comparison.                                           */
               if(ConvertSDPDataElementToUUID128(ServiceClassAttribute->SDP_Data_Element.SDP_Data_Element_Sequence[ServiceClassIndex], &RecordServiceClass) == 0)
               {
                  /* The Service Class UUID was successfully located, so*/
                  /* see if it is the Service Class we are looking for. */
                  if(COMPARE_UUID_128(RecordServiceClass, ServiceClass))
                  {
                     /* The Service Class UUID has been found in the    */
                     /* current service record.                         */
                     ret_val = TRUE;
                     break;
                  }
               }
            }
         }
      }
   }

   return(ret_val);
}

   /* The following function is a utility function that exists to locate*/
   /* a particular Attribute within a parsed SDP record. This function  */
   /* returns a pointer to the Attribute data, or NULL if the attribute */
   /* does not exist in the given SDP record.                           */
static SDP_Data_Element_t *FindSDPAttribute(SDP_Service_Attribute_Response_Data_t *ServiceRecord, Word_t AttributeID)
{
   unsigned int        AttributeIndex;
   SDP_Data_Element_t *AttributePtr = NULL;

   /* First, make sure that the input parameters appear to be           */
   /* semi-valid.                                                       */
   if(ServiceRecord)
   {
      /* Loop through all available attributes in the record to find the*/
      /* requested attribute.                                           */
      for(AttributeIndex=0;AttributeIndex<(unsigned int)ServiceRecord->Number_Attribute_Values;AttributeIndex++)
      {
         /* Check whether we have found the requested attribute.        */
         if(ServiceRecord->SDP_Service_Attribute_Value_Data[AttributeIndex].Attribute_ID == AttributeID)
         {
            /* The attribute has been found. Save the attribute and stop*/
            /* searching the record.                                    */
            AttributePtr = ServiceRecord->SDP_Service_Attribute_Value_Data[AttributeIndex].SDP_Data_Element;
            break;
         }
      }
   }

   return(AttributePtr);
}

   /* The following function is a utility function that exists to       */
   /* convert an SDP Data Element, which contains a UUID, into a 128-bit*/
   /* UUID. This function returns zero if successful, or a negative     */
   /* error code on failure.                                            */
static int ConvertSDPDataElementToUUID128(SDP_Data_Element_t DataElement, UUID_128_t *UUID)
{
   int ret_val;

   /* First, make sure that the input parameters appear to be           */
   /* semi-valid.                                                       */
   if(UUID)
   {
      switch(DataElement.SDP_Data_Element_Type)
      {
         case deUUID_128:
            *UUID   = DataElement.SDP_Data_Element.UUID_128;
            ret_val = 0;
            break;
         case deUUID_32:
            SDP_ASSIGN_BASE_UUID(*UUID);
            ASSIGN_SDP_UUID_32_TO_SDP_UUID_128(*UUID, DataElement.SDP_Data_Element.UUID_32);
            ret_val = 0;
            break;
         case deUUID_16:
            SDP_ASSIGN_BASE_UUID(*UUID);
            ASSIGN_SDP_UUID_16_TO_SDP_UUID_128(*UUID, DataElement.SDP_Data_Element.UUID_16);
            ret_val = 0;
            break;
         default:
            /* No other data type is allowed for this parameter, so     */
            /* return an error.                                         */
            ASSIGN_SDP_UUID_128(*UUID, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            break;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* query the HDSET SDP information from a remote device. The         */
   /* BluetoothAddress parameter is the BD_ADDR of the remote           */
   /* device. The ServiceInformation parameter is a pointer to a        */
   /* structure for storing the service information. This function      */
   /* returns zero if successful and a negative return error code if    */
   /* there was an error.                                               */
int BTPSAPI HDSM_Query_Remote_Device_Service_Information(BD_ADDR_t RemoteDeviceAddress, HDSM_Service_Information_t *ServiceInformation)
{
   int                            ret_val;
   UUID_128_t                     HDSHSServiceClass;
   UUID_128_t                     HDSAGServiceClass;
   UUID_128_t                     RFCOMMProtocolUUID;
   UUID_128_t                     RecordProtocolUUID;
   unsigned int                   ProtocolIndex;
   unsigned int                   ServiceRecordIndex;
   unsigned int                   RawServiceDataLength;
   unsigned int                   PortNumber;
   unsigned char                 *RawServiceDataBuffer;
   SDP_Data_Element_t            *Attribute;
   DEVM_Parsed_SDP_Data_t         ParsedSDPData;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   SDP_ASSIGN_HEADSET_PROFILE_UUID_128(HDSHSServiceClass);
   SDP_ASSIGN_HEADSET_AUDIO_GATEWAY_PROFILE_UUID_128(HDSAGServiceClass);
   SDP_ASSIGN_RFCOMM_UUID_128(RFCOMMProtocolUUID);

   /* Make sure the parameters seem semi-valid.                         */
   if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (ServiceInformation))
   {
      BTPS_MemInitialize(ServiceInformation, 0, sizeof(HDSM_Service_Information_t));

      PortNumber                = 0;

      /* Obtain the DEVM lock to control the services.                  */
      if(DEVM_AcquireLock())
      {
         if((ret_val = DEVM_QueryRemoteDeviceServices(RemoteDeviceAddress, FALSE, 0, NULL, &RawServiceDataLength)) >= 0)
         {
            if(RawServiceDataLength)
            {
               if((RawServiceDataBuffer = (unsigned char *)BTPS_AllocateMemory(RawServiceDataLength)) != NULL)
               {
                  if((ret_val = DEVM_QueryRemoteDeviceServices(RemoteDeviceAddress, FALSE, RawServiceDataLength, RawServiceDataBuffer, NULL)) == (int)RawServiceDataLength)
                  {
                     /* Release the lock because we are finished with   */
                     /* it.                                             */
                     DEVM_ReleaseLock();

                     RawServiceDataLength = (unsigned int)ret_val;
                     ret_val              = 0;

                     if((ret_val = DEVM_ConvertRawSDPStreamToParsedSDPData(RawServiceDataLength, RawServiceDataBuffer, &ParsedSDPData)) == 0)
                     {
                        /* Determine the number of Message Access       */
                        /* records and the amount of extra storage      */
                        /* required for the service details.            */
                        for(ServiceRecordIndex = 0; ServiceRecordIndex < ParsedSDPData.NumberServiceRecords; ServiceRecordIndex++)
                        {
                           if((ServiceRecordContainsServiceClass(&ParsedSDPData.SDPServiceAttributeResponseData[ServiceRecordIndex], HDSHSServiceClass)) || (ServiceRecordContainsServiceClass(&ParsedSDPData.SDPServiceAttributeResponseData[ServiceRecordIndex], HDSAGServiceClass)))
                           {
                              /* ServerPort                             */
                              if((Attribute = FindSDPAttribute(&ParsedSDPData.SDPServiceAttributeResponseData[ServiceRecordIndex], SDP_ATTRIBUTE_ID_PROTOCOL_DESCRIPTOR_LIST)) != NULL)
                              {
                                 //XXX Replace with a generic "Get RFCOMM Port" routine once implemented in SPPM
                                 if(Attribute->SDP_Data_Element_Type == deSequence)
                                 {
                                    for(ProtocolIndex = 0; ProtocolIndex < Attribute->SDP_Data_Element_Length; ProtocolIndex++)
                                    {
                                       if((Attribute->SDP_Data_Element.SDP_Data_Element_Sequence[ProtocolIndex].SDP_Data_Element_Type == deSequence) && (Attribute->SDP_Data_Element.SDP_Data_Element_Sequence[ProtocolIndex].SDP_Data_Element_Length >= 2))
                                       {
                                          /* Check if we have the RFCOMM*/
                                          /* protocol descriptor.       */
                                          if((Attribute->SDP_Data_Element.SDP_Data_Element_Sequence[ProtocolIndex].SDP_Data_Element.SDP_Data_Element_Sequence[0].SDP_Data_Element_Type == deUUID_16) || (Attribute->SDP_Data_Element.SDP_Data_Element_Sequence[ProtocolIndex].SDP_Data_Element.SDP_Data_Element_Sequence[0].SDP_Data_Element_Type == deUUID_32) || (Attribute->SDP_Data_Element.SDP_Data_Element_Sequence[ProtocolIndex].SDP_Data_Element.SDP_Data_Element_Sequence[0].SDP_Data_Element_Type == deUUID_128))
                                          {
                                             switch(Attribute->SDP_Data_Element.SDP_Data_Element_Sequence[ProtocolIndex].SDP_Data_Element.SDP_Data_Element_Sequence[0].SDP_Data_Element_Type)
                                             {
                                                case deUUID_16:
                                                   SDP_ASSIGN_BASE_UUID(RecordProtocolUUID);
                                                   ASSIGN_SDP_UUID_16_TO_SDP_UUID_128(RecordProtocolUUID, Attribute->SDP_Data_Element.SDP_Data_Element_Sequence[ProtocolIndex].SDP_Data_Element.SDP_Data_Element_Sequence[0].SDP_Data_Element.UUID_16);
                                                   break;
                                                case deUUID_32:
                                                   SDP_ASSIGN_BASE_UUID(RecordProtocolUUID);
                                                   ASSIGN_SDP_UUID_32_TO_SDP_UUID_128(RecordProtocolUUID, Attribute->SDP_Data_Element.SDP_Data_Element_Sequence[ProtocolIndex].SDP_Data_Element.SDP_Data_Element_Sequence[0].SDP_Data_Element.UUID_32);
                                                   break;
                                                case deUUID_128:
                                                   RecordProtocolUUID = Attribute->SDP_Data_Element.SDP_Data_Element_Sequence[ProtocolIndex].SDP_Data_Element.SDP_Data_Element_Sequence[0].SDP_Data_Element.UUID_128;
                                                   break;
                                                default:
                                                   BTPS_MemInitialize(&RecordProtocolUUID, 0, sizeof(UUID_128_t));
                                             }

                                             if(COMPARE_UUID_128(RecordProtocolUUID, RFCOMMProtocolUUID))
                                             {
                                                if(Attribute->SDP_Data_Element.SDP_Data_Element_Sequence[ProtocolIndex].SDP_Data_Element.SDP_Data_Element_Sequence[1].SDP_Data_Element_Type == deUnsignedInteger1Byte)
                                                   PortNumber = Attribute->SDP_Data_Element.SDP_Data_Element_Sequence[ProtocolIndex].SDP_Data_Element.SDP_Data_Element_Sequence[1].SDP_Data_Element.UnsignedInteger1Byte;
                                             }
                                          }
                                       }
                                    }
                                 }

                                 if(ServiceRecordContainsServiceClass(&ParsedSDPData.SDPServiceAttributeResponseData[ServiceRecordIndex], HDSHSServiceClass))
                                 {
                                    ServiceInformation->Flags                        |= HDSM_SERVICE_INFORMATION_FLAGS_HEADSET_DATA_VALID;
                                    ServiceInformation->HeadsetInformation.PortNumber = PortNumber;

                                    if((Attribute = FindSDPAttribute(&ParsedSDPData.SDPServiceAttributeResponseData[ServiceRecordIndex], SDP_ATTRIBUTE_ID_REMOTE_AUDIO_VOLUME_CONTROL)) != NULL)
                                    {
                                       if((Attribute->SDP_Data_Element_Type == deBoolean) && (Attribute->SDP_Data_Element.Boolean))
                                          ServiceInformation->HeadsetInformation.Flags |= HDSM_SUPPORTED_FEATURES_MASK_HEADSET_SUPPORTS_REMOTE_AUDIO_VOLUME_CONTROLS;
                                    }
                                 }
                                 else
                                 {
                                    ServiceInformation->Flags                             |= HDSM_SERVICE_INFORMATION_FLAGS_AUDIO_GATEWAY_DATA_VALID;
                                    ServiceInformation->AudioGatewayInformation.PortNumber = PortNumber;
                                 }
                              }
                           }
                        }
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_SERVICE_RECORD_ATTRIBUTE_DATA_INVALID;
                  }
                  else
                     DEVM_ReleaseLock();

                  BTPS_FreeMemory(RawServiceDataBuffer);
               }
               else
               {
                  DEVM_ReleaseLock();
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
               }
            }
            else
            {
               DEVM_ReleaseLock();
               ret_val = BTPM_ERROR_CODE_DEVICE_SERVICES_NOT_KNOWN;
            }
         }
         else
         {
            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));

   return(ret_val);
}


/*****< linuxgatm_srv.h >******************************************************/
/*      Copyright 2014 Stonestreet One.                                       */
/*      All Rights ReSRVved.                                                  */
/*                                                                            */
/*  LINUXGATM_SRV - Simple Linux application using Bluetopia Platform Manager */
/*                  Generic Attribute Profile (GATT) Manager Application      */
/*                  Programming Interface (API) - Server Role Only.           */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/21/14  T. Cook        Initial creation. (Based on LinuxGATM_CLT)      */
/******************************************************************************/
#ifndef __LINUXGATM_SRVH__
#define __LINUXGATM_SRVH__

#include "SS1BTPM.h"          /* BTPM Application Programming Interface.      */

   /*********************************************************************/
   /* Service Table Structures.                                         */
   /*********************************************************************/

   /* The following type definition represents the container structure  */
   /* for a Characteristic Attribute.                                   */
typedef struct _tagCharacteristicInfo_t
{
   unsigned long  CharacteristicPropertiesMask;
   unsigned long  SecurityPropertiesMask;
   UUID_128_t     CharacteristicUUID;
   Boolean_t      AllocatedValue;
   unsigned int   MaximumValueLength;
   unsigned int   ValueLength;
   Byte_t        *Value;
} CharacteristicInfo_t;

   /* The following type definition represents the container structure  */
   /* for a Descriptor Attribute.                                       */
typedef struct _tagDescriptorInfo_t
{
   unsigned long  DescriptorPropertiesMask;
   unsigned long  SecurityPropertiesMask;
   UUID_128_t     CharacteristicUUID;
   Boolean_t      AllocatedValue;
   unsigned int   MaximumValueLength;
   unsigned int   ValueLength;
   Byte_t        *Value;
} DescriptorInfo_t;

   /* The following enumeration represents all of the different         */
   /* attributes that may be added in a service table.                  */
typedef enum
{
   atInclude,
   atCharacteristic,
   atDescriptor
} AttributeType_t;

   /* The following type definition represents the container structure  */
   /* for a Service Table.                                              */
   /* * NOTE * For an AttributeType of atInclude the AttributeParameter */
   /*          is not used (the include will automatically reference the*/
   /*          previous service that was registered).                   */
typedef struct _tagAttributeInfo_t
{
   AttributeType_t  AttributeType;
   unsigned int     AttributeOffset;
   void            *Attribute;
} AttributeInfo_t;

   /* The following type definition represents the container structure  */
   /* for an dynamically allocated service.                             */
typedef struct _tagServiceInfo_t
{
   unsigned long                  Flags;
   unsigned int                   ServiceID;
   DWord_t                        PersistentUID;
   UUID_128_t                     ServiceUUID;
   GATT_Attribute_Handle_Group_t  ServiceHandleRange;
   unsigned int                   NumberAttributes;
   AttributeInfo_t               *AttributeList;
} ServiceInfo_t;

#define SERVICE_TABLE_FLAGS_USE_PERSISTENT_UID                 0x00000001
#define SERVICE_TABLE_FLAGS_SECONDARY_SERVICE                  0x00000002

   /*********************************************************************/
   /* Service Tables.                                                   */
   /*********************************************************************/
   /* * NOTE * For simplicity this application will not include Client  */
   /*          Characteristic Configuration Descriptors (CCCD) for      */
   /*          characteristics that are indicatable/notifiable.  This is*/
   /*          because the CCCD is a per client value that is stored    */
   /*          persistently for bonded devices.  This application, whose*/
   /*          only purpose is showing the usage of the APIs, does not  */
   /*          store per client values and also does not store values   */
   /*          persistently.                                            */
   /* * NOTE * To Calculate the AttributeOffset apply the following     */
   /*          formula:                                                 */
   /*                                                                   */
   /*             AttributeOffset = 1 + (NumPrevIncludes * 1) +         */
   /*                               (NumPrevCharacteristics * 2) +      */
   /*                               (NumPrevDescriptors * 1)            */
   /*                                                                   */
   /*          where:                                                   */
   /*                                                                   */
   /*             NumPrevIncludes = The number of previous Include      */
   /*                               Definition that exist in the        */
   /*                               service table prior to the attribute*/
   /*                               (Include, Characteristic or         */
   /*                               Descriptor) that is being added.    */
   /*                                                                   */
   /*             NumPrevCharacteristics = The number of previous       */
   /*                               Characteristics that exist in the   */
   /*                               service table prior to the attribute*/
   /*                               (Include, Characteristic or         */
   /*                               Descriptor) that is being added.    */
   /*                                                                   */
   /*             NumPrevDescriptors = The number of previous           */
   /*                               Descriptors that exist in the       */
   /*                               service table prior to the attribute*/
   /*                               (Include, Characteristic or         */
   /*                               Descriptor) that is being added.    */

   /*********************************************************************/
   /* Service Declaration.                                              */
   /*********************************************************************/

static CharacteristicInfo_t Srv1Attr1=
{
   /* Characteristic Properties.                                        */
   GATM_CHARACTERISTIC_PROPERTIES_READ | GATM_CHARACTERISTIC_PROPERTIES_WRITE | GATM_CHARACTERISTIC_PROPERTIES_INDICATE,

   /* Characteristic Security Properties.                               */
   GATM_SECURITY_PROPERTIES_AUTHENTICATED_ENCRYPTION_WRITE,

   /* Characteristic UUID.                                              */
   { 0x6A, 0xFE, 0x4D, 0x41, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   117,

   /* Current value length.                                             */
   117,

   /* Current value.                                                    */
   (Byte_t *)"VUHhfCQDLvB~!%^&*()-_+={}[]|:;.<>?/#uoPXonUEjisFQjbWGlXmAoQsSSqJwexZsugfFtamHcTun~!%^&*()-_+={}[]|:;.<>?/#AfjLKCeuMia"
};

static CharacteristicInfo_t Srv1Attr2=
{
   /* Characteristic Properties.                                        */
   GATM_CHARACTERISTIC_PROPERTIES_READ | GATM_CHARACTERISTIC_PROPERTIES_WRITE | GATM_CHARACTERISTIC_PROPERTIES_NOTIFY | GATM_CHARACTERISTIC_PROPERTIES_AUTHENTICATED_SIGNED_WRITES,

   /* Characteristic Security Properties.                               */
   GATM_SECURITY_PROPERTIES_UNAUTHENTICATED_ENCRYPTION_READ | GATM_SECURITY_PROPERTIES_UNAUTHENTICATED_ENCRYPTION_WRITE | GATM_SECURITY_PROPERTIES_AUTHENTICATED_SIGNED_WRITES,

   /* Characteristic UUID.                                              */
   { 0x6A, 0xFE, 0x51, 0x96, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   45,

   /* Current value length.                                             */
   0,

   /* Current value.                                                    */
   (Byte_t *)NULL
};

static CharacteristicInfo_t Srv1Attr3=
{
   /* Characteristic Properties.                                        */
   GATM_CHARACTERISTIC_PROPERTIES_WRITE,

   /* Characteristic Security Properties.                               */
   GATM_SECURITY_PROPERTIES_NO_SECURITY,

   /* Characteristic UUID.                                              */
   { 0x6A, 0xFE, 0x52, 0x8B, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   50,

   /* Current value length.                                             */
   0,

   /* Current value.                                                    */
   (Byte_t *)NULL
};


   /* The following array is the array containing attributes that are   */
   /* registered by this service.                                       */
static AttributeInfo_t SrvTable1[] =
{
   { atCharacteristic, 1, (void *)&Srv1Attr1 },
   { atCharacteristic, 3, (void *)&Srv1Attr2 },
   { atCharacteristic, 5, (void *)&Srv1Attr3 }
};

   /*********************************************************************/
   /* End of Service Declaration.                                       */
   /*********************************************************************/

   /*********************************************************************/
   /* Service Declaration.                                              */
   /*********************************************************************/

static CharacteristicInfo_t Srv2Attr2=
{
   /* Characteristic Properties.                                        */
   GATM_CHARACTERISTIC_PROPERTIES_READ | GATM_CHARACTERISTIC_PROPERTIES_WRITE_WO_RESP | GATM_CHARACTERISTIC_PROPERTIES_WRITE,

   /* Characteristic Security Properties.                               */
   GATM_SECURITY_PROPERTIES_AUTHENTICATED_ENCRYPTION_READ | GATM_SECURITY_PROPERTIES_UNAUTHENTICATED_ENCRYPTION_WRITE,

   /* Characteristic UUID.                                              */
   { 0x6A, 0xFE, 0x55, 0xA3, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   464,

   /* Current value length.                                             */
   0,

   /* Current value.                                                    */
   (Byte_t *)NULL
};

static CharacteristicInfo_t Srv2Attr3=
{
   /* Characteristic Properties.                                        */
   GATM_CHARACTERISTIC_PROPERTIES_READ | GATM_CHARACTERISTIC_PROPERTIES_WRITE_WO_RESP,

   /* Characteristic Security Properties.                               */
   GATM_SECURITY_PROPERTIES_NO_SECURITY,

   /* Characteristic UUID.                                              */
   { 0x6A, 0xFE, 0x56, 0x89, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   36,

   /* Current value length.                                             */
   36,

   /* Current value.                                                    */
   (Byte_t *)"rlgzzRztTCqnCBBrsm~!%^&*()-_+={}[]|"
};

static CharacteristicInfo_t Srv2Attr4=
{
   /* Characteristic Properties.                                        */
   GATM_CHARACTERISTIC_PROPERTIES_READ | GATM_CHARACTERISTIC_PROPERTIES_WRITE,

   /* Characteristic Security Properties.                               */
   GATM_SECURITY_PROPERTIES_UNAUTHENTICATED_ENCRYPTION_READ | GATM_SECURITY_PROPERTIES_UNAUTHENTICATED_ENCRYPTION_WRITE,

   /* Characteristic UUID.                                              */
   { 0x6A, 0xFE, 0x5C, 0x40, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   62,

   /* Current value length.                                             */
   0,

   /* Current value.                                                    */
   (Byte_t *)NULL
};

static DescriptorInfo_t Srv2Attr5=
{
   /* Descriptor Properties.                                            */
   GATM_DESCRIPTOR_PROPERTIES_READ,

   /* Descriptor Security Properties.                                   */
   GATM_SECURITY_PROPERTIES_UNAUTHENTICATED_ENCRYPTION_READ,

   /* Descriptor UUID.                                                  */
   { 0x6A, 0xFE, 0x5D, 0x2A, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   264,

   /* Current value length.                                             */
   10,

   /* Current value.                                                    */
   (Byte_t *)"HelloWorld"
};

static CharacteristicInfo_t Srv2Attr6=
{
   /* Characteristic Properties.                                        */
   GATM_CHARACTERISTIC_PROPERTIES_READ | GATM_CHARACTERISTIC_PROPERTIES_WRITE,

   /* Characteristic Security Properties.                               */
   GATM_SECURITY_PROPERTIES_UNAUTHENTICATED_ENCRYPTION_READ | GATM_SECURITY_PROPERTIES_UNAUTHENTICATED_ENCRYPTION_WRITE,

   /* Characteristic UUID.                                              */
   { 0x6A, 0xFE, 0x5E, 0x32, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   321,

   /* Current value length.                                             */
   321,

   /* Current value.                                                    */
   (Byte_t *)"qioFsYUBoESuvABqUCDyOXWmevWjxJsaYYMvpnoY~!%^&*()-_+={}[]|:;.<>?/#ShTtUGfemhchQnTAuZFTUwBSSSNhjDcBlASzZaMDsqWkCVAWCYIiqvIWhUcWUHwPIqKTLiYzQFNugrkvZWwqjRexoJeAiwEZrqjxHIukLiAWhUUoV~!%^&*()-_+={}[]|:;.<>?/#tgvnqYVCfywRZgBTARae~!%^&*()-_+={}[]|:;.<>?/#vmBGQSf~!%^&*()-_+={}[]|:;.<>?/#iJMviBnTpRvLjOTVwKUoAHbOdMTWRlvNXSrRNOyhAaFXRXBGxzOyMigBvOTHCNsuSgxscaBucAhmtuRocF~!%^&*()-_+={}[]|:;.<>?/#AAJbaYbMvoMzYspPdXMjPBGxv~!%^&*()-_+={}[]|:;.<>?/#fTTpc~!%^&*()-_+={}[]|:;.<>?/#mHfzsCpjnhdubiwGAhidAV"
};

static CharacteristicInfo_t Srv2Attr7=
{
   /* Characteristic Properties.                                        */
   GATM_CHARACTERISTIC_PROPERTIES_READ | GATM_CHARACTERISTIC_PROPERTIES_WRITE_WO_RESP,

   /* Characteristic Security Properties.                               */
   GATM_SECURITY_PROPERTIES_UNAUTHENTICATED_ENCRYPTION_READ | GATM_SECURITY_PROPERTIES_AUTHENTICATED_ENCRYPTION_WRITE,

   /* Characteristic UUID.                                              */
   { 0x6A, 0xFE, 0x63, 0x01, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   186,

   /* Current value length.                                             */
   186,

   /* Current value.                                                    */
   (Byte_t *)"kGmFzGcmIWrFLGfeYBwzWRwXCKHlXYdnVPugxwJ~!%^&*()-_+={}[]|:;.<>?/#ZaimuLdaHNDALPEzzCtEaHSZyKHfuSAIa~!%^&*()-_+={}[]|:;.<>?/#nKxCfeXahNzeegKGZBErbDHiYciGOqy~!%^&*()-_+={}[]|:;.<>?/#~!%^&*()-_+={}[]|:;.<>?/#KomahJaknrBClz~!%^&*()-_+={}[]|:;.<>?/#cKQSngsQCDjPABccaYNstELvfkWzxMbIrZtzcxvoTxtSAlDfoysREDZLzW~!%^&*()-_+={}[]|:;.<>?/#CUtI~!%^&*()-_+={}[]|:;.<>?/#"
};

static CharacteristicInfo_t Srv2Attr8=
{
   /* Characteristic Properties.                                        */
   GATM_CHARACTERISTIC_PROPERTIES_WRITE,

   /* Characteristic Security Properties.                               */
   GATM_SECURITY_PROPERTIES_UNAUTHENTICATED_ENCRYPTION_WRITE,

   /* Characteristic UUID.                                              */
   { 0x6A, 0xFE, 0x66, 0xCA, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   120,

   /* Current value length.                                             */
   0,

   /* Current value.                                                    */
   (Byte_t *)NULL
};

static DescriptorInfo_t Srv2Attr9=
{
   /* Descriptor Properties.                                            */
   GATM_DESCRIPTOR_PROPERTIES_READ,

   /* Descriptor Security Properties.                                   */
   GATM_SECURITY_PROPERTIES_NO_SECURITY,

   /* Descriptor UUID.                                                  */
   { 0x6A, 0xFE, 0x67, 0x9A, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   172,

   /* Current value length.                                             */
   10,

   /* Current value.                                                    */
   (Byte_t *)"HelloWorld"
};


   /* The following array is the array containing attributes that are   */
   /* registered by this service.                                       */
static AttributeInfo_t SrvTable2[] =
{
   { atInclude, 1, NULL },
   { atCharacteristic, 2, (void *)&Srv2Attr2 },
   { atCharacteristic, 4, (void *)&Srv2Attr3 },
   { atCharacteristic, 6, (void *)&Srv2Attr4 },
   { atDescriptor, 8, (void *)&Srv2Attr5 },
   { atCharacteristic, 9, (void *)&Srv2Attr6 },
   { atCharacteristic, 11, (void *)&Srv2Attr7 },
   { atCharacteristic, 13, (void *)&Srv2Attr8 },
   { atDescriptor, 15, (void *)&Srv2Attr9 }
};

   /*********************************************************************/
   /* End of Service Declaration.                                       */
   /*********************************************************************/

   /*********************************************************************/
   /* Service Declaration.                                              */
   /*********************************************************************/

static CharacteristicInfo_t Srv3Attr2=
{
   /* Characteristic Properties.                                        */
   GATM_CHARACTERISTIC_PROPERTIES_READ,

   /* Characteristic Security Properties.                               */
   GATM_SECURITY_PROPERTIES_NO_SECURITY,

   /* Characteristic UUID.                                              */
   { 0x6A, 0xFE, 0x69, 0x37, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   229,

   /* Current value length.                                             */
   10,

   /* Current value.                                                    */
   (Byte_t *)"HelloWorld"
};

static CharacteristicInfo_t Srv3Attr3=
{
   /* Characteristic Properties.                                        */
   GATM_CHARACTERISTIC_PROPERTIES_WRITE,

   /* Characteristic Security Properties.                               */
   GATM_SECURITY_PROPERTIES_NO_SECURITY,

   /* Characteristic UUID.                                              */
   { 0x6A, 0xFE, 0x69, 0xF2, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   165,

   /* Current value length.                                             */
   0,

   /* Current value.                                                    */
   (Byte_t *)NULL
};

static CharacteristicInfo_t Srv3Attr4=
{
   /* Characteristic Properties.                                        */
   GATM_CHARACTERISTIC_PROPERTIES_WRITE,

   /* Characteristic Security Properties.                               */
   GATM_SECURITY_PROPERTIES_NO_SECURITY,

   /* Characteristic UUID.                                              */
   { 0x6A, 0xFE, 0x6A, 0xA7, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   495,

   /* Current value length.                                             */
   0,

   /* Current value.                                                    */
   (Byte_t *)NULL
};

static DescriptorInfo_t Srv3Attr5=
{
   /* Descriptor Properties.                                            */
   GATM_DESCRIPTOR_PROPERTIES_READ | GATM_DESCRIPTOR_PROPERTIES_WRITE,

   /* Descriptor Security Properties.                                   */
   GATM_SECURITY_PROPERTIES_AUTHENTICATED_ENCRYPTION_READ,

   /* Descriptor UUID.                                                  */
   { 0x6A, 0xFE, 0x6B, 0x65, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   243,

   /* Current value length.                                             */
   227,

   /* Current value.                                                    */
   (Byte_t *)"SDZSWqRxoK~!%^&*()-_+={}[]|:;.<>?/#BDsKpSyoekWZShYdZmPJEnZPJGCnqAzYGnSewzqxNKLQwHgZVVOevQkuBoZXLeIksvfaRwEjjEBcmVvjdTmvuEoTnDMSfSNaAKlkGLzkyOTzmsW~!%^&*()-_+={}[]|:;.<>?/#czlMJCADkIIDWJAqBnkcUowtmEa~!%^&*()-_+={}[]|:;.<>?/#xkhbaBCPvFBVodzsKqaoqCMDOXjLINajTIJUHoUWJAlAWKDeYsBfqBv~!%^&*()-_+={}[]|:;.<>?/#xyMxzvKpDSB~!%^&*()-_+={}[]|:;.<>?/#jnGiLaKg"
};


   /* The following array is the array containing attributes that are   */
   /* registered by this service.                                       */
static AttributeInfo_t SrvTable3[] =
{
   { atInclude, 1, NULL },
   { atCharacteristic, 2, (void *)&Srv3Attr2 },
   { atCharacteristic, 4, (void *)&Srv3Attr3 },
   { atCharacteristic, 6, (void *)&Srv3Attr4 },
   { atDescriptor, 8, (void *)&Srv3Attr5 }
};

   /*********************************************************************/
   /* End of Service Declaration.                                       */
   /*********************************************************************/

   /*********************************************************************/
   /* Service Declaration.                                              */
   /*********************************************************************/

static CharacteristicInfo_t Srv4Attr2=
{
   /* Characteristic Properties.                                        */
   GATM_CHARACTERISTIC_PROPERTIES_WRITE,

   /* Characteristic Security Properties.                               */
   GATM_SECURITY_PROPERTIES_NO_SECURITY,

   /* Characteristic UUID.                                              */
   { 0x6A, 0xFE, 0x6F, 0x22, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   307,

   /* Current value length.                                             */
   0,

   /* Current value.                                                    */
   (Byte_t *)NULL
};

static CharacteristicInfo_t Srv4Attr3=
{
   /* Characteristic Properties.                                        */
   GATM_CHARACTERISTIC_PROPERTIES_WRITE_WO_RESP | GATM_CHARACTERISTIC_PROPERTIES_WRITE,

   /* Characteristic Security Properties.                               */
   GATM_SECURITY_PROPERTIES_AUTHENTICATED_ENCRYPTION_WRITE,

   /* Characteristic UUID.                                              */
   { 0x6A, 0xFE, 0x6F, 0xFE, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   426,

   /* Current value length.                                             */
   0,

   /* Current value.                                                    */
   (Byte_t *)NULL
};

static DescriptorInfo_t Srv4Attr4=
{
   /* Descriptor Properties.                                            */
   GATM_DESCRIPTOR_PROPERTIES_READ | GATM_DESCRIPTOR_PROPERTIES_WRITE,

   /* Descriptor Security Properties.                                   */
   GATM_SECURITY_PROPERTIES_AUTHENTICATED_ENCRYPTION_READ | GATM_SECURITY_PROPERTIES_UNAUTHENTICATED_ENCRYPTION_WRITE,

   /* Descriptor UUID.                                                  */
   { 0x6A, 0xFE, 0x70, 0xE7, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   490,

   /* Current value length.                                             */
   469,

   /* Current value.                                                    */
   (Byte_t *)"~!%^&*()-_+={}[]|:;.<>?/#urpPyrtiaBXDTzKNyWgozZuCTpXYlkOCVWCbUbZIUrqjUWkNflnkUthgfAJtXYOGsINpAoVBOQwHGcjSEviprvXzjzljZtALcMGPbenLdbHpBObmL~!%^&*()-_+={}[]|:;.<>?/#eigmIow~!%^&*()-_+={}[]|:;.<>?/#UGTUGlhZREyIGDZCqoqmFxYkmrXgKCWBeRypnRDUjJfwmRAJRIWLvzIPUnLBYOFllTUvgQaGwXBQLIwuF~!%^&*()-_+={}[]|:;.<>?/#KGqLVpsnKcXPdOKHDlvLXZukFDtHnnfGiESZCqYEtwGVOiabkWTCHxVgzzUMrAPThqbxnjewzCBOYYHOMRQKhgVrCCGrPfkYMLuJxQfQhBKzFwgByxcaoPnrCVskADR~!%^&*()-_+={}[]|:;.<>?/#vSZOsRyMHCN~!%^&*()-_+={}[]|:;.<>?/#AwbVVsUIGWVDxTwBjjqaOWzfXAEdHaOLnBVtUunpEDSszUNWnOlWQrZzaNbDdwx~!%^&*()-_+={}[]|:;.<>?/#hXxn~!%^&*()-_+={}[]|:;.<>?/#yAWs~!%^&*()-_+={}[]|:;.<>?/#dWmDMwXVkDPDkigimWc~!%^&*()-_+={}[]|:;.<>?/#WzsBmVhLGaRgFxAJrupOWlkrpNFWPO"
};

static CharacteristicInfo_t Srv4Attr5=
{
   /* Characteristic Properties.                                        */
   GATM_CHARACTERISTIC_PROPERTIES_WRITE,

   /* Characteristic Security Properties.                               */
   GATM_SECURITY_PROPERTIES_AUTHENTICATED_ENCRYPTION_WRITE,

   /* Characteristic UUID.                                              */
   { 0x6A, 0xFE, 0x77, 0x51, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   487,

   /* Current value length.                                             */
   0,

   /* Current value.                                                    */
   (Byte_t *)NULL
};

static CharacteristicInfo_t Srv4Attr6=
{
   /* Characteristic Properties.                                        */
   GATM_CHARACTERISTIC_PROPERTIES_WRITE,

   /* Characteristic Security Properties.                               */
   GATM_SECURITY_PROPERTIES_AUTHENTICATED_ENCRYPTION_WRITE,

   /* Characteristic UUID.                                              */
   { 0x6A, 0xFE, 0x78, 0x46, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   267,

   /* Current value length.                                             */
   0,

   /* Current value.                                                    */
   (Byte_t *)NULL
};

static DescriptorInfo_t Srv4Attr7=
{
   /* Descriptor Properties.                                            */
   GATM_DESCRIPTOR_PROPERTIES_WRITE,

   /* Descriptor Security Properties.                                   */
   GATM_SECURITY_PROPERTIES_NO_SECURITY,

   /* Descriptor UUID.                                                  */
   { 0x6A, 0xFE, 0x78, 0xFF, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   189,

   /* Current value length.                                             */
   0,

   /* Current value.                                                    */
   (Byte_t *)NULL
};


   /* The following array is the array containing attributes that are   */
   /* registered by this service.                                       */
static AttributeInfo_t SrvTable4[] =
{
   { atInclude, 1, NULL },
   { atCharacteristic, 2, (void *)&Srv4Attr2 },
   { atCharacteristic, 4, (void *)&Srv4Attr3 },
   { atDescriptor, 6, (void *)&Srv4Attr4 },
   { atCharacteristic, 7, (void *)&Srv4Attr5 },
   { atCharacteristic, 9, (void *)&Srv4Attr6 },
   { atDescriptor, 11, (void *)&Srv4Attr7 }
};

   /*********************************************************************/
   /* End of Service Declaration.                                       */
   /*********************************************************************/

   /*********************************************************************/
   /* Service Declaration.                                              */
   /*********************************************************************/

static CharacteristicInfo_t Srv5Attr1=
{
   /* Characteristic Properties.                                        */
   GATM_CHARACTERISTIC_PROPERTIES_READ | GATM_CHARACTERISTIC_PROPERTIES_WRITE_WO_RESP | GATM_CHARACTERISTIC_PROPERTIES_WRITE,

   /* Characteristic Security Properties.                               */
   GATM_SECURITY_PROPERTIES_AUTHENTICATED_ENCRYPTION_READ | GATM_SECURITY_PROPERTIES_AUTHENTICATED_ENCRYPTION_WRITE,

   /* Characteristic UUID.                                              */
   { 0x6A, 0xFE, 0x7A, 0xCE, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   326,

   /* Current value length.                                             */
   0,

   /* Current value.                                                    */
   (Byte_t *)NULL
};

static CharacteristicInfo_t Srv5Attr2=
{
   /* Characteristic Properties.                                        */
   GATM_CHARACTERISTIC_PROPERTIES_READ | GATM_CHARACTERISTIC_PROPERTIES_WRITE,

   /* Characteristic Security Properties.                               */
   GATM_SECURITY_PROPERTIES_AUTHENTICATED_ENCRYPTION_READ,

   /* Characteristic UUID.                                              */
   { 0x6A, 0xFE, 0x7B, 0xD8, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   233,

   /* Current value length.                                             */
   233,

   /* Current value.                                                    */
   (Byte_t *)"VRWGsXStNGTcZJTqQkQxnTXZBdGjtjBKaAWWGvMFGxDleYXtztTYBvyQquoo~!%^&*()-_+={}[]|:;.<>?/#XuKgBLoyu~!%^&*()-_+={}[]|:;.<>?/#LuaLebVuyVIYaAgAEpEBJnkOoaRPz~!%^&*()-_+={}[]|:;.<>?/#jGwjjXaZrXuSkgJdHzDelfBzFRntFNZQNrxbgcU~!%^&*()-_+={}[]|:;.<>?/#PhdYpBIyxqaibS~!%^&*()-_+={}[]|:;.<>?/#xJtAOtvIUZwCToViALctNDDguIKjWgQMPJPaMSHbGwtCDrJOvnylIZMPWrXvIRibFImIHiEaqwroS"
};

static CharacteristicInfo_t Srv5Attr3=
{
   /* Characteristic Properties.                                        */
   GATM_CHARACTERISTIC_PROPERTIES_READ | GATM_CHARACTERISTIC_PROPERTIES_WRITE,

   /* Characteristic Security Properties.                               */
   GATM_SECURITY_PROPERTIES_UNAUTHENTICATED_ENCRYPTION_WRITE,

   /* Characteristic UUID.                                              */
   { 0x6A, 0xFE, 0x7F, 0xD9, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   220,

   /* Current value length.                                             */
   220,

   /* Current value.                                                    */
   (Byte_t *)"abERkFtyw~!%^&*()-_+={}[]|:;.<>?/#PFyhGjfDuAdCqIgBllooqMAxQcvoVIOzwPI~!%^&*()-_+={}[]|:;.<>?/#KDfuLyHqdPlbonxkrtMcsHyrDKuIFHNFctzEsbdDlYyhpTs~!%^&*()-_+={}[]|:;.<>?/#uhHpehiGaLXhvqzGDcZjGqkYinUWovoDGXPHUoxErJTaLoIkESbgRMeHJAmEfwHsCzxF~!%^&*()-_+={}[]|:;.<>?/#fwecsjvwosyRUJxjZVoauSlBRglfi~!%^&*()-_+={}[]|:;.<>?/#MBXtEdkCzlrLhyYKkMcaeLnNaqO"
};

static CharacteristicInfo_t Srv5Attr4=
{
   /* Characteristic Properties.                                        */
   GATM_CHARACTERISTIC_PROPERTIES_WRITE_WO_RESP | GATM_CHARACTERISTIC_PROPERTIES_WRITE,

   /* Characteristic Security Properties.                               */
   GATM_SECURITY_PROPERTIES_AUTHENTICATED_ENCRYPTION_WRITE,

   /* Characteristic UUID.                                              */
   { 0x6A, 0xFE, 0x83, 0x6B, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   296,

   /* Current value length.                                             */
   0,

   /* Current value.                                                    */
   (Byte_t *)NULL
};

static CharacteristicInfo_t Srv5Attr5=
{
   /* Characteristic Properties.                                        */
   GATM_CHARACTERISTIC_PROPERTIES_READ | GATM_CHARACTERISTIC_PROPERTIES_WRITE_WO_RESP,

   /* Characteristic Security Properties.                               */
   GATM_SECURITY_PROPERTIES_UNAUTHENTICATED_ENCRYPTION_WRITE,

   /* Characteristic UUID.                                              */
   { 0x6A, 0xFE, 0x84, 0x56, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   494,

   /* Current value length.                                             */
   0,

   /* Current value.                                                    */
   (Byte_t *)NULL
};

static DescriptorInfo_t Srv5Attr6=
{
   /* Descriptor Properties.                                            */
   GATM_DESCRIPTOR_PROPERTIES_READ,

   /* Descriptor Security Properties.                                   */
   GATM_SECURITY_PROPERTIES_NO_SECURITY,

   /* Descriptor UUID.                                                  */
   { 0x6A, 0xFE, 0x85, 0x21, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   417,

   /* Current value length.                                             */
   349,

   /* Current value.                                                    */
   (Byte_t *)"ZLyNTJKNaxZcH~!%^&*()-_+={}[]|:;.<>?/#EEIealwBbtwOnKTCRafurXYsYTnhaBeUWgqmeIcFdcoQRFDKsLdOmq~!%^&*()-_+={}[]|:;.<>?/#NQEDYGgBuJrftgIHUXJNrISkqOJkjEYwqxGfJUVVuQBqSgDwiJzCJSptJykLhVBffIJXqaBEgZggyioVUYJhCJrjXtGNCWirdaAefsirCxyFiAjfxfCZSEVdXewuByBoUFWvIw~!%^&*()-_+={}[]|:;.<>?/#oyjZNkHouTeuwRkzTsUY~!%^&*()-_+={}[]|:;.<>?/#zAUBeqLLSUMYvMh~!%^&*()-_+={}[]|:;.<>?/#ihbCdBBLYkbVFLNZfpYfTBNLPFIxKwuFTKxQINtabCfRWiqEufdurxVaXCDb~!%^&*()-_+={}[]|:;.<>?/#goQypeNfKntfqMLrxfZCvYknOefYToAGvqLqVcwFZGMAJBd"
};

static CharacteristicInfo_t Srv5Attr7=
{
   /* Characteristic Properties.                                        */
   GATM_CHARACTERISTIC_PROPERTIES_READ | GATM_CHARACTERISTIC_PROPERTIES_WRITE_WO_RESP,

   /* Characteristic Security Properties.                               */
   GATM_SECURITY_PROPERTIES_AUTHENTICATED_ENCRYPTION_READ,

   /* Characteristic UUID.                                              */
   { 0x6A, 0xFE, 0x8A, 0x37, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   374,

   /* Current value length.                                             */
   10,

   /* Current value.                                                    */
   (Byte_t *)"HelloWorld"
};

static CharacteristicInfo_t Srv5Attr8=
{
   /* Characteristic Properties.                                        */
   GATM_CHARACTERISTIC_PROPERTIES_READ | GATM_CHARACTERISTIC_PROPERTIES_WRITE_WO_RESP,

   /* Characteristic Security Properties.                               */
   GATM_SECURITY_PROPERTIES_UNAUTHENTICATED_ENCRYPTION_WRITE,

   /* Characteristic UUID.                                              */
   { 0x6A, 0xFE, 0x8A, 0xFF, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   213,

   /* Current value length.                                             */
   10,

   /* Current value.                                                    */
   (Byte_t *)"HelloWorld"
};

static CharacteristicInfo_t Srv5Attr9=
{
   /* Characteristic Properties.                                        */
   GATM_CHARACTERISTIC_PROPERTIES_READ | GATM_CHARACTERISTIC_PROPERTIES_WRITE_WO_RESP | GATM_CHARACTERISTIC_PROPERTIES_WRITE,

   /* Characteristic Security Properties.                               */
   GATM_SECURITY_PROPERTIES_AUTHENTICATED_ENCRYPTION_WRITE,

   /* Characteristic UUID.                                              */
   { 0x6A, 0xFE, 0x8B, 0xCD, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   418,

   /* Current value length.                                             */
   0,

   /* Current value.                                                    */
   (Byte_t *)NULL
};


   /* The following array is the array containing attributes that are   */
   /* registered by this service.                                       */
static AttributeInfo_t SrvTable5[] =
{
   { atCharacteristic, 1, (void *)&Srv5Attr1 },
   { atCharacteristic, 3, (void *)&Srv5Attr2 },
   { atCharacteristic, 5, (void *)&Srv5Attr3 },
   { atCharacteristic, 7, (void *)&Srv5Attr4 },
   { atCharacteristic, 9, (void *)&Srv5Attr5 },
   { atDescriptor, 11, (void *)&Srv5Attr6 },
   { atCharacteristic, 12, (void *)&Srv5Attr7 },
   { atCharacteristic, 14, (void *)&Srv5Attr8 },
   { atCharacteristic, 16, (void *)&Srv5Attr9 }
};

   /*********************************************************************/
   /* End of Service Declaration.                                       */
   /*********************************************************************/

   /*********************************************************************/
   /* Service Declaration.                                              */
   /*********************************************************************/

static CharacteristicInfo_t Srv6Attr2=
{
   /* Characteristic Properties.                                        */
   GATM_CHARACTERISTIC_PROPERTIES_WRITE,

   /* Characteristic Security Properties.                               */
   GATM_SECURITY_PROPERTIES_NO_SECURITY,

   /* Characteristic UUID.                                              */
   { 0x6A, 0xFE, 0x8D, 0xCB, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   374,

   /* Current value length.                                             */
   0,

   /* Current value.                                                    */
   (Byte_t *)NULL
};

static CharacteristicInfo_t Srv6Attr3=
{
   /* Characteristic Properties.                                        */
   GATM_CHARACTERISTIC_PROPERTIES_READ | GATM_CHARACTERISTIC_PROPERTIES_WRITE_WO_RESP,

   /* Characteristic Security Properties.                               */
   GATM_SECURITY_PROPERTIES_UNAUTHENTICATED_ENCRYPTION_WRITE,

   /* Characteristic UUID.                                              */
   { 0x6A, 0xFE, 0x8E, 0xB5, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

   /* Value is allocated dynamically.                                   */
   FALSE,

   /* Maximum value length.                                             */
   255,

   /* Current value length.                                             */
   255,

   /* Current value.                                                    */
   (Byte_t *)"JHRgqzMWRhsbGOsxWrszFcyPpOzv~!%^&*()-_+={}[]|:;.<>?/#gJQbSFvpLKbBktISgdCfOgAgDcAOEMMsrEajXMvKkAPBxxoKTIFRAnWfPpegGfCzFDjRgZlyskytEIQDihRKbzPGJzXkwinWguoxkdQOLoJqbjjgjcRdzBeewuZKkEhZjrGpINilcPBxNhepqxHeJnFfotxgNcxnmnYuJTc~!%^&*()-_+={}[]|:;.<>?/#muyF~!%^&*()-_+={}[]|:;.<>?/#LmriVMHhxlGLoaIJnoGXAxCcokMphszDikMuGW~!%^&*()-_+={}[]|:;.<>?/#YXASuzLtfXImfc"
};


   /* The following array is the array containing attributes that are   */
   /* registered by this service.                                       */
static AttributeInfo_t SrvTable6[] =
{
   { atInclude, 1, NULL },
   { atCharacteristic, 2, (void *)&Srv6Attr2 },
   { atCharacteristic, 4, (void *)&Srv6Attr3 }
};

   /*********************************************************************/
   /* End of Service Declaration.                                       */
   /*********************************************************************/

   /* The following array is the array containing attributes that are   */
   /* registered by this service.                                       */
static ServiceInfo_t ServiceTable[] =
{
   {
      /* Service Flags.                                                 */
      0,

      /* Service ID.                                                    */
      0,

      /* PersistentUID.                                                 */
      0,

      /* Service UUID.                                                  */
      { 0x6A, 0xFE, 0x54, 0x81, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

      /* Service Handle Range.                                          */
      {0, 0},

      /* Number of Service Table Entries.                               */
      3,

      /* Service Attribute List.                                        */
      SrvTable1
   },
   {
      /* Service Flags.                                                 */
      SERVICE_TABLE_FLAGS_USE_PERSISTENT_UID,

      /* Service ID.                                                    */
      0,

      /* PersistentUID.                                                 */
      0,

      /* Service UUID.                                                  */
      { 0x6A, 0xFE, 0x68, 0x65, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

      /* Service Handle Range.                                          */
      {0, 0},

      /* Number of Service Table Entries.                               */
      9,

      /* Service Attribute List.                                        */
      SrvTable2
   },
   {
      /* Service Flags.                                                 */
      SERVICE_TABLE_FLAGS_USE_PERSISTENT_UID,

      /* Service ID.                                                    */
      0,

      /* PersistentUID.                                                 */
      0,

      /* Service UUID.                                                  */
      { 0x6A, 0xFE, 0x6E, 0x1B, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

      /* Service Handle Range.                                          */
      {0, 0},

      /* Number of Service Table Entries.                               */
      5,

      /* Service Attribute List.                                        */
      SrvTable3
   },
   {
      /* Service Flags.                                                 */
      0,

      /* Service ID.                                                    */
      0,

      /* PersistentUID.                                                 */
      0,

      /* Service UUID.                                                  */
      { 0x6A, 0xFE, 0x79, 0xCE, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

      /* Service Handle Range.                                          */
      {0, 0},

      /* Number of Service Table Entries.                               */
      7,

      /* Service Attribute List.                                        */
      SrvTable4
   },
   {
      /* Service Flags.                                                 */
      SERVICE_TABLE_FLAGS_SECONDARY_SERVICE | SERVICE_TABLE_FLAGS_USE_PERSISTENT_UID,

      /* Service ID.                                                    */
      0,

      /* PersistentUID.                                                 */
      0,

      /* Service UUID.                                                  */
      { 0x6A, 0xFE, 0x8C, 0xB1, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

      /* Service Handle Range.                                          */
      {0, 0},

      /* Number of Service Table Entries.                               */
      9,

      /* Service Attribute List.                                        */
      SrvTable5
   },
   {
      /* Service Flags.                                                 */
      SERVICE_TABLE_FLAGS_USE_PERSISTENT_UID,

      /* Service ID.                                                    */
      0,

      /* PersistentUID.                                                 */
      0,

      /* Service UUID.                                                  */
      { 0x6A, 0xFE, 0x92, 0x3A, 0x6C, 0x52, 0x10, 0x14, 0x8B, 0x7A, 0x95, 0x02, 0x01, 0xB8, 0x65, 0xBC  },

      /* Service Handle Range.                                          */
      {0, 0},

      /* Number of Service Table Entries.                               */
      3,

      /* Service Attribute List.                                        */
      SrvTable6
   }
};

#define NUMBER_OF_SERVICES                                     (sizeof(ServiceTable)/sizeof(ServiceInfo_t))

#endif


/*****< CPPMUTIL.c >***********************************************************/
/*      Copyright (c) 2016 Qualcomm Technologies, Inc.                        */
/*      All Rights Reserved                                                   */
/*                                                                            */
/*  CPPMUTIL - Cycling Power Profile Manager Utility Functions                */
/*                                                                            */
/*  Author:  Glenn Steenrod                                                   */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/06/15  G. Steenrod    Initial creation.                               */
/******************************************************************************/

#include "SS1BTCPS.h"            /* Bluetopia CPS Prototypes/Constants.       */
#include "SS1BTPM.h"                                                               
#include "CPPMType.h"
#include "CPPMAPI.h"
#include "CPPMUTIL.h"

   /* Utility Functions                                                 */
   /*    Remote Device List Functions                                   */
   /*       Service Instance List Functions                             */
   /*       Transaction List Functions                                  */
   /*    Registered Callback List Functions                             */
   /*       Active Notifiers and Indicators List Functions              */

static Transaction_Entry_t *DeleteTransactionEntry(Transaction_Entry_t **TransactionEntryList, unsigned int TransactionID);



   /*********************************************************************/
   /*      Device Entry List Functions                                  */
   /*********************************************************************/

   /* AddDeviceEntryActual adds an entry to the global device list      */
   /* only if no entry with the same bluetooth address is already       */
   /* present.                                                          */
Boolean_t AddDeviceEntryActual(Device_Entry_t **DeviceEntryList, Device_Entry_t *EntryToAdd)
{
   return(BSC_AddGenericListEntry_Actual(ekBD_ADDR_t, BTPS_STRUCTURE_OFFSET(Device_Entry_t, BluetoothAddress), BTPS_STRUCTURE_OFFSET(Device_Entry_t, NextDeviceEntry), (void **)DeviceEntryList, (void *)EntryToAdd));
}

   /* SearchDeviceEntry returns a device entry with the specified       */
   /* bluetooth address if such an entry is present in the provided     */
   /* list.                                                             */
Device_Entry_t *SearchDeviceEntry(Device_Entry_t **DeviceEntryList, BD_ADDR_t *Address)
{
   return((Device_Entry_t *)BSC_SearchGenericListEntry(ekBD_ADDR_t, (void *)Address, BTPS_STRUCTURE_OFFSET(Device_Entry_t, BluetoothAddress), BTPS_STRUCTURE_OFFSET(Device_Entry_t, NextDeviceEntry), (void **)DeviceEntryList));
}

   /* DeleteDeviceEntry removes a device entry with the specified       */
   /* bluetooth address if such an entry is present in the provided     */
   /* list. It returns a pointer to the entry.                          */
Device_Entry_t *DeleteDeviceEntry(Device_Entry_t **DeviceEntryList, BD_ADDR_t *Address)
{
   return((Device_Entry_t *)BSC_DeleteGenericListEntry(ekBD_ADDR_t, (void *)Address, BTPS_STRUCTURE_OFFSET(Device_Entry_t, BluetoothAddress), BTPS_STRUCTURE_OFFSET(Device_Entry_t, NextDeviceEntry), (void **)DeviceEntryList));
}

   /* FreeDeviceEntryList frees all the entries of a device list.       */
void FreeDeviceEntryList(Device_Entry_t **DeviceEntryList)
{
   Device_Entry_t *DeviceEntry;
   Device_Entry_t *tmpDeviceEntry;

   /* Loop through the list and free each Device entry.                 */
   for(DeviceEntry = *DeviceEntryList; DeviceEntry;)
   {
      tmpDeviceEntry = DeviceEntry;
      DeviceEntry    = DeviceEntry->NextDeviceEntry;

      FreeDeviceEntryMemory(tmpDeviceEntry);
   }

   DeviceEntryList = NULL;
}

   /* FreeDeviceEntryMemory frees the instances list element of a       */
   /* device entry and then the entry itself.                           */
void FreeDeviceEntryMemory(Device_Entry_t *EntryToFree)
{
   /* Free the Instance list before freeing the Device entry.           */
   BSC_FreeGenericListEntryList((void **)&(EntryToFree->InstanceEntryList), BTPS_STRUCTURE_OFFSET(Instance_Entry_t, NextInstanceEntry));

   BSC_FreeGenericListEntryMemory((void *)EntryToFree);
}




   /*********************************************************************/
   /*      Instance Entry List Functions                                */
   /*********************************************************************/

   /* SearchDeviceInstanceEntry searches the provided device list for   */
   /* an entry with the specified bluetooth address and then searches   */
   /* that entry's instance list for an Instance with the specified ID. */
   /* If such an instance is found, it is returned.                     */
Instance_Entry_t *SearchDeviceInstanceEntry(Device_Entry_t **DeviceEntryList, BD_ADDR_t *Address, unsigned int InstanceID)
{
   Device_Entry_t   *DeviceEntry;
   Instance_Entry_t *InstanceEntry;

   if((DeviceEntry = SearchDeviceEntry(DeviceEntryList, Address)) != NULL)
   {
      InstanceEntry = SearchInstanceEntry(&(DeviceEntry->InstanceEntryList), InstanceID);
   }
   else
      InstanceEntry = NULL;

   return(InstanceEntry);
}

   /* SearchInstanceEntryByAttributeHandle searches the provided        */
   /* instance list for the specified attribute handle in the service   */
   /* handles set of an instance.                                       */
Instance_Entry_t *SearchInstanceEntryByAttributeHandle(Instance_Entry_t **EntryList, unsigned int AttributeOffset, Word_t AttributeHandle)
{
   Instance_Entry_t *Result;

   if((AttributeOffset >= BTPS_STRUCTURE_OFFSET(Instance_Entry_t, ServiceHandles)) && (AttributeOffset <= (BTPS_STRUCTURE_OFFSET(Instance_Entry_t, ServiceHandles) + CPS_CLIENT_INFORMATION_DATA_SIZE)))
   {
      Result = (Instance_Entry_t *)BSC_SearchGenericListEntry(ekWord_t, (void *)&AttributeHandle, AttributeOffset, BTPS_STRUCTURE_OFFSET(Instance_Entry_t, NextInstanceEntry), (void **)EntryList);
   }
   else
      Result = NULL;

   return(Result);
}

   /* SearchInstanceEntry returns an instance with the specified        */
   /*  instance ID if it finds one in the list                          */ 
Instance_Entry_t *SearchInstanceEntry(Instance_Entry_t **EntryList, unsigned int InstanceID)
{
   return((Instance_Entry_t *)BSC_SearchGenericListEntry(ekUnsignedInteger, (void *)&InstanceID, BTPS_STRUCTURE_OFFSET(Instance_Entry_t, InstanceID), BTPS_STRUCTURE_OFFSET(Instance_Entry_t, NextInstanceEntry), (void **)EntryList));
}

   /* AddInstanceEntryActual adds an instance entry to a list ensuring  */
   /* that an entry with the same instance ID is not already present.   */
Boolean_t AddInstanceEntryActual(Instance_Entry_t **EntryList, Instance_Entry_t *EntryToAdd)
{
   return(BSC_AddGenericListEntry_Actual(ekUnsignedInteger, BTPS_STRUCTURE_OFFSET(Instance_Entry_t, InstanceID), BTPS_STRUCTURE_OFFSET(Instance_Entry_t, NextInstanceEntry), (void **)EntryList, (void *)EntryToAdd));
}

   /* CalculateNumberOfInstances returns the number of instances in a   */
   /* list.                                                             */
unsigned int CalculateNumberOfInstances(Instance_Entry_t *InstanceEntryList)
{
   unsigned int      Result;
   Instance_Entry_t *InstanceEntry;

   for(InstanceEntry = InstanceEntryList, Result = 0; InstanceEntry; InstanceEntry = InstanceEntry->NextInstanceEntry, Result++);

   return(Result);
}



   /*********************************************************************/
   /*      Tranaction Entry List Functions                              */
   /*********************************************************************/

   /* AddTransactionEntry adds an entry to a transaction list.          */
Transaction_Entry_t *AddTransactionEntry(Transaction_Entry_t **TransactionEntryList, Transaction_Entry_t *EntryToAdd)
{
   return((Transaction_Entry_t *)BSC_AddGenericListEntry(sizeof(Transaction_Entry_t), ekUnsignedInteger, BTPS_STRUCTURE_OFFSET(Transaction_Entry_t, TransactionID), sizeof(Transaction_Entry_t), BTPS_STRUCTURE_OFFSET(Transaction_Entry_t, NextTransactionEntry), (void **)TransactionEntryList, ((void *)EntryToAdd)));
}

   /* DeleteTransactionEntry searches for a transaction entry in a list */
   /* with the specified transaction ID and removes it.                 */
static Transaction_Entry_t *DeleteTransactionEntry(Transaction_Entry_t **TransactionEntryList, unsigned int TransactionID)
{
   return((Transaction_Entry_t *)BSC_DeleteGenericListEntry(ekUnsignedInteger, (void *)&(TransactionID), BTPS_STRUCTURE_OFFSET(Transaction_Entry_t, TransactionID), BTPS_STRUCTURE_OFFSET(Transaction_Entry_t, NextTransactionEntry), (void **)TransactionEntryList));
}

   /* FreeTransactionEntryMemory frees the specified entry's memory.    */
void FreeTransactionEntryMemory(Transaction_Entry_t *EntryToFree)
{
   BSC_FreeGenericListEntryMemory((void *)EntryToFree);
}



   /*********************************************************************/
   /*      Callback Entry List Functions                                */
   /*********************************************************************/

   /* AddCallbackEntry adds a callback entry to the specified list.     */
Callback_Entry_t *AddCallbackEntry(Callback_Entry_t **CallbackEntryList, Callback_Entry_t *EntryToAdd)
{
   return((Callback_Entry_t *)BSC_AddGenericListEntry(sizeof(Callback_Entry_t), ekUnsignedInteger, BTPS_STRUCTURE_OFFSET(Callback_Entry_t, CallbackID), sizeof(Callback_Entry_t), BTPS_STRUCTURE_OFFSET(Callback_Entry_t, NextCallbackEntry), (void **)CallbackEntryList, ((void *)EntryToAdd)));
}

   /* DeleteCallbackEntry searches for an entry in a callback list with */
   /* the specified callback ID and removes it from the list. It        */
   /* returns the removed entry.                                        */                                   
Callback_Entry_t *DeleteCallbackEntry(Callback_Entry_t **CallbackEntryList, unsigned int CallbackID)
{
   return((Callback_Entry_t *)BSC_DeleteGenericListEntry(ekUnsignedInteger, (void *)&CallbackID, BTPS_STRUCTURE_OFFSET(Callback_Entry_t, CallbackID), BTPS_STRUCTURE_OFFSET(Callback_Entry_t, NextCallbackEntry), (void **)CallbackEntryList));
}

   /* SearchCallbackEntry searches for an entry in a callback list with */
   /* the specified callback ID. If such an entry is found, it is       */
   /* returned.                                                         */
Callback_Entry_t *SearchCallbackEntry(Callback_Entry_t **CallbackEntryList, unsigned int CallbackID)
{
   return((Callback_Entry_t *)BSC_SearchGenericListEntry(ekUnsignedInteger, (void *)&CallbackID, BTPS_STRUCTURE_OFFSET(Callback_Entry_t, CallbackID), BTPS_STRUCTURE_OFFSET(Callback_Entry_t, NextCallbackEntry), (void **)CallbackEntryList));
}

   /* SearchCallbackEntryByTransactionID searches the transaction lists */
   /* of every callback entry in a callback list for a transaction with */
   /* the specified ID. If found, it is returned.                       */
void SearchCallbackEntryByTransactionID(Callback_Entry_t **CallbackEntryList, Callback_Entry_t **CallbackEntry, Transaction_Entry_t **TransactionEntry, unsigned int TransactionID)
{
   for(*CallbackEntry = *CallbackEntryList; *CallbackEntry;)
   {  
      if((*TransactionEntry = DeleteTransactionEntry(&((*CallbackEntry)->TransactionEntryList), TransactionID)) != NULL)
      {
         break;
      }
      else
      {
         *CallbackEntry = (*CallbackEntry)->NextCallbackEntry;
      }
   }
}

   /* FreeCallbackEntryList frees the memory of every calllback entry   */
   /* in a list.                                                        */
void FreeCallbackEntryList(Callback_Entry_t **CallbackEntryList)
{
   Callback_Entry_t *CallbackEntry;
   Callback_Entry_t *tmpCallbackEntry;

   /* Loop through the list and free each callback entry.               */
   for(CallbackEntry = *CallbackEntryList; CallbackEntry;)
   {
      tmpCallbackEntry = CallbackEntry;
      CallbackEntry    = CallbackEntry->NextCallbackEntry;

      FreeCallbackEntryMemory(tmpCallbackEntry);
   }

   CallbackEntryList = NULL;
}

   /* FreeCallbackEntryMemory frees the entries in the specified        */
   /* callback entry's unsolicited updates list and transactions list   */
   /* and then frees the memory of the callback entry itself.           */ 
void FreeCallbackEntryMemory(Callback_Entry_t *EntryToFree)
{
   /* Free the Notifications-Indications enabled list.                  */
   BSC_FreeGenericListEntryList((void **)&(EntryToFree->UnsolicitedUpdateEntryList), BTPS_STRUCTURE_OFFSET(Unsolicited_Update_Entry_t, NextUnsolicitedUpdateEntry));

   /* Free the Transaction list before freeing the Device entry.        */
   BSC_FreeGenericListEntryList((void **)&(EntryToFree->TransactionEntryList), BTPS_STRUCTURE_OFFSET(Transaction_Entry_t, NextTransactionEntry));
   
   BSC_FreeGenericListEntryMemory((void *)EntryToFree);
}



   /*********************************************************************/
   /*      Unsolicited Update Entry List Functions                      */
   /*********************************************************************/

   /* AddUnsolicitedUpdateEntry adds a unique entry to the unsolicited  */
   /* update list of a callback entry. An entry is considered a         */
   /* duplicate if the address, instance ID and handle are the same.    */ 
Unsolicited_Update_Entry_t *AddUnsolicitedUpdateEntry(Unsolicited_Update_Entry_t **EntryList, Unsolicited_Update_Entry_t *EntryToAdd)
{
   Unsolicited_Update_Entry_t *UnsolicitedUpdateEntry;

   /* Iterate the list to check for any duplicate entries.              */
   for(UnsolicitedUpdateEntry = *EntryList; UnsolicitedUpdateEntry;)
   {
      if(COMPARE_BD_ADDR(UnsolicitedUpdateEntry->BluetoothAddress, EntryToAdd->BluetoothAddress) && (UnsolicitedUpdateEntry->InstanceID == EntryToAdd->InstanceID) && (UnsolicitedUpdateEntry->Handle == EntryToAdd->Handle))
      {
         /* A duplicate has been found.                                 */
         break;
      }
      else
         UnsolicitedUpdateEntry = UnsolicitedUpdateEntry->NextUnsolicitedUpdateEntry;
   }

   /* If a duplicate was not found, then add the entry to the list.     */
   if(!UnsolicitedUpdateEntry)
   {
      UnsolicitedUpdateEntry = (Unsolicited_Update_Entry_t *)BSC_AddGenericListEntry(sizeof(Unsolicited_Update_Entry_t), ekNone, 0, sizeof(Unsolicited_Update_Entry_t), BTPS_STRUCTURE_OFFSET(Unsolicited_Update_Entry_t, NextUnsolicitedUpdateEntry), (void **)EntryList, ((void *)EntryToAdd));
   }

   return(UnsolicitedUpdateEntry);
}

   /* SearchUnsolicitedUpdateEntry searches for an entry in an          */
   /* unsolicited updates list with the specified bluetooth address,    */
   /* instance ID and attribute handle value. If such an entry is found */
   /* it returns it.                                                    */ 
Unsolicited_Update_Entry_t *SearchUnsolicitedUpdateEntry(Unsolicited_Update_Entry_t **EntryList, BD_ADDR_t *BluetoothAddress, unsigned int InstanceID, Word_t Handle)
{
   Unsolicited_Update_Entry_t *UnsolicitedUpdateEntry;

   if(EntryList)
   {
      /* Search for the specified entry.                                */
      for(UnsolicitedUpdateEntry = *EntryList; UnsolicitedUpdateEntry;)
      {
         if(COMPARE_BD_ADDR(UnsolicitedUpdateEntry->BluetoothAddress, *BluetoothAddress) && (UnsolicitedUpdateEntry->InstanceID == InstanceID) && (UnsolicitedUpdateEntry->Handle == Handle))
         {
            /* The entry has been found.                                */
            break;
         }
         else
            UnsolicitedUpdateEntry = UnsolicitedUpdateEntry->NextUnsolicitedUpdateEntry;
      }
   }
   else
      UnsolicitedUpdateEntry = NULL;

   return(UnsolicitedUpdateEntry);
}

   /* DeleteUnsolicitedUpdateEntry searches for an entry in an          */
   /* unsolicited update list with the specified address, instance ID   */
   /* and attribute handle and, if it finds it, removes it from the     */
   /* list.                                                             */
Unsolicited_Update_Entry_t *DeleteUnsolicitedUpdateEntry(Unsolicited_Update_Entry_t **EntryList, BD_ADDR_t *BluetoothAddress, unsigned int InstanceID, Word_t Handle)
{
   Unsolicited_Update_Entry_t *UnsolicitedUpdate;

   /* Search for the entry.                                             */
   if((UnsolicitedUpdate = SearchUnsolicitedUpdateEntry(EntryList, BluetoothAddress, InstanceID, Handle)) != NULL)
   {
      /* Remove the entry from the list.                                */
      UnsolicitedUpdate = BSC_DeleteGenericListEntry(ekEntryPointer, (void *)UnsolicitedUpdate, 0, BTPS_STRUCTURE_OFFSET(Unsolicited_Update_Entry_t, NextUnsolicitedUpdateEntry), (void **)EntryList);
   }

   return(UnsolicitedUpdate);
}

   /* FreeUnsolicitedUpdateEntryMemory frees the memory of an           */
   /* unsolicited update entry.                                         */ 
void FreeUnsolicitedUpdateEntryMemory(Unsolicited_Update_Entry_t *EntryToFree)
{
   BSC_FreeGenericListEntryMemory((void *)EntryToFree);
}


   /* GetNextID increments the value pointed to by the NextID parameter */
   /* by one.                                                           */
void GetNextID(unsigned int *NextID)
{
   if(NextID)
   {
      ++(*NextID);

      if(*NextID & 0x80000000)
         *NextID = 1;
   }
}


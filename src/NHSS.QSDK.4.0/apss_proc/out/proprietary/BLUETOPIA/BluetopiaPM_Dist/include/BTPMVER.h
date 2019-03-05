/*****< btpmver.h >************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMVER - Stonestreet One Bluetooth Protocol Stack Platform Manager       */
/*            Version Information.                                            */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/23/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __BTPMVERH__
#define __BTPMVERH__

   /* Bluetooth Protocol Stack Platform Manager Major and Minor Version */
   /* Numbers.                                                          */
#define BTPM_VERSION_MAJOR_VERSION_NUMBER                   4
#define BTPM_VERSION_MINOR_VERSION_NUMBER                   0

   /* Bluetooth Protocol Stack Platform Manager Release Number.         */
#ifndef BTPM_VERSION_RELEASE_NUMBER
   #define BTPM_VERSION_RELEASE_NUMBER                      1
#endif

   /* Bluetooth Protocol Stack Platform Manager Revision Number.        */
#ifndef BTPM_VERSION_REVISION_NUMBER
   #define BTPM_VERSION_REVISION_NUMBER                     1
#endif

   /* Constants used to convert numeric constants to string constants   */
   /* (used in MACRO's below).                                          */
#define BTPM_VERSION_NUMBER_TO_STRING(_x)                   #_x
#define BTPM_VERSION_CONSTANT_TO_STRING(_y)                 BTPM_VERSION_NUMBER_TO_STRING(_y)

   /* Bluetooth Protocol Stack Platform Manager Constant Version String */
   /* of the form:                                                      */
   /*    "a.b.c.d"                                                      */
   /* where:                                                            */
   /*    a - BTPM_VERSION_MAJOR_VERSION_NUMBER                          */
   /*    b - BTPM_VERSION_MINOR_VERSION_NUMBER                          */
   /*    c - BTPM_VERSION_RELEASE_NUMBER                                */
   /*    d - BTPM_VERSION_REVISION_NUMBER                               */
#define BTPM_VERSION_VERSION_STRING                         BTPM_VERSION_CONSTANT_TO_STRING(BTPM_VERSION_MAJOR_VERSION_NUMBER) "." BTPM_VERSION_CONSTANT_TO_STRING(BTPM_VERSION_MINOR_VERSION_NUMBER) "." BTPM_VERSION_CONSTANT_TO_STRING(BTPM_VERSION_RELEASE_NUMBER) "." BTPM_VERSION_CONSTANT_TO_STRING(BTPM_VERSION_REVISION_NUMBER)

   /* File/Product/Company Name Copyrights and Descriptions.            */
#define BTPM_VERSION_PRODUCT_NAME_STRING                    "Bluetopia Platform Manager"

#define BTPM_VERSION_COMPANY_NAME_STRING                    "Stonestreet One"

#define BTPM_VERSION_COPYRIGHT_STRING                       "Copyright (C) 2000-14 Stonestreet One"

#endif

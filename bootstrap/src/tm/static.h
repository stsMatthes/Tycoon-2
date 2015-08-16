/*
 * This file is part of the Tycoon-2 system.
 *
 * The Tycoon-2 system is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation (Version 2).
 *
 * The Tycoon-2 system is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public
 * License along with the Tycoon-2 system; see the file LICENSE.
 * If not, write to AB 4.02, Softwaresysteme, TU Hamburg-Harburg
 * D-21071 Hamburg, Germany. http://www.sts.tu-harburg.de
 * 
 * Copyright (c) 1996-1998 Higher-Order GmbH, Hamburg. All rights reserved.
 *
 */
/*
  Copyright (c) 1996 Higher-Order GmbH, Hamburg. All rights reserved.

  $File: //depot/tycoon2/stsmain/bootstrap/src/tm/static.h $ $Revision: #3 $ $Date: 2003/10/02 $ Andreas Gawecki, Marc Weikard, Andre Willomat

  List of statically linked symbols
  
  Add lines in the form

    LIB(lib,ext)
    SYM(sym1)
    SYM(sym2)
    ...
    SYM(symN)
    ENDLIB
    
  for each symbol sym1..symN and library "lib.ext" you will reference from TYC.
  
*/


#ifdef rt_LIB_ADABAS

  /* ADABAS D ODBC library */

  LIB(libodbc,so)
  SYM(SQLAllocConnect)
  SYM(SQLAllocEnv)
  SYM(SQLAllocStmt)
  SYM(SQLBindCol)
  SYM(SQLBindParameter)
  SYM(SQLCancel)
  SYM(SQLColAttributes)
  SYM(SQLConnect)
  SYM(SQLDataSources)
  SYM(SQLDescribeCol)
  SYM(SQLDescribeParam)
  SYM(SQLDisconnect)
  SYM(SQLError)
  SYM(SQLExecDirect)
  SYM(SQLExecute)
  SYM(SQLExtendedFetch)
  SYM(SQLFetch)
  SYM(SQLFreeConnect)
  SYM(SQLFreeEnv)
  SYM(SQLFreeStmt)
  SYM(SQLGetCursorName)
  SYM(SQLGetData)
  SYM(SQLGetInfo)
  SYM(SQLGetFunctions)
  SYM(SQLGetTypeInfo)
  SYM(SQLMoreResults)
  SYM(SQLNativeSql)
  SYM(SQLNumParams)
  SYM(SQLNumResultCols)
  SYM(SQLParamData)
  SYM(SQLPrepare)
  SYM(SQLPutData)
  SYM(SQLRowCount)
  SYM(SQLSetConnectOption)
  SYM(SQLSetCursorName)
  SYM(SQLSetPos)
  SYM(SQLSetScrollOptions)
  SYM(SQLTables)
  SYM(SQLTransact)
  ENDLIB
  
#endif

#ifdef rt_LIB_OCI

  /* ORACLE V7 OCI */

  LIB(libocic,so)
  
  SYM(orlon)
  SYM(olog)
  SYM(ologof)
  SYM(oopen)
  SYM(oparse)
  SYM(obndra)
  SYM(obndrv)
  SYM(odescr)
  SYM(odefin)
  SYM(oexec)
  SYM(oexfet)
  SYM(oexn)
  SYM(ofen)
  SYM(oflng)
  SYM(ocan)
  SYM(oclose)
  SYM(oerhms)
  SYM(ocom)
  SYM(orol)
  SYM(ocon)
  SYM(ocof)
  
  ENDLIB
  
#endif

#ifdef rt_LIB_ERX
  LIB(liberx,so)
  SYM(ERXCall)
  SYM(ERXConnect)
  SYM(ERXDisconnect)
  SYM(ERXGetArchitecture)
  SYM(ERXGetLastError)
  SYM(ERXRegister)
  SYM(ERXReset)
  SYM(ERXUnregister)
  ENDLIB
  
  LIB(libsag,so)
  SYM(ccalc_call)
  SYM(ccalc_result)
  SYM(natimg_rpcSuch)
  SYM(natimg_rpcSuch_dokid1)
  SYM(natimg_rpcSuch_dokid2)
  SYM(natimg_rpcSuch_kurztitel)
  SYM(natimg_rpcSuch_retcode)
  SYM(natimg_rpcGetNr)
  SYM(natimg_rpcGetNr_anzahl)
  SYM(natimg_rpcGetNr_retcode)
  SYM(natimg_rpcGetIm)
  SYM(natimg_rpcGetIm_path)
  SYM(natimg_rpcGetIm_retcode)
  ENDLIB
#endif


/* Declare static TycoonOS symbols */

  LIB(libTOS,so)

  SYM(tos_init)

  SYM(tosDate_time)
  SYM(tosDate_clock)
  SYM(tosDate_clocksPerSecond)
  SYM(tosDate_normalize)
  SYM(tosDate_fromTime)
  SYM(tosDate_asTime)
  SYM(tosDate_fromString)
  SYM(tosDate_sizeOfHandle)
  SYM(tosDate_setHandle)
  SYM(tosDate_format)
  SYM(tosDate_offset)
  SYM(tosDate_zone)
  SYM(tosDate_second)
  SYM(tosDate_minute)
  SYM(tosDate_hour)
  SYM(tosDate_day)
  SYM(tosDate_month)
  SYM(tosDate_year)
  SYM(tosDate_weekDay)
  SYM(tosDate_yearDay)
  SYM(tosDate_isDST)

  SYM(tosDirectory_realpath)
  SYM(tosDirectory_chroot)
  SYM(tosDirectory_create)
  SYM(tosDirectory_remove)
  SYM(tosDirectory_sizeOfT)
  SYM(tosDirectory_open)
  SYM(tosDirectory_read)
  SYM(tosDirectory_close)

  SYM(tosError_getText)
  SYM(tosError_log)

  SYM(tosFile_stdin)
  SYM(tosFile_stdout)
  SYM(tosFile_stderr)
  SYM(tosFile_open)
  SYM(tosFile_sync)
  SYM(tosFile_close)
  SYM(tosFile_read1)
  SYM(tosFile_write1)
  SYM(tosFile_readChar)
  SYM(tosFile_writeChar)
  SYM(tosFile_exists)
  SYM(tosFile_isCharacterDevice)
  SYM(tosFile_copy)
  SYM(tosFile_rename)
  SYM(tosFile_remove)
  SYM(tosFile_mkSymLink)

  SYM(tosFileinfo_bufferSize)
  SYM(tosFileinfo_getBuffer)
  SYM(tosFileinfo_isFile)
  SYM(tosFileinfo_isDirectory)
  SYM(tosFileinfo_isSymbolicLink)
  SYM(tosFileinfo_lastModified)
  SYM(tosFileinfo_lastAccessed)
  SYM(tosFileinfo_lastStatusChange)
  SYM(tosFileinfo_size)

  /* Only a subset from tosFilename */
  SYM(tosFilename_normalize)
  SYM(tosFilename_tempName)
  SYM(tosFilename_tempNameSize)

  SYM(tosFilepos_seek)
  SYM(tosFilepos_seekEnd)
  SYM(tosFilepos_seekCur)

  SYM(tosInt32_fromInt)
  SYM(tosInt32_asInt)
  SYM(tosInt32_fromString)
  SYM(tosInt32_order)
  SYM(tosInt32_add)
  SYM(tosInt32_sub)
  SYM(tosInt32_mul)
  SYM(tosInt32_div)
  SYM(tosInt32_mod)

  SYM(tosLong_fromInt)
  SYM(tosLong_asInt)
  SYM(tosLong_order)
  SYM(tosLong_add)
  SYM(tosLong_sub)
  SYM(tosLong_mul)
  SYM(tosLong_and)
  SYM(tosLong_div)
  SYM(tosLong_mod)

  SYM(tosMd5_contextSize)
  SYM(tosMd5_init)
  SYM(tosMd5_update)
  SYM(tosMd5_final)

  SYM(tosMemory_formatString)
  SYM(tosMemory_isNull)
  SYM(tosMemory_peekInt)
  SYM(tosMemory_peekString)
  SYM(tosMemory_peekChar)
  SYM(tosMemory_peekUnsignedChar)
  SYM(tosMemory_peekShort)
  SYM(tosMemory_peekReal)
  SYM(tosMemory_peekLong)
  SYM(tosMemory_pokeInt)
  SYM(tosMemory_pokeString)
  SYM(tosMemory_pokeChar)
  SYM(tosMemory_pokeUnsignedChar)
  SYM(tosMemory_pokeShort)
  SYM(tosMemory_pokeReal)
  SYM(tosMemory_pokeLong)
  SYM(tosMemory_IntToInt32)
  
  SYM(tosNet_getIPAddrLen)
  SYM(tosNet_getMaxHostNameLen)
  SYM(tosNet_getHostName)
  SYM(tosNet_getHostByName)

  SYM(tosProcess_getpid)
  SYM(tosProcess_getppid)
  SYM(tosProcess_getUserCPUtime)
  SYM(tosProcess_getKernelCPUtime)
  SYM(tosProcess_kill)
  SYM(tosProcess_signal)
  SYM(tosProcess_execute)
  SYM(tosProcess_system)
 
  SYM(tosReal_fromInt)
  SYM(tosReal_asInt)
  SYM(tosReal_fromLong)
  SYM(tosReal_asLong)
  SYM(tosReal_fromString)
  SYM(tosReal_floatNaN)
  SYM(tosReal_doubleNaN)
  SYM(tosReal_order)
  SYM(tosReal_add)
  SYM(tosReal_sub)
  SYM(tosReal_mul)
  SYM(tosReal_div)
  SYM(tosReal_sqrt)
  SYM(tosReal_ln)
  SYM(tosReal_sin)
  SYM(tosReal_cos)
  SYM(tosReal_tan)
  SYM(tosReal_asin)
  SYM(tosReal_acos)
  SYM(tosReal_atan)
  SYM(tosReal_pow)
  SYM(tosReal_expE)

  SYM(tosSecurity_uidToString)
  SYM(tosSecurity_gidToString)
  SYM(tosSecurity_uidFromString)
  SYM(tosSecurity_gidFromString)
  SYM(tosSecurity_getCurrentUID)
  SYM(tosSecurity_getCurrentGID)
  SYM(tosSecurity_setCurrentUID)
  SYM(tosSecurity_setCurrentGID)
  SYM(tosSecurity_chown)

  SYM(tosSocket_nil)
  SYM(tosSocket_newStreamSocket)
  SYM(tosSocket_newDatagramSocket)
  SYM(tosSocket_newServerSocket)
  SYM(tosSocket_close)
  SYM(tosSocket_connect)
  SYM(tosSocket_listen)
  SYM(tosSocket_bind)
  SYM(tosSocket_accept)
  SYM(tosSocket_read)
  SYM(tosSocket_write)
  SYM(tosSocket_readChar)
  SYM(tosSocket_writeChar)
  SYM(tosSocket_address)
  SYM(tosSocket_remoteAddress)
  SYM(tosSocket_port)
  SYM(tosSocket_remotePort)

  SYM(tosString_locateSubString)
  SYM(tosString_locateLastSubString)
  SYM(tosString_locateSomeChar)
  SYM(tosString_locateSomeCharBefore)

  ENDLIB


/*
 * ANSI-C
 *
 * Portable ANSI-C symbols. For all functions with filename parameter better
 * use the corresponding functions from TycoonOS.
 *
 */

  LIB(libc, so)

  SYM(abort)
  SYM(atof)
  SYM(atoi)
  SYM(exit)
  SYM(free)
  SYM(getenv)
  SYM(malloc)
  SYM(memset)
  SYM(rand)
  SYM(remove)
  SYM(rename)
  SYM(sprintf)
  SYM(srand)
  SYM(strcat)
  SYM(strcmp)
  SYM(strcpy)
  SYM(strerror)
  SYM(strftime)
  SYM(strlen)
  SYM(strncat)
  SYM(strncmp)
  SYM(strncpy)
  SYM(strstr)
  SYM(strtod)
  SYM(system)
  SYM(tmpnam)
  ENDLIB


/* TVM Interface */

  LIB(libtvm,so)
  /* profile interface */
  SYM(tmprofile_reset)
  SYM(tmprofile_result)
  SYM(tmprofile_start)
  SYM(tmprofile_stop)
  SYM(tmprofile_setResolution)
  SYM(tmprofile_setMode)
  ENDLIB

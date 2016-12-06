#ifndef forPublicRelease
/*=============================================================================
	TargetConditionals.h

	$Log: TargetConditionals.h,v $
	Revision 1.1  2006/06/07 00:51:05  seangies
	Added windows-specific project files and code.
	
	Revision 1.3  2005/09/27 19:10:13  bpietsch
	Merge  QT7Branch     (QT7Base0 -> QTMerge33_QT702_to_TOT_QT7) Shipped:   QT702GM
	  ==>  Top Of Tree    PreMerge:   QTMerge33_QT702_to_TOT_TOT  PostMerge: QTMerge33_QT702_to_TOT
	_______________________________________________________________________________________________
	
	Revision 1.2  2005/09/23 02:39:02  bpietsch
	QT702GM to TOT merge -- files added on QT7Branch that were not added to TOT <dkudo>
	
	Revision 1.1.24.1  2005/08/24 21:17:22  duano
	Add forPublicRelease conditional so we can sanitize this header for use in the Windows SDK. [4181600]
	
	Revision 1.1  2003/05/15 22:15:13  jsam
	Renamed from TargetConditionals.h.  Only used on non-mac. <grc>
	
	Revision 1.3  2003/04/03 19:17:36  thaiwey
	Add this for windows build after big header change.
	
	Revision 1.1  2002/12/02 18:30:39  gregc
	First time.
	
	Note: This file exists only to serve the Windows build

=============================================================================*/
#else
/*
	File:		TargetConditionals.h

	Copyright:	© 2002-2005 by Apple Computer, Inc., all rights reserved.

	Simple redirect header to ConditionalMacros.h for use on Windows.
*/
#endif

#if !defined(__TargetConditionals_h__)
#define __TargetConditionals_h__

#include "ConditionalMacros.h"

#endif

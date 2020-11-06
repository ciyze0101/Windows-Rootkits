#pragma once
#include "Head.h"

/*********************************************************
function:		SimulateApi
description:
**********************************************************/
NTSTATUS SimulateApi(ULONG64 param1)
{
	MyPrint(_TitleAndFunc"param1:%16IX\n", param1);
	return STATUS_SUCCESS;
}

/*********************************************************
function:		ApiTransfer_SimulateApi
description:	test of call apis in the hidden apis
**********************************************************/

NTSTATUS ApiTransfer_SimulateApi(PHIDDEN_PAGE_RECORD pHiddenPageRecord, ULONG64 param1)
{
	//BOOL		IsIrqlChanged = FALSE;
	NTSTATUS	Status;

	/*
	if (HighestIrql < HIDDEN_IRQL)
	{
	ChangeIrql(HighestIrql);
	IsIrqlChanged = TRUE;
	}
	*/

	ContextHiddenToOriginal(pHiddenPageRecord);

	Status = SimulateApi(param1);

	ContextOriginalToHidden(pHiddenPageRecord);

	/*
	if (IsIrqlChanged)
	ChangeIrql(HIDDEN_IRQL);
	*/

	return Status;
}
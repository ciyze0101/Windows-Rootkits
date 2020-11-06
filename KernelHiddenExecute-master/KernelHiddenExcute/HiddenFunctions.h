#pragma once
#include "Head.h"

#pragma code_seg(SECTION_NAME_HIDDEN)

NTSTATUS HiddenFunctionA(PHIDDEN_PAGE_RECORD pHiddenPageRecord)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	Status = ApiTransfer_SimulateApi(pHiddenPageRecord, 0xFAFAFAFAFAFAFAFA);

	return Status;
}

#pragma code_seg()
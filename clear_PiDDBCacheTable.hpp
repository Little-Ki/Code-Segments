#pragma once

struct PiDDBCacheEntry
{
	LIST_ENTRY 	List;
	UNICODE_STRING 	DriverName;
	ULONG 		TimeDateStamp;
	NTSTATUS 	LoadStatus;
	char 		_0x0028[16]; // data from the shim engine, or uninitialized memory for custom drivers
};

bool LocatePiDDB(PERESOURCE* lock, PRTL_AVL_TABLE* table) {
	// ...Search with using pattern scan.
}

static bool RemoveEntry(const PiDDBCacheEntry& dummy) {
	PERESOURCE 	PiDDBLock;
	PRTL_AVL_TABLE 	PiDDBCacheTable;
	
	if(!LocatePiDDB(&PiDDBLock, &PiDDBCacheTable)) {
		return false;
	}

	// acquire the ddb resource lock
	ExAcquireResourceExclusiveLite(PiDDBLock, TRUE);
	
	// search target entry in the table
	auto FoundEntryPtr = (PiDDBCacheEntry*)RtlLookupElementGenericTableAvl(PiDDBCacheTable, &dummy);
	if(!FoundEntryPtr)
	{
		// release the ddb resource lock
		ExReleaseResourceLite(PiDDBLock);
		return false;
	}
	 
	// first, unlink from the list
	RemoveEntryList(&FoundEntryPtr->List);
	// then delete the element from the avl table
	RtlDeleteElementGenericTableAvl(PiDDBCacheTable, pFoundEntry);

	// release the ddb resource lock
	ExReleaseResourceLite(PiDDBLock);
	
	return true;
}


set(SOURCES MediaImportChangesetTask.cpp
            MediaImportCleanupTask.cpp
            MediaImportRemovalTask.cpp
            MediaImportRetrievalTask.cpp
            MediaImportScrapingTask.cpp
            MediaImportSourceRegistrationTask.cpp
            MediaImportSynchronisationTask.cpp
            MediaImportUpdateTask.cpp)

set(HEADERS MediaImportChangesetTask.h
            MediaImportCleanupTask.h
            MediaImportRemovalTask.h
            MediaImportRetrievalTask.h
            MediaImportScrapingTask.h
            MediaImportSourceRegistrationTask.h
            MediaImportSynchronisationTask.h
            MediaImportUpdateTask.h)

core_add_library(media_import_task)

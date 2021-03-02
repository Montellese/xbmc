set(SOURCES MediaImportImportsRemovalJob.cpp
            MediaImportItemsSynchronizationJob.cpp
            MediaImportSourceJobBase.cpp
            MediaImportSourceReadyJob.cpp
            MediaImportSourceRegistrationJob.cpp
            MediaImportTaskProcessorJob.cpp
            MediaImportThreadedItemsSynchronizationJob.cpp
            MediaImportUpdateItemOnSourceJob.cpp)

set(HEADERS MediaImportImportsRemovalJob.h
            MediaImportItemsSynchronizationJob.h
            MediaImportSourceActivationJob.h
            MediaImportSourceJobBase.h
            MediaImportSourceReadyJob.h
            MediaImportSourceRegistrationJob.h
            MediaImportTaskProcessorJob.h
            MediaImportTaskTypes.h
            MediaImportThreadedItemsSynchronizationJob.h
            MediaImportUpdateItemOnSourceJob.h)

core_add_library(media_import_jobs)

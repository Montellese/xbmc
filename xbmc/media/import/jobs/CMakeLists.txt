set(SOURCES MediaImportImportsRemovalJob.cpp
            MediaImportItemsSynchronizationJob.cpp
            MediaImportSourceJobBase.cpp
            MediaImportSourceReadyJob.cpp
            MediaImportSourceRegistrationJob.cpp
            MediaImportTaskProcessorJob.cpp
            MediaImportUpdateItemOnSourceJob.cpp)

set(HEADERS MediaImportImportsRemovalJob.h
            MediaImportItemsSynchronizationJob.h
            MediaImportSourceActivationJob.h
            MediaImportSourceJobBase.h
            MediaImportSourceReadyJob.h
            MediaImportSourceRegistrationJob.h
            MediaImportTaskProcessorJob.h
            MediaImportTaskTypes.h
            MediaImportUpdateItemOnSourceJob.h)

core_add_library(media_import_jobs)

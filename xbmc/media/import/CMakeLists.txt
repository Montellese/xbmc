set(SOURCES MediaImport.cpp
            MediaImportManager.cpp
            MediaImportSource.cpp
            MediaImportTaskProcessorJob.cpp)

set(HEADERS IMediaImporter.h
            IMediaImportHandler.h
            IMediaImportHandlerManager.h
            IMediaImportRepository.h
            IMediaImportTask.h
            MediaImport.h
            MediaImportChangesetTypes.h
            MediaImportManager.h
            MediaImportSource.h
            MediaImportTaskProcessorJob.h
            MediaImportTaskTypes.h)

core_add_library(media_import)

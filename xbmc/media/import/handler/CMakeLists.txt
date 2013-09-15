set(SOURCES EpisodeImportHandler.cpp
            MovieImportHandler.cpp
            MusicVideoImportHandler.cpp
            SeasonImportHandler.cpp
            TvShowImportHandler.cpp
            VideoImportHandler.cpp)

set(HEADERS EpisodeImportHandler.h
            MovieImportHandler.h
            MusicVideoImportHandler.h
            SeasonImportHandler.h
            TvShowImportHandler.h
            VideoImportHandler.h)

core_add_library(media_import_handler)

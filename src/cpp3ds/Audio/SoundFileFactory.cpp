////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2015 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <cpp3ds/Audio/SoundFileFactory.hpp>
#ifdef CPP3DS_ENABLE_FLAC
#include <cpp3ds/Audio/SoundFileReaderFlac.hpp>
#include <cpp3ds/Audio/SoundFileWriterFlac.hpp>
#endif
#ifdef CPP3DS_ENABLE_OGG
#include <cpp3ds/Audio/SoundFileReaderOgg.hpp>
//#include <cpp3ds/Audio/SoundFileWriterOgg.hpp>
#endif
#ifdef CPP3DS_ENABLE_MP3
#include <cpp3ds/Audio/SoundFileReaderMp3.hpp>
#endif
#include <cpp3ds/Audio/SoundFileReaderWav.hpp>
#include <cpp3ds/Audio/SoundFileWriterWav.hpp>
#include <cpp3ds/System/FileInputStream.hpp>
#include <cpp3ds/System/MemoryInputStream.hpp>


namespace
{
    // Register all the built-in readers and writers if not already done
    void ensureDefaultReadersWritersRegistered()
    {
        static bool registered = false;
        if (!registered)
        {
#ifdef CPP3DS_ENABLE_FLAC
            cpp3ds::SoundFileFactory::registerReader<cpp3ds::priv::SoundFileReaderFlac>();
            cpp3ds::SoundFileFactory::registerWriter<cpp3ds::priv::SoundFileWriterFlac>();
#endif
#ifdef CPP3DS_ENABLE_OGG
            cpp3ds::SoundFileFactory::registerReader<cpp3ds::priv::SoundFileReaderOgg>();
//            cpp3ds::SoundFileFactory::registerWriter<cpp3ds::priv::SoundFileWriterOgg>();
#endif
#ifdef CPP3DS_ENABLE_MP3
			cpp3ds::SoundFileFactory::registerReader<cpp3ds::priv::SoundFileReaderMp3>();
#endif
            cpp3ds::SoundFileFactory::registerReader<cpp3ds::priv::SoundFileReaderWav>();
            cpp3ds::SoundFileFactory::registerWriter<cpp3ds::priv::SoundFileWriterWav>();
            registered = true;
        }
    }
}

namespace cpp3ds
{
SoundFileFactory::ReaderFactoryArray SoundFileFactory::s_readers;
SoundFileFactory::WriterFactoryArray SoundFileFactory::s_writers;


////////////////////////////////////////////////////////////
SoundFileReader* SoundFileFactory::createReaderFromFilename(const std::string& filename)
{
    // Register the built-in readers/writers on first call
    ensureDefaultReadersWritersRegistered();

    // Wrap the input file into a file stream
    FileInputStream stream;
    if (!stream.open(filename))
        return NULL;

    // Test the filename in all the registered factories
    for (ReaderFactoryArray::const_iterator it = s_readers.begin(); it != s_readers.end(); ++it)
    {
        stream.seek(0);
        if (it->check(stream))
            return it->create();
    }

    // No suitable reader found
    return NULL;
}


////////////////////////////////////////////////////////////
SoundFileReader* SoundFileFactory::createReaderFromMemory(const void* data, std::size_t sizeInBytes)
{
    // Register the built-in readers/writers on first call
    ensureDefaultReadersWritersRegistered();

    // Wrap the memory file into a file stream
    MemoryInputStream stream;
    stream.open(data, sizeInBytes);

    // Test the stream for all the registered factories
    for (ReaderFactoryArray::const_iterator it = s_readers.begin(); it != s_readers.end(); ++it)
    {
        stream.seek(0);
        if (it->check(stream))
            return it->create();
    }

    // No suitable reader found
    return NULL;
}


////////////////////////////////////////////////////////////
SoundFileReader* SoundFileFactory::createReaderFromStream(InputStream& stream)
{
    // Register the built-in readers/writers on first call
    ensureDefaultReadersWritersRegistered();

    // Test the stream for all the registered factories
    for (ReaderFactoryArray::const_iterator it = s_readers.begin(); it != s_readers.end(); ++it)
    {
        stream.seek(0);
        if (it->check(stream))
            return it->create();
    }

    // No suitable reader found
    return NULL;
}


////////////////////////////////////////////////////////////
SoundFileWriter* SoundFileFactory::createWriterFromFilename(const std::string& filename)
{
    // Register the built-in readers/writers on first call
    ensureDefaultReadersWritersRegistered();

    // Test the filename in all the registered factories
    for (WriterFactoryArray::const_iterator it = s_writers.begin(); it != s_writers.end(); ++it)
    {
        if (it->check(filename))
            return it->create();
    }

    // No suitable writer found
    return NULL;
}

} // namespace cpp3ds

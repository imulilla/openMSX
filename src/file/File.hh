// $Id$

#ifndef __FILEOPENER_HH__
#define __FILEOPENER_HH__

#include <memory>
#include <string>
#include <time.h>
#include "openmsx.hh"
#include "FileException.hh"

using std::auto_ptr;
using std::string;

namespace openmsx {

class FileBase;

enum OpenMode {
	NORMAL,
	TRUNCATE,
	CREATE,
	LOAD_PERSISTENT,
	SAVE_PERSISTENT,
};

class File
{
public:
	/**
	 * Create file object and open underlying file.
	 * @param url Full URL or relative path of the file
	 *   that will be represented by this file object.
	 * @param mode Mode to open the file in:
	 * @throws FileException
	 */
	File(const string& url, OpenMode mode = NORMAL);
	
	/**
	 * Destroy file object.
	 */
	~File();

	/**
	 * Read from file.
	 * @param buffer Destination address
	 * @param num Number of bytes to read
	 * @throws FileException
	 */
	void read(byte* buffer, unsigned num);

	/**
	 * Write to file.
	 * @param buffer Source address
	 * @param num Number of bytes to write
	 * @throws FileException
	 */
	void write(const byte* buffer, unsigned num);

	/**
	 * Map file in memory.
	 * @param writeBack Set to true if writes to the memory block
	 *              should also be written to the file. Note that
	 *              the file may only be updated when you munmap
	 *              again (may happen earlier but not guaranteed).
	 * @result Pointer to memory block.
	 * @throws FileException
	 */
	byte* mmap(bool writeBack = false);

	/**
	 * Unmap file from memory. 
	 * @throws FileException
	 */
	void munmap();

	/**
	 * Returns the size of this file
	 * @result The size of this file
	 * @throws FileException
	 */
	unsigned getSize();

	/**
	 * Move read/write pointer to the specified position.
	 * @param pos Position in bytes from the beginning of the file.
	 * @throws FileException
	 */
	void seek(unsigned pos);

	/**
	 * Get the current position of the read/write pointer.
	 * @result Position in bytes from the beginning of the file.
	 * @throws FileException
	 */
	unsigned getPos();

	/**
	 * Truncate file size. Enlarging file size always works, but
	 * making file smaller doesn't work on some platforms (windows)
	 * @throws FileException
	 */
	void truncate(unsigned size);

	/**
	 * Returns the URL of this file object.
	 * @throws FileException
	 */
	const string getURL() const;

	/**
	 * Get a local filename for this object. Useful if this object
	 * refers to a HTTP or FTP file.
	 * @result Filename of a local file that is identical to the
	 *         file that this object refers to.
	 * @throws FileException
	 */
	const string getLocalName() const;

	/**
	 * Check if this file is readonly
	 * @result true iff file is readonly
	 * @throws FileException
	 */
	bool isReadOnly() const;

	/**
	 * Get the date/time of last modification
	 * @throws FileException
	 */
	time_t getModificationDate();

private:
	auto_ptr<FileBase> file;
};

} // namespace openmsx

#endif


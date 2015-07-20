#pragma once
#include <stdint.h>

namespace leap {
  struct create_delete;
  struct descriptor;
  class OArchive;
  class IArchive;

  enum class serial_type {
    varint = 0,
    b64 = 1,
    string = 2,
    b32 = 5
  };

  /// <summary>
  /// Interface used for general purpose field serialization
  /// </summary>
  struct field_serializer {
    /// <returns>
    /// The field type, used for serial operations
    /// </returns.
    virtual serial_type type(void) const = 0;

    /// <returns>
    /// The exact number of bytes that will be required in the serialize operation
    /// </returns>
    virtual uint64_t size(const void* pObj) const = 0;

    /// <summary>
    /// Serializes the object into the specified buffer
    /// </summary>
    virtual void serialize(OArchive& ar, const void* pObj) const = 0;

    /// <summary>
    /// Deserializes the object from the specified archive
    /// </summary>
    /// <param name="ar">The input archive</param>
    /// <param name="pObj">A pointer to the memory receiving the deserialized object</param>
    /// <param name="ncb">The maximum number of bytes to be read</param>
    /// <remarks>
    /// The "ncb" field is only valid if the type of this object is serial_type::string.
    /// </remarks>
    virtual void deserialize(IArchive& ar, void* pObj, uint64_t ncb) const = 0;
  };

  /// <summary>
  /// Represents a stateful serialization archive structure
  /// </summary>
  class OArchive {
  public:
    /// <summary>
    /// Creates a new output archive based on the specified stream
    /// </summary>
    virtual ~OArchive(void) {}

    /// <summary>
    /// Registers an object for serialization, returning the ID that will be given to the object
    /// </summary>
    virtual uint32_t RegisterObject(const field_serializer& serializer, const void* pObj) = 0;

    /// <summary>
    /// Writes the specified bytes to the output stream
    /// </summary>
    virtual void Write(const void* pBuf, uint64_t ncb) const = 0;

    /// <returns>
    /// The number of bytes that would be required to serialize the specified integer with varint encoding
    /// </returns>
    static uint16_t VarintSize(int64_t value);

    /// <summary>
    /// Writes the specified integer as a varint to the output stream
    /// </sumamry>
    void WriteVarint(int64_t value) const;

    // Convenience overloads:
    void Write(int32_t value) const { return Write(&value, sizeof(value)); }
    void Write(uint32_t value) const { return Write(&value, sizeof(value)); }
    void Write(int64_t value) const { return Write(&value, sizeof(value)); }
    void Write(uint64_t value) const { return Write(&value, sizeof(value)); }
  };

  class IArchive {
  public:
    /// <summary>
    /// Creates a new output archive based on the specified stream
    /// </summary>
    virtual ~IArchive(void) {}

    /// <summary>
    /// Registers an encountered identifier for later deserialization
    /// </summary>
    /// <param name="pfnAllocate">The allocation routine, to be invoked if the object isn't found</param>
    /// <param name="desc">The descriptor that will be used to deserialize the object, if needed</param>
    /// <param name="objId">The ID of the object that has been encountered</param>
    /// <returns>A pointer to the object</returns>
    /// <remarks>
    /// While this method does always return a valid pointer, and the pointed-to object is guaranteed to
    /// be at a minimum default constructed, the returned object may nevertheless have yet to be deserialized.
    /// </remarks>
    virtual void* Lookup(const create_delete& cd, const field_serializer& serializer, uint32_t objId) = 0;

    /// <summary>
    /// Reads the specified number of bytes from the input stream
    /// </summary>
    virtual void Read(void* pBuf, uint64_t ncb) = 0;

    // Convenience overloads:
    void Read(int32_t& val) { Read(&val, sizeof(val)); }
    void Read(uint32_t& val) { Read(&val, sizeof(val)); }
    void Read(int64_t& val) { Read(&val, sizeof(val)); }
    void Read(uint64_t& val) { Read(&val, sizeof(val)); }

    /// <summary>
    /// Discards the specified number of bytes from the input stream
    /// </summary>
    virtual void Skip(uint64_t ncb) = 0;

    /// <returns>
    /// The total number of bytes read so far
    /// </returns>
    virtual uint64_t Count(void) const = 0;

    /// <summary>
    /// Interprets bytes from the output stream as a single varint
    /// </sumamry>
    int64_t ReadVarint(void);
  };
}
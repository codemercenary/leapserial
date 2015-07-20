#pragma once
#include "Archive.h"
#include <vector>
#include <stack>
#include <map>
#include <set>

namespace leap {

  struct VTable {
    uint32_t offset;
    uint16_t size;
    uint16_t tableSize;
    std::vector<uint16_t> offsets;

    std::tuple<uint16_t, uint16_t, std::vector<uint16_t>> make_tuple() const { return std::make_tuple(size, tableSize, offsets); }

    bool operator==(const VTable& other) const {
      return make_tuple() == other.make_tuple();
    }
    bool operator<(const VTable& other) const {
      return make_tuple() < other.make_tuple();
    }
    
  };

  //Note: Multiple objects with the same vtable are currently not well supported, as all complex types
  //will always have their vtables written.
  class OArchiveFlatbuffer :
    public OArchiveRegistry
  {
  public:
    OArchiveFlatbuffer(std::ostream& os);

    // Finishes writing, since this is a buffered system. Mostly used for testing.
    void Finish();

    // OArchiveRegistry overrides
    void WriteByteArray(const void* pBuf, uint64_t ncb, bool writeSize = false) override;
    void WriteString(const void* pBuf, uint64_t charCount, uint8_t charSize) override;
    void WriteBool(bool value) override;
    void WriteInteger(int64_t value, uint8_t ncb) override;
    using OArchive::WriteInteger;
    void WriteFloat(float value) override;
    void WriteFloat(double value) override;
    void WriteObjectReference(const field_serializer& serializer, const void* pObj) override;
    void WriteObject(const field_serializer& serializer, const void* pObj) override;
    void WriteDescriptor(const descriptor& descriptor, const void* pObj) override;
    void WriteArray(const field_serializer& desc, uint64_t n, std::function<const void*()> enumerator) override;
    void WriteDictionary(
      uint64_t n,
      const field_serializer& keyDesc,
      std::function<const void*()> keyEnumerator,
      const field_serializer& valueDesc,
      std::function<const void*()> valueEnumerator
      ) override;

    //Size query functions.  Note that these do not return the total size of the object,
    //but rather the size they take up in a table (sizeof(uint32_t) for objects stored by reference).
    uint64_t SizeInteger(int64_t value, uint8_t ncb) const override;
    uint64_t SizeFloat(float value) const override;
    uint64_t SizeFloat(double value) const override;
    uint64_t SizeBool(bool value) const override;
    uint64_t SizeString(const void* pBuf, uint64_t ncb, uint8_t charSize) const override;
    uint64_t SizeObjectReference(const field_serializer& serializer, const void* pObj) const override;
    uint64_t SizeDescriptor(const descriptor& descriptor, const void* pObj) const override;
    uint64_t SizeArray(const field_serializer& desc, uint64_t n, std::function<const void*()> enumerator) const override;
    uint64_t SizeDictionary(
      uint64_t n,
      const field_serializer& keyDesc,
      std::function<const void*()> keyEnumerator,
      const field_serializer& valueDesc,
      std::function<const void*()> valueEnumerator
      ) const override;

  private:
    std::ostream& os;

    //Flatbuffer messages are most easily built backwards, so we'll write into this,
    //Then dump it into os when we're done with the message.
    std::vector<uint8_t> m_builder;

    std::set<VTable> m_vTables;

    std::map<const void*, uint32_t> m_offsets;
    uint8_t m_largestAligned = 1;

    void WriteRelativeOffset(const void* pObj);
    void Align(uint8_t boundary);
    void PreAlign(uint32_t len, uint8_t alignment);
  };

  class IArchiveFlatbuffer : 
    public IArchiveRegistry
  {
  public:
    IArchiveFlatbuffer(std::istream& is);
    ~IArchiveFlatbuffer();

    void ReadObject(const field_serializer& sz, void* pObj, internal::AllocationBase* pOwner) override;

    ReleasedMemory ReadObjectReferenceResponsible(ReleasedMemory(*pfnAlloc)(), const field_serializer& sz, bool isUnique) override;

    void Skip(uint64_t ncb) override;
    uint64_t Count(void) const { return 0; }

    void ReadDescriptor(const descriptor& descriptor, void* pObj, uint64_t ncb) override;
    void ReadByteArray(void* pBuf, uint64_t ncb) override;
    void ReadString(std::function<void*(uint64_t)> getBufferFn, uint8_t charSize, uint64_t ncb) override;
    bool ReadBool() override;
    uint64_t ReadInteger(uint8_t ncb) override;
    void ReadFloat(float& value) override;
    void ReadFloat(double& value) override;
    void ReadArray(std::function<void(uint64_t)> sizeBufferFn, const field_serializer& t_serializer, std::function<void*()> enumerator, uint64_t n) override;
    void ReadDictionary(const field_serializer& keyDesc,
      void* key,
      const field_serializer& valueDesc,
      void* value,
      std::function<void(const void* key, const void* value)> inserter
      ) override;

    void* ReadObjectReference(const create_delete& cd, const field_serializer& desc) override;

  private:
    std::vector<uint8_t> m_data;
    
    //The offset into m_data to read from - set to the end of the last read object after
    //every read operation, causing it to automatically advance through fields
    uint32_t m_offset = 0;

    //data read helpers:
    template<typename T>
    T GetValue(uint32_t off) { return *reinterpret_cast<T*>(&m_data[off]); }
  };

}
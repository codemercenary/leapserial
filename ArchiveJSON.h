#pragma once
#include "Archive.h"
#include "DataStructures/Value.h"

namespace leap {

  class OArchiveJSON :
    public OArchiveRegistry
  {
  public:
    OArchiveJSON(std::ostream& os);

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

    Value m_value;
  };

  class IArchiveJSON :
    public IArchiveRegistry
  {
  public:
    IArchiveJSON(std::istream& is);

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
  };
}; 
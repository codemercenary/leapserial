// Copyright (C) 2012-2015 Leap Motion, Inc. All rights reserved.
#include "stdafx.h"
#include "AESStream.h"
#include <aes/aes256.h>

using namespace leap;

AES256Base::AES256Base(const std::array<uint8_t, 32>& key) :
  ctx(new aes256_context)
{
  aes256_init(ctx.get(), key.data());
  lastBlock = {};
  NextBlock();
}

AES256Base::~AES256Base(void) {}

void AES256Base::NextBlock(void) {
  // Encrypt the last fully encrypted block, we will use this as the basis for xoring the next one
  chained = lastBlock;
  aes256_encrypt_ecb(ctx.get(), chained.data());
  blockByte = 0;
}

AESDecryptionStream::AESDecryptionStream(std::unique_ptr<IInputStream>&& is, const std::array<uint8_t, 32>& key) :
  InputFilterStreamBase(std::move(is)),
  AES256Base(key)
{}

std::streamsize AESDecryptionStream::Length(void) {
  // We do not add any bytes relative to the underlying stream, we can just return its
  // byte count directly
  return is->Length();
}

std::streampos AESDecryptionStream::Tell(void) {
  return is->Tell();
}

bool AESDecryptionStream::Transform(const void* input, size_t& ncbIn, void* output, size_t& ncbOut) {
  size_t nWritten = 0;
  while (nWritten < ncbIn && nWritten < ncbOut) {
    // Instantiate a block if we have to:
    if (blockByte == chained.size())
      NextBlock();

    // XOR to implement our CBC mode
    lastBlock[blockByte] = reinterpret_cast<const uint8_t*>(input)[nWritten];
    reinterpret_cast<uint8_t*>(output)[nWritten] = chained[blockByte] ^ lastBlock[blockByte];

    // Advance
    blockByte++;
    nWritten++;
  }
  ncbOut = nWritten;
  ncbIn = nWritten;
  return true;
}

AESEncryptionStream::AESEncryptionStream(std::unique_ptr<IOutputStream>&& os, const std::array<uint8_t, 32>& key) :
  OutputFilterStreamBase(std::move(os)),
  AES256Base(key)
{}

bool AESEncryptionStream::Transform(const void* input, size_t& ncbIn, void* output, size_t& ncbOut, bool) {
  size_t nWritten = 0;
  while (nWritten < ncbIn && nWritten < ncbOut) {
    // Instantiate a block if we have to:
    if (blockByte == chained.size())
      NextBlock();

    // XOR to implement our CBC mode
    lastBlock[blockByte] = chained[blockByte] ^ reinterpret_cast<const uint8_t*>(input)[nWritten];
    reinterpret_cast<uint8_t*>(output)[nWritten] = lastBlock[blockByte];

    // Advance
    blockByte++;
    nWritten++;
  }
  ncbOut = nWritten;
  ncbIn = nWritten;

  return true;
}

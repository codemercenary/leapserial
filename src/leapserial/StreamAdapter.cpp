// Copyright (C) 2012-2015 Leap Motion, Inc. All rights reserved.
#include "stdafx.h"
#include "StreamAdapter.h"

using namespace leap;

bool InputStreamAdapter::IsEof(void) const {
  return is.eof();
}

std::streamsize InputStreamAdapter::Read(void* pBuf, std::streamsize ncb) {
  is.read((char*)pBuf, ncb);
  if (is.fail() && !is.eof())
    return -1;
  return is.gcount();
}

std::streamsize InputStreamAdapter::Skip(std::streamsize ncb) {
  is.ignore(ncb);
  if (is.fail() && !is.eof())
    return -1;
  return is.gcount();
}

InputStreamAdapter* InputStreamAdapter::Seek(std::streamsize off) {
  is.seekg(off, std::ios::beg);
  return this;
}

bool OutputStreamAdapter::Write(const void* pBuf, std::streamsize ncb) {
  return (bool)os.write((const char*)pBuf, ncb);
}

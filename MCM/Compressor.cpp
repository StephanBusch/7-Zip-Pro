#include "Compressor.hpp"
void CompressorFactories::init(){
  if (instance == nullptr){
    instance = new CompressorFactories;
    instance->addCompressor(false, new Compressor::FactoryOf<Store>);
  }
}
size_t MemCopyCompressor::getMaxExpansion(size_t in_size){
  return in_size;
}
size_t MemCopyCompressor::compressBytes(byte*in, byte*out, size_t count){
  memcpy16(out, in, count);
  return count;
}
void MemCopyCompressor::decompressBytes(byte*in, byte*out, size_t count){
  memcpy16(out, in, count);
}
size_t BitStreamCompressor::getMaxExpansion(size_t in_size){
  return in_size*kBits / 8 + 100;
}
size_t BitStreamCompressor::compressBytes(byte*in, byte*out, size_t count){
  MemoryBitStream<true>stream_out(out);
  for (; count; --count){
    stream_out.writeBits(*in++, kBits);
  }
  stream_out.flush();
  return stream_out.getData() - out;
}
void BitStreamCompressor::decompressBytes(byte*in, byte*out, size_t count){
  MemoryBitStream<true>stream_in(in);
  for (size_t i = 0; i < count; ++i){
    out[i] = stream_in.readBits(kBits);
  }
}
void Store::compress(Stream*in, Stream*out, uint64_t count){
  static const uint64_t kBufferSize = 8 * KB;
  byte buffer[kBufferSize];
  while (count > 0){
    const size_t read = in->read(buffer, (size_t)std::min(count, kBufferSize));
    if (read == 0){
      break;
    }
    out->write(buffer, read);
    count -= read;
  }
}
void Store::decompress(Stream*in, Stream*out, uint64_t count){
  static const uint64_t kBufferSize = 8 * KB;
  byte buffer[kBufferSize];
  while (count > 0){
    const size_t read = in->read(buffer, (size_t)std::min(count, kBufferSize));
    if (read == 0){
      break;
    }
    out->write(buffer, read);
    count -= read;
  }
}
void CompressorFactories::addCompressor(bool is_legacy, Compressor::Factory*factory){
  if (!is_legacy){
    factories.push_back(factory);
  }
  legacy_factories.push_back(factory);
}
uint32_t CompressorFactories::findFactoryIndex(Compressor::Factory*factory)const{
  for (uint32_t i = 0; i < legacy_factories.size(); ++i){
    if (legacy_factories[i] == factory){
      return i;
    }
  }
  assert(!"Can't find compressor");
  return static_cast<uint32_t>(-1);
}
CompressorFactories::CompressorFactories(){
}
Compressor::Factory*CompressorFactories::getLegacyFactory(uint32_t index){
  return legacy_factories[index];
}
Compressor::Factory*CompressorFactories::getFactory(uint32_t index){
  return factories[index];
}
Compressor*CompressorFactories::makeCompressor(uint32_t type){
  auto*factory = CompressorFactories::getInstance()->getFactory(type);
  check(factory != nullptr);
  return factory->create();
}

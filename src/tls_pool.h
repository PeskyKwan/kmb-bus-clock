#pragma once
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
// mbedTLS asks for two ~16.7KB record buffers per HTTPS request. On this board
// only one heap block is normally large enough, so long runtime splits it and
// the handshake fails with MBEDTLS_ERR_SSL_ALLOC_FAILED. Reserve the two record
// buffers once and lend them to every request; other sizes use the normal heap.
// One TLS session runs at a time; a third concurrent request falls back safely.
class TlsSlabPool{
 public:
 static constexpr int count=2;
 static constexpr size_t minBytes=16384,slabBytes=16896;
 bool install(uint8_t*a,uint8_t*b){if(!a||!b)return false;slabs[0]=a;slabs[1]=b;return true;}
 bool ready()const{return slabs[0]&&slabs[1];}
 bool owns(const void*p)const{for(auto*s:slabs)if(p&&p==s)return true;return false;}
 void* take(size_t n,size_t size){
  if(!n||size>SIZE_MAX/n)return nullptr;const size_t bytes=n*size;if(bytes<minBytes||bytes>slabBytes)return nullptr;
  for(int i=0;i<count;i++){if(!slabs[i])continue;bool expected=false;if(used[i].compare_exchange_strong(expected,true)){memset(slabs[i],0,bytes);hits++;return slabs[i];}}
  misses++;return nullptr;
 }
 bool give(void*p){for(int i=0;i<count;i++)if(p&&p==slabs[i]){used[i].store(false);return true;}return false;}
 int inUse()const{int n=0;for(auto&u:used)n+=u.load();return n;}
 std::atomic<uint32_t> hits{0},misses{0};
 private:
 uint8_t* slabs[count]={};std::atomic<bool> used[count]={};
};

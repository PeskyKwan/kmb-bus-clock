#include <cassert>
#include <cstdint>
#include <cstdio>
#include <thread>
#include <vector>
#include "../src/tls_pool.h"
int main(){
 TlsSlabPool pool;static uint8_t a[TlsSlabPool::slabBytes],b[TlsSlabPool::slabBytes];
 // Not installed: every request falls back to the heap.
 assert(!pool.ready()&&!pool.take(1,16717));assert(!pool.install(a,nullptr)&&!pool.ready());
 assert(pool.install(a,b)&&pool.ready());
 // Only mbedTLS record-buffer sizes are lent; everything else uses the heap.
 assert(!pool.take(1,0)&&!pool.take(1,4096)&&!pool.take(1,16383)&&!pool.take(1,TlsSlabPool::slabBytes+1));assert(!pool.take(SIZE_MAX,2));
 memset(a,0xEE,sizeof(a));memset(b,0xEE,sizeof(b));
 void*in=pool.take(1,16717);void*out=pool.take(16717,1);assert(in&&out&&in!=out);assert(pool.inUse()==2&&pool.hits==2);
 // calloc semantics: requested bytes are zeroed.
 for(int i=0;i<16717;i++)assert(((uint8_t*)in)[i]==0&&((uint8_t*)out)[i]==0);
 // A third concurrent request falls back instead of failing.
 assert(!pool.take(1,16717)&&pool.misses==2);
 assert(pool.owns(in)&&!pool.owns(a+1)&&!pool.owns(nullptr));
 int heapValue=0;assert(!pool.give(&heapValue)&&!pool.give(nullptr));
 assert(pool.give(in)&&pool.inUse()==1);void*again=pool.take(1,16384);assert(again==in);assert(pool.give(again)&&pool.give(out)&&pool.inUse()==0);
 // Many alternating sessions reuse the same two fixed buffers.
 for(int i=0;i<1000;i++){void*x=pool.take(1,16717),*y=pool.take(1,16717);assert(x&&y&&x!=y&&pool.owns(x)&&pool.owns(y));pool.give(x);pool.give(y);}
 // Concurrent tasks never receive the same slab twice.
 std::vector<std::thread> threads;std::atomic<int> overlaps{0};std::atomic<int> holders[2]={};
 for(int t=0;t<4;t++)threads.emplace_back([&]{for(int i=0;i<20000;i++){void*p=pool.take(1,16717);if(!p)continue;int k=p==a?0:1;if(holders[k].fetch_add(1))overlaps++;holders[k].fetch_sub(1);pool.give(p);}});
 for(auto&t:threads)t.join();assert(overlaps==0&&pool.inUse()==0);
 puts("TLS record-buffer pool sizing, reuse, fallback and concurrency checks passed");
}
